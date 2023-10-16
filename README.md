# Seam Carving
<p align="center">
  <img src="output/gifs/nightcity.gif"/>
</p>

## What
Seam carving is a method for implementing content-aware resizing or image retargeting. It aims to reduce the height or width of an image without losing information or altering the subject's proportions, major drawbacks of cropping and unlocking aspect-ratio respectively. 

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
