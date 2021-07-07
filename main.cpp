//Patrick Wheeler || 5 Jul 2021 || Seam Carve
#include "CImg.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
using namespace std;
using namespace cimg_library;

//-----------------------------KERNEL-------------------------------
//Laplacian of Gaussian estimation(sigma=1.4)
float loG[9][9] = {{0, 1, 1, 2, 2, 2, 1, 1, 0},
	{1, 2, 4, 5, 5, 5, 4, 2, 1}, 
	{1, 4, 5, 3, 0, 3, 5, 4, 1},
	{2, 5, 3, -12, -24, -12, 3, 5, 2},
	{2, 5, 0, -24, -40, -24, 0, 5, 2}, 
	{2, 5, 3, -12, -24, -12, 3, 5, 2},
	{1, 4, 5, 3, 0, 3, 5, 4, 1},
	{1, 2, 4, 5, 5, 5, 4, 2, 1}, 
	{0, 1, 1, 2, 2, 2, 1, 1, 0}};

//-----------------------------HELPERS------------------------------
//Struct to hold color vals of pixel
typedef struct Pixel{
	float r, g, b;
}Pixel;

//Set array vals to zero
void set_zero(int w, int h, float** arr){
	for (int i = 0; i < h; i++)
		for (int j = 0; j < w; j++)
			arr[i][j] = 0.0;
}

//Free array memory
void delete_arr(int h, float** arr){
	for (int i = 0; i < h; i++)
		delete[] arr[i];
	delete[] arr;
}

//Create 2D array
float** create_arr(int w, int h){
	float** arr = new float*[h];
	for (int i = 0; i < h; i++)
		arr[i] = new float[w];
	set_zero(w, h, arr);
	return arr;
}

//Pad w/zeros
float** pad_arr(int w, int h, int n, float** arr){
	float** pad_arr = create_arr(w + 2*n, h + 2*n);
	for (int i = 0; i < h; i++)
		for (int j = 0; j < w; j++)
			pad_arr[i + n][j + n] = arr[i][j];
	return pad_arr;
}

//Return largest number in arr
float get_max(int w, int h, float** arr){
	float max = 0.0;
	for(int i = 0; i < h; i++)
		for (int j = 0; j < w; j++)
			if(arr[i][j] > max)
				max = arr[i][j];
	return max;
}

//Return result of matrix convolution at a point
float calc_val(int y, int x, float** arr){
	float sum = 0.0;
	for (int m = 0; m < 9; m++)
		for(int n = 0; n < 9; n++)
			sum += loG[m][n] * arr[y + m][x + n];
	return(sqrt(sum*sum));
}

//Return min-val index at row, between col range[lo, hi], of arr[][]
int find_mindx(int row, int lo, int hi, float** arr, int w){
	int curr, indx;
	int min = INT_MAX;
	for (int col = lo; col <= hi; col++){
		//Pixel coords OOB
		if ((col < 0) || (col > w - 1))
			continue;
		curr = arr[row][col];
		if (curr < min){
			min = curr;
			indx = col;
		}
	}
	return indx;
}

//--------------------------FN PROTOTYPES---------------------------
//Initialize temp_img w/src pixel vals and grey_img w/pixel luminance
void init_img(CImg<float> src, float** grey_img, vector<vector<Pixel>> &temp_img);
//Calculates LoG across pixels--init false to calculate LoG for seam only
void calcLoG(int w, int h, float** grey, float** dst, int* seam, bool init);
//Find the seam--contains ind of seam at each row
void update_seam(int w, int h, float** total_e, int* seam);
//Get the mask and apply it
void get_mask(int w, int h, float** dst, CImg<float> src);

//Returns an array of thresholded values(0->255)
float** thresh(int w, int h, float** vals);
//Return array with cumulative min of adjacent pixels
float** accumulate(int w, int h, float** vals);
//Returns copy of arr vals with seam removal
float** remove_seam(int w, int h, int* seam, float** arr);

//--------------------------------DRIVER----------------------------
int main(int argc, char** argv){
	//----------------------CImg cli arguments----------------------
	cimg_usage("Get arguments");
	const char* fname = cimg_option("-i", (char*)0, "input image file");
	const char* output = cimg_option("-o", (char*)0, "output image file");
	const int percent = cimg_option("-p", 10, "percentage to remove");
	const bool mask = cimg_option("-m", false, "apply a mask");

	//-----------------Image and energy components------------------
	CImg<float> img(fname);
	int w = img.width();
	int h = img.height();
	vector<vector<Pixel>> temp_img(h, vector<Pixel> (w, {0, 0, 0}));
	float** grey = create_arr(w, h);
	float** energy = create_arr(w, h);
	init_img(img, grey, temp_img);
	
	//-----------------------Seam reduction-------------------------
	double per = percent/100.0;
	int num_seams = floor(per * w);
	//Seam will always be of length of img height
	int* seam = new int[h];
	//Initialize
	calcLoG(w, h, grey, energy, seam, true);
	if (mask)
		get_mask(w, h, energy, img);
	
	for (int i = 0; i < num_seams; i++){
		float** vals = thresh(w, h, energy);
		float** total_e = accumulate(w, h, vals);
		update_seam(w, h, total_e, seam);
		energy = remove_seam(w, h, seam, energy);
		grey = remove_seam(w, h, seam, grey);
		//Remove seam from ref color image
		for(int k = 0; k < h; k++)
			temp_img[k].erase(temp_img[k].begin() + seam[k]);
		//Decrement width
		w--;
		//Update energy following seam removal
		calcLoG(w, h, grey, energy, seam, false);
		delete_arr(h, total_e);
		delete_arr(h, vals);
	}
	delete_arr(h, energy);
	delete_arr(h, grey);
	delete[] seam;

	//---------------------------Export-----------------------------
	CImg<float> final_img(w, h, 1, 3);
	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++){
			final_img(x, y, 0) = temp_img[y][x].r;
			final_img(x, y, 1) = temp_img[y][x].g;
			final_img(x, y, 2) = temp_img[y][x].b;
		}
	}
	final_img.save(output);
	return 0;
}

void init_img(CImg<float> src, float** arr, vector<vector<Pixel>> &temp_img){
	float r, g, b;
	int w = src.width();
	int h = src.height();
	int num_channels = src.spectrum();
	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++){
			if (num_channels == 1){
				r = g = b = src(x, y);
			}else{
				r = (float)src(x, y, 0);
				g = (float)src(x, y, 1);
				b = (float)src(x, y, 2);
			}
			Pixel curr = {r, g, b};
			temp_img[y][x] = curr;
			arr[y][x] = (0.3*r + 0.59*g + 0.11*b);
		}
	}
}

void get_mask(int w, int h, float** dst, CImg<float> src){
	float white[] = {255, 255, 255}; 
  float red[] = {255, 0, 0};
	CImg<float> mask(w, h, 1, 1, 0);
	CImgDisplay dsp(src,"Draw Mask"); 
    //Display window for masking 
    while (!dsp.is_closed() && !dsp.is_keyQ()) {
			dsp.resize(true);
			int m_x, m_y;
			if(dsp.button()){ 
        	m_x = dsp.mouse_x() * w/dsp.width(); 
        	m_y = dsp.mouse_y() * h/dsp.height();
          src.draw_circle(m_x, m_y, 15, red, 0.05); 
          mask.draw_circle(m_x, m_y, 15, white, 0.05); 
          dsp.display(src); 
        }
			dsp.wait();
    }
	//Add mask vals to dst array
	for (int i = 0; i < h; i++)
		for (int j = 0; j < w; j++)
			dst[i][j] += (mask(j, i, 0)*255.0);
	
}

void calcLoG(int w, int h, float** grey, float** dst, int* seam, bool init){
	float** tmp = pad_arr(w, h, 4, grey);
	for (int i = 0; i < h; i++){
		//If initializing, calculate values for every pixel
		if (init){
			for (int j = 0; j < w; j++)
				dst[i][j] = calc_val(i, j, tmp);
		//Calculate value for only seam pixel in each row
		}else{
			int j = seam[i];
			//Seam was on right edge; decrement
			if(j == w)
				j = w - 1;
			dst[i][j] = calc_val(i, j, tmp);
		}
	}
	delete_arr(h + 8, tmp);
}

void update_seam(int w, int h, float** total_e, int* seam){
	//Find min of entire bottom row
	int i = h-1;
	int prev = find_mindx(i, 0, w - 1, total_e, w);
	seam[i] = prev;
	//Find min starting at prev to top
	for (int i = h - 2; i >= 0; i--){
		int curr = find_mindx(i, prev - 1, prev + 1, total_e, w);
		seam[i] = curr;
		prev = curr;
	}
}

float** thresh(int w, int h, float** vals){
	float max = get_max(w, h, vals);
	float** ret = create_arr(w, h);
	for (int i = 0; i < h; i++){
		for (int j = 0; j < w; j++){
			float val = (vals[i][j]/max) * 255.0;
			if (val > 40.0)
				ret[i][j] = 255.0;
			else
				ret[i][j] = val * 3.0;
		}
	}
	return ret;
}

float** accumulate(int w, int h, float** vals){
	float** sigma = create_arr(w, h);
	for(int i = 0; i < h; i++){
		for(int j = 0; j < w; j++){
			//Copy all vals in first row
			if (i == 0){
				sigma[i][j] = vals[i][j];
				continue;
			}else{
				int col = find_mindx(i - 1, j - 1, j + 1, sigma, w);
				sigma[i][j] = sigma[i - 1][col] + vals[i][j];
			}
		}
	}
	return sigma;
}

float** remove_seam(int w, int h, int* seam, float** arr){
	float** new_arr = create_arr(w - 1, h);
	//Copy over vals, excluding cols where seam was located
	for (int i = 0; i < h; i++){
		int indx = seam[i];
		for (int j = 0, indx_ref = 0; j < w - 1; j++, indx_ref++){
			if (j == indx)
				++indx_ref;
			new_arr[i][j] = arr[i][indx_ref];	
		}
	}
	delete_arr(h, arr);
	return new_arr;
}
