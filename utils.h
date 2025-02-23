#ifndef UTILS_H
#define UTILS_H

#define KERNEL_SIZE 3

typedef struct {
	int r;
	int g;
	int b;
} pixel_struct; // Pixel with RGB channels

typedef struct {
	int width;
	int height;
	int max_color;
	char format[3];
	pixel_struct **pixels; // Matrix of pixels that form the image color
} image_struct;

const int EDGE[KERNEL_SIZE][KERNEL_SIZE] = {
{-1, -1, -1},
{-1,  8, -1},
{-1, -1, -1}
};

const int SHARPEN[KERNEL_SIZE][KERNEL_SIZE] = {
{ 0, -1,  0},
{-1,  5, -1},
{ 0, -1,  0}
};

const int BLUR[KERNEL_SIZE][KERNEL_SIZE] = {
{1, 1, 1},
{1, 1, 1},
{1, 1, 1}
};

const int GAUSSIAN_BLUR[KERNEL_SIZE][KERNEL_SIZE] = {
{1, 2, 1},
{2, 4, 2},
{1, 2, 1}
};

#endif
