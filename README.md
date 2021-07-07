# Seam Carving
<p align="center">
  <img src="output/gifs/nightcity.gif"/>
</p>

## What
Seam carving, a method for implementing content-aware resizing or image retargeting, aims to reduce the height or width of an image w/o loss of detail(i.e. cropping) or proportion(i.e. squeezing/stretching).

## How
### Define and apply an energy function e(x, y) to an image.  
I decided to convert the image to greyscale and convole it with a LoG([Laplacian of Gaussian](https://homepages.inf.ed.ac.uk/rbf/HIPR2/log.htm)) operator since it both reduces noise and detects edges, but there are a lot of ways to go about that.

### Main loop
#### 1. Calculate the cumulative energy map  
Sum the lowest value pixel directly above and adjacent to it. Since there isn't a row above the first, those values will always remain the same. Pseudo code looks something like this:  
```
From row = 1 to height:  
	sigma_e(row, col) = e(row, col) + min(e(row - 1, col - 1), e(row - 1, col), e(row - 1, col + 1)) 
```
Ex:
```
	1 | 3 | 2	1 | 3 | 2  
	_________	_________  
	2 | 1 | 5 ----> 3 | 2 | 7  
	_________	_________    
	1 | 5 | 7	3 | 7 | 9
```

#### 2. Seam removal
You basically use the same procedure as above but in reverse row order. To find the start of the seam, traverse every col in the bottom row and find the min. Then you only have to find the minimum of the three pixels above and adjacent to it. Pseudo code:  
```
row = height  
seam[row] = min(sigma_e(height, col = 0 to width))  

From row = height to 1:  
	col = seam[row]  
	seam[row - 1] = min(sigma_e(row - 1, col - 1), sigma_e(row - 1, col), sigma_e(row - 1, col + 1))
```

#### 3. Update energy values along the seam
Pixels along the seam have new neighbors, so their energy values are no longer accurate. Re-apply your energy function at all of the pixels on the seam. I derived my e(x, y) from the greyscale image, so I keep a working copy of it throughout the program.

## Drawbacks
Without image segmentation or consideration for edge gradient direction, removing too many seams from a "busy" picture is likely to result in output warped beyond recognition. I did include a masking tool to select portions of an image you'd like protected. Below is an example of the process with and w/o a mask:  

| Mask | No Mask | 
|:----------:|:----------:|
|![Masked](output/gifs/child_mask.gif)|![No mask](output/gifs/child_no_mask.gif)| 

## Usage
Compile with makefile  
Requires CImg.h, ImageMagick for image types  

### Options

| command | description |
|:---:|:---:|
| -i | (char*) input path/to/image |
| -o | (char*) output path/to/save |
| -p | (int) percentage of image to resize by |
| -m | (bool) whether or not to apply mask |
