<snippet>
  <content>
# Height Fields
## Demo
Demo: https://www.youtube.com/watch?v=8gTvvWlwdmY
<br />
## Introduction
Height fields may be found in many applications of computer graphics. They are used to represent terrain in video games and simulations, and also often utilized to represent data in three dimensions. In this project, height fields will be created from grey scale images.
## Features
1. Support color (bpp=3) in input images.
2. Just use a color image with the same command. 
2. Color the vertices based on color values taken from another image of equal size with smooth color gradients. Just provide another image in the 2nd parameter and press '5'.
3. Texturemap the surface with an arbitrary image. Just provide another image in the 2nd parameter and press '6'.

## How To Run My Code

1. One parameter: 
./assign1 spiral.jpg
2. Two parameters:
./assign1 spiral.jpg trojan.jpg

For both one parameter and two parameters, the first parameter is used to build the 3D model, i.e. calculate the z value from it's greyscale pixel value (bpp = 1) or RGB value (bpp = 3 or 4). 

For the two parameter case, the 2nd parameter is also a image. 
The 2nd image can be used in two ways:
1. Color the vertices of 1st image with the 2nd image's corresponding color
2. Texture mapping the 3D model with the 2nd image

Keyboard Operations:

Transform:
Mouse Left: rotate around x & y axis
Mouse Middle: rotate around z axis
Control + Mouse Left: Move along x & y axis
Control + Mouse Middle: Move along z axis
Shift + Mouse Left: Scale along x & y axis
Shift + Mouse Middle: Scale along z axis

Rendering Mode Switching (Number Keys Are Used):
'1': Render as Points
'2': Render as Lines
'3': Render as Solid Triangles
'4': Toggle between Enable and Disable: GL_POINT_SMOOTH, GL_LINE_SMOOTH, GL_POLYGON_SMOOTH
'5': Toggle between two different color schemes: 1. color based on greyscale or RGB value (the original one) 2. color based on color values taken from another image
'6': Toggle between enabling texture mapping and disabling texture mapping
'a': start to save screenshot (start number and end number of screenshot finename can be set in the program)

If there is no 2nd parameter when we ran the program, '5' and '6' are useless.

If it's in 'Texture Mapping' mode, pressing '5' will take no effect, i.e. texture mapping and color the 3D model based on another image's color can not happen at the same time.

></content>
  <tabTrigger></tabTrigger>
</snippet>







