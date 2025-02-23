#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

static inline int check_image_loaded(const image_struct *img)
{
	if (!img) {
		printf("No image loaded\n");
		return 0;
	}
	return 1;
}

int unknown_command(image_struct *image)
{
	if (!check_image_loaded(image))
		return 0;
	printf("Invalid command\n");
	return 0;
}

void free_pixels(pixel_struct **pixel, image_struct *image)
{
	for (int i = 0; i < image->height; i++) {
		free(pixel[i]);
	}
	free(pixel);
}

image_struct *load_image(const char *filename)
{
	FILE *file = fopen(filename, "rb");
	if (!file) {
		printf("Failed to load %s\n", filename);
		return NULL;
	}

	// Allocate memory for the image
	image_struct *image = (image_struct *)malloc(sizeof(image_struct));
	if (!image) {
		printf("Memory allocation failed\n");
		fclose(file);
		return NULL;
	}

	// Read format
	fscanf(file, "%2s", image->format);

	// Read dimensions
	fscanf(file, "%d %d %d", &image->width, &image->height, &image->max_color);

	fgetc(file); // Consume the newline after the header

	// Allocate memory for pixel matrix
	image->pixels = malloc(image->height * sizeof(pixel_struct *));
	for (int i = 0; i < image->height; i++) {
		image->pixels[i] = calloc(image->width, sizeof(pixel_struct));
	}

	// Load pixels based on format
	if (strcmp(image->format, "P2") == 0) {
		// ASCII Grayscale
		for (int i = 0; i < image->height; i++) {
			for (int j = 0; j < image->width; j++) {
				int value;
				fscanf(file, "%d", &value);
				image->pixels[i][j].r = value;
				image->pixels[i][j].g = value;
				image->pixels[i][j].b = value;
			}
		}
	} else if (strcmp(image->format, "P3") == 0) {
		// ASCII RGB
		for (int i = 0; i < image->height; i++) {
			for (int j = 0; j < image->width; j++) {
				fscanf(file, "%d", &image->pixels[i][j].r);
				fscanf(file, "%d", &image->pixels[i][j].g);
				fscanf(file, "%d", &image->pixels[i][j].b);
			}
		}
	} else if (strcmp(image->format, "P5") == 0) {
		// Binary Grayscale
		for (int i = 0; i < image->height; i++) {
			for (int j = 0; j < image->width; j++) {
				unsigned char value;
				fread(&value, sizeof(unsigned char), 1, file);
				image->pixels[i][j].r = value;
				image->pixels[i][j].g = value;
				image->pixels[i][j].b = value;
			}
		}
	} else if (strcmp(image->format, "P6") == 0) {
		// Binary RGB
		for (int i = 0; i < image->height; i++) {
			for (int j = 0; j < image->width; j++) {
				fread(&image->pixels[i][j].r, sizeof(unsigned char), 1, file);
				fread(&image->pixels[i][j].g, sizeof(unsigned char), 1, file);
				fread(&image->pixels[i][j].b, sizeof(unsigned char), 1, file);
			}
		}
	} else {
		printf("Unsupported format: %s\n", image->format);
		fclose(file);
		free(image);
		return NULL;
	}

	fclose(file);
	printf("Loaded %s\n", filename);
	return image;
}

void save_image(const char *filename, const image_struct *image, int ascii)
{
	if (!check_image_loaded(image))
		return;
	FILE *file = fopen(filename, "w");
	if (!file) {
		printf("Failed to save %s\n", filename);
		return;
	}
	char format[3];

	if (ascii) {
		if (!strcmp(image->format, "P5"))
			strcpy(format, "P2");
		else if (!strcmp(image->format, "P6"))
			strcpy(format, "P3");
		else
			strcpy(format, image->format);
	} else {
		if (!strcmp(image->format, "P2"))
			strcpy(format, "P5");
		else if (!strcmp(image->format, "P3"))
			strcpy(format, "P6");
		else
			strcpy(format, image->format);
	}

	// Write header
	fprintf(file, "%s\n%d %d\n", format, image->width, image->height);

	fprintf(file, "%d\n", image->max_color);

	// Write pixels based on format
	if (strcmp(format, "P2") == 0) {
		// ASCII Grayscale
		for (int i = 0; i < image->height; i++) {
			for (int j = 0; j < image->width; j++) {
				fprintf(file, "%d ", image->pixels[i][j].r);
			}
			fprintf(file, "\n");
		}
	} else if (strcmp(format, "P3") == 0) {
		// ASCII RGB
		for (int i = 0; i < image->height; i++) {
			for (int j = 0; j < image->width; j++) {
				fprintf(file, "%d %d %d ", image->pixels[i][j].r,
						image->pixels[i][j].g,
						image->pixels[i][j].b);
			}
			fprintf(file, "\n");
		}
	} else if (strcmp(format, "P5") == 0) {
		// Binary Grayscale
		for (int i = 0; i < image->height; i++) {
			for (int j = 0; j < image->width; j++) {
				unsigned char value = (unsigned char)image->pixels[i][j].r;
				fwrite(&value, sizeof(unsigned char), 1, file);
			}
		}
	} else if (strcmp(format, "P6") == 0) {
		// Binary RGB
		for (int i = 0; i < image->height; i++) {
			for (int j = 0; j < image->width; j++) {
				fwrite(&image->pixels[i][j].r, sizeof(unsigned char), 1, file);
				fwrite(&image->pixels[i][j].g, sizeof(unsigned char), 1, file);
				fwrite(&image->pixels[i][j].b, sizeof(unsigned char), 1, file);
			}
		}
	}

	fclose(file);
	printf("Saved %s\n", filename);
}

int clamp(int value)
{
	if (value < 0)
		return 0;
	if (value > 255)
		return 255;
	return value;
}

int apply_filters(image_struct *image, image_struct *copy_image,
							char *filter, int y1,
							int x1)
{
	if (strcmp(image->format, "P3") != 0 && strcmp(image->format, "P6") != 0) {
		printf("Easy, Charlie Chaplin\n");
		return -1;
	}
	// Select the kernel based on the filter parameter
	const int (*kernel)[KERNEL_SIZE];
	int kernel_divisor = 1; // Default divisor for normalization
	if (strcmp(filter, "EDGE") == 0) {
		kernel = EDGE;
	} else if (strcmp(filter, "SHARPEN") == 0) {
		kernel = SHARPEN;
	} else if (strcmp(filter, "BLUR") == 0) {
		kernel = BLUR;
		kernel_divisor = 9; // Sum of BLUR kernel
	} else if (strcmp(filter, "GAUSSIAN_BLUR") == 0) {
		kernel = GAUSSIAN_BLUR;
		kernel_divisor = 16; // Sum of GAUSSIAN_BLUR kernel
	} else {
		printf("APPLY parameter invalid\n");
		return -1;
	}
	// Create a temporary image to store results
	pixel_struct **temp_pixels = malloc(image->height * sizeof(pixel_struct *));
	for (int i = 0; i < image->height; i++)
		temp_pixels[i] = malloc(image->width * sizeof(pixel_struct));
	if (!copy_image) {
		x1 = 0;
		y1 = 0;
		copy_image = image;
	}
	// Apply the kernel
	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			int global_i = y1 + i;
			int global_j = x1 + j;
			// Check if the current pixel has all the necessary neighbors
			if (global_i - 1 < 0 || global_i + 1 >= copy_image->height ||
				global_j - 1 < 0 || global_j + 1 >= copy_image->width) {
				// The pixel does not have all the neighbors ->remains unchanged
				temp_pixels[i][j] = image->pixels[i][j];
				continue;
			}
			int r_sum = 0, g_sum = 0, b_sum = 0;

			for (int ki = 0; ki < KERNEL_SIZE; ki++) {
				for (int kj = 0; kj < KERNEL_SIZE; kj++) {
					int neighbor_i = global_i + ki - 1; // neighbor line
					int neighbor_j = global_j + kj - 1; // neighbor column

					r_sum += copy_image->pixels[neighbor_i][neighbor_j].r *
					kernel[ki][kj];
					g_sum += copy_image->pixels[neighbor_i][neighbor_j].g *
					kernel[ki][kj];
					b_sum += copy_image->pixels[neighbor_i][neighbor_j].b *
					kernel[ki][kj];
				}
			}

			// Normalize and clamp
			temp_pixels[i][j].r = clamp(r_sum / kernel_divisor);
			temp_pixels[i][j].g = clamp(g_sum / kernel_divisor);
			temp_pixels[i][j].b = clamp(b_sum / kernel_divisor);
		}
	}
	// Copy results back to the original image
	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			image->pixels[i][j] = temp_pixels[i][j];
		}
	}
	// Free temporary image memory
	free_pixels(temp_pixels, image);

	printf("APPLY %s done\n", filter);
	return 0;
}

image_struct *select_pixels(int x1, int x2, int y1, int y2, image_struct *image)
{
	image_struct *selected_image = malloc(sizeof(image_struct));
	if (!selected_image) {
		fprintf(stderr, "Memory allocation failed\n");
		return NULL;
	}
	selected_image->height = abs(y2 - y1);
	selected_image->width = abs(x2 - x1);
	selected_image->max_color = image->max_color;
	strcpy(selected_image->format, image->format);
	// Allocate memory for selected pixels
	selected_image->pixels = malloc(selected_image->height *
	sizeof(pixel_struct *));
	for (int i = 0; i < selected_image->height; i++) {
		selected_image->pixels[i] = malloc(selected_image->width *
		sizeof(pixel_struct));
		if (!selected_image->pixels[i]) {
			fprintf(stderr, "Memory allocation failed\n");

			// Free the previously allocated memory
			for (int k = 0; k < i; k++) {
				free(selected_image->pixels[k]);
			}
			free(selected_image->pixels);
			free(selected_image);
			return NULL;
		}
	}

	for (int i = 0; i < selected_image->height; i++) {
		for (int j = 0; j < selected_image->width; j++) {
			selected_image->pixels[i][j].r = image->pixels[i + y1][j + x1].r;
			selected_image->pixels[i][j].g = image->pixels[i + y1][j + x1].g;
			selected_image->pixels[i][j].b = image->pixels[i + y1][j + x1].b;
		}
	}
	printf("Selected %d %d %d %d\n", x1, y1, x2, y2);
	return selected_image;
}

int check_coords(int *x1, int *y1, int *x2, int *y2, image_struct *image)
{
	if (*x1 > *x2) {
		int aux = *x1;
		*x1 = *x2;
		*x2 = aux;
	}
	if (*y1 > *y2) {
		int aux = *y1;
		*y1 = *y2;
		*y2 = aux;
	}
	if (*x1 < 0 || *x1 > image->width || *x2 < 0 || *x2 > image->width ||
		*y1 < 0 || *y1 > image->height || *y2 < 0 || *y2 > image->height ||
		*x1 == *x2 || *y1 == *y2) {
		printf("Invalid set of coordinates\n");
		return 1;
	}
	return 0;
}

void free_image(image_struct *image)
{
	for (int i = 0; i < image->height; i++) {
		if (image->pixels[i])
			free(image->pixels[i]);
	}
	if (image->pixels)
		free(image->pixels);
	if (image)
		free(image);
}

int check_y(int y)
{
	if (y < 2 || y > 256) {
		return 0;
	}
	while (y % 2 == 0) {
		y /= 2;
	}
	if (y != 1) {
		return 0;
	}
	return 1;
}

void print_histogram(int value)
{
	printf("%d\t|\t", value);
	for (int i = 0; i < value; i++) {
		printf("*");
	}
	printf("\n");
}

// Calculate the value frequencies and the maximum frequency.
void calculate_frequencies(image_struct *image, int vf[257])
{
	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			vf[image->pixels[i][j].r]++;
		}
	}
}

// Generate the histogram using the calculated frequencies.
void generate_histogram(int vf[257], int x, int y, int maxx)
{
	int interval = 256 / y;
	for (int i = 0; i < 256; i += interval) {
		int sum = 0;
		for (int j = i; j < i + interval; j++) {
			sum += vf[j];
		}
		int value = (sum * x) / maxx;
		print_histogram(value);
	}
}

// The main function that ties the two functionalities together.
void occurrences(image_struct *image, int x, int y)
{
	if (!check_image_loaded(image))
		return;
	if (strcmp(image->format, "P3") == 0 ||
		strcmp(image->format, "P6") == 0) {
		printf("Black and white image needed\n");
		return;
	} else if (check_y(y) == 0) {
		printf("Invalid set of parameters\n");
		return;
	}

	int vf[257] = {0}, maxx = 0;

	// Calculate the frequencies and the maximum value.
	calculate_frequencies(image, vf);

	// Calculate the maximum frequency.
	int interval = 256 / y;
	for (int i = 0; i < 256; i += interval) {
		int sum = 0;
		for (int j = i; j < i + interval; j++) {
			sum += vf[j];
		}
		if (sum > maxx) {
			maxx = sum;
		}
	}
	// Generate the histogram based on the frequencies.
	generate_histogram(vf, x, y, maxx);
}

int process_sum(int vf[257], int value)
{
	int sum = 0;
	for (int i = 0; i <= value; i++) {
		sum += vf[i];
	}
	return sum;
}

void equalize_image(image_struct *image)
{
	if (!check_image_loaded(image))
		return;
	if (strcmp(image->format, "P3") == 0 ||
		strcmp(image->format, "P6") == 0) {
		printf("Black and white image needed\n");
		return;
	}
	int area, vf[257] = {0};
	area = image->width * image->height;
	calculate_frequencies(image, vf);
	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			int sum = process_sum(vf, image->pixels[i][j].r);
			image->pixels[i][j].r = 255 * sum / area;
		}
	}
	printf("Equalize done\n");
}

void rotate_90(image_struct *image)
{
	pixel_struct **rotated_pixels;
	rotated_pixels = malloc(image->width * sizeof(pixel_struct *));
	if (!rotated_pixels) {
		printf("Memory allocation failed\n");
		return;
	}

	for (int i = 0; i < image->width; i++) {
		rotated_pixels[i] = malloc(image->height * sizeof(pixel_struct));
		if (!rotated_pixels[i]) {
			printf("Memory allocation failed");
			for (int j = 0; j < i; j++) {
				free(rotated_pixels[j]);
			}
			free(rotated_pixels);
			return;
		}
	}

	// Copy pixels in the rotated matrix
	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			rotated_pixels[j][image->height - 1 - i] = image->pixels[i][j];
		}
	}

	for (int i = 0; i < image->height; i++) {
		free(image->pixels[i]);
	}
	free(image->pixels);

	image->pixels = rotated_pixels;

	int temp = image->width;
	image->width = image->height;
	image->height = temp;
}

void image_rotation(int angle, image_struct *image, image_struct **copy_image)
{
	if (*copy_image) {
		if ((*copy_image)->height == image->height &&
			(*copy_image)->width == image->width) {
			free_image(*copy_image);
			(*copy_image) = NULL;
		} else if (image->width != image->height) {
			printf("The selection must be square\n");
			return;
		}
	}
	if (angle % 90 != 0) {
		printf("Unsupported rotation angle\n");
		return;
	}
	int aux_angle = angle;
	if (aux_angle < 0) {
		aux_angle += 360;
	}
	while (aux_angle) {
		rotate_90(image);
		aux_angle -= 90;
	}
	printf("Rotated %d\n", angle);
}

void merge_matr(image_struct *image, image_struct *copy_image,
				int x1, int y1)
{
	// Inserăm matricea B în A
	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			copy_image->pixels[y1 + i][x1 + j] = image->pixels[i][j];
		}
	}
}

// Function to handle the "CROP" command
void handle_crop_command(image_struct **image, image_struct **copy_image)
{
	if (!check_image_loaded(*image)) {
		return; // Exit if no image is loaded
	}

	if (*copy_image) {
		free_image(*copy_image);
		*copy_image = NULL;
	}

	printf("Image cropped\n");
}

// Function to handle the "SELECT ALL" command
void handle_select_all_command(image_struct **image, image_struct **copy_image)
{
	if (!check_image_loaded(*image)) {
		return; // Exit if no image is loaded
	}

	if (*copy_image) {
		free_image(*image);
		*image = *copy_image;
		*copy_image = NULL;
	}

	printf("Selected ALL\n");
}

// Function to handle the operation
void handle_select_pixel(int *x1, int *y1, int *x2, int *y2,
													image_struct **image,
													image_struct **copy_image)
{
	if (!check_image_loaded(*image)) {
		return; // Exit if no image is loaded
	}

	if (!check_coords(x1, y1, x2, y2, *copy_image ? *copy_image : *image)) {
		if (!*copy_image) {
			*copy_image = *image;
			*image = NULL;
		} else if (*image) {
			free_image(*image);
			*image = NULL;
		}

		image_struct *selected_image;
		selected_image = select_pixels(*x1, *x2, *y1, *y2, *copy_image);
		*image = selected_image;
	}
}

// Function to free both images
void free_images(image_struct **copy_image, image_struct **image)
{
	if (*copy_image) {
		free_image(*copy_image);
		*copy_image = NULL; // Set to NULL to prevent dangling pointers
	}
	if (*image) {
		free_image(*image);
		*image = NULL; // Set to NULL to prevent dangling pointers
	}
}

int main(void)
{
	image_struct *image = NULL, *copy_image = NULL;
	char line[256];
	int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
	while (fgets(line, sizeof(line), stdin)) {
		char command[20]; // To store the main command
		char arg1[256], arg2[256], arg3[256], arg4[256]; // Additional arguments
		line[strcspn(line, "\n")] = 0;
		if (sscanf(line, "%s%s%s%s%s", command, arg1, arg2, arg3, arg4) == 5) {
			if (strcmp(command, "SELECT") == 0) {
				if (strcmp(arg4, "abc") == 0) {
					printf("Invalid command\n");
					continue;
				}
				x1 = atoi(arg1), y1 = atoi(arg2),
				x2 = atoi(arg3), y2 = atoi(arg4);
				handle_select_pixel(&x1, &y1, &x2, &y2, &image, &copy_image);
			}
		} else if (sscanf(line, "%s%s%s%s", command, arg1, arg2, arg3) == 4) {
			unknown_command(image);
		} else if (sscanf(line, "%s %s %s", command, arg1, arg2) == 3) {
			if (strcmp(command, "SAVE") == 0) {
				save_image(arg1, copy_image ? copy_image : image, 1);
			} else if (strcmp(command, "HISTOGRAM") == 0) {
				int x = atoi(arg1), y = atoi(arg2);
				occurrences(image, x, y);
			} else
				unknown_command(image);
		} else if (sscanf(line, "%s %s", command, arg1) == 2) {
			if (strcmp(command, "SAVE") == 0) {
				save_image(arg1, copy_image ? copy_image : image, 0);
			} else if (strcmp(command, "LOAD") == 0) {
				if (copy_image) {
					free_image(copy_image);
					copy_image = NULL;
				}
				if (image)
					free_image(image);
				image = load_image(arg1);
			} else if (!strcmp(command, "SELECT") && !strcmp(arg1, "ALL")) {
				handle_select_all_command(&image, &copy_image);
			} else if (strcmp(command, "APPLY") == 0) {
				if (!check_image_loaded(image))
					continue;
				int result = apply_filters(image, copy_image, arg1, y1, x1);
				if (copy_image && result == 0)
					merge_matr(image, copy_image, x1, y1);
			} else if (strcmp(command, "ROTATE") == 0) {
				int angle = atoi(arg1);
				if (!check_image_loaded(image))
					continue;
				image_rotation(angle, image, &copy_image);
				if (copy_image)
					merge_matr(image, copy_image, x1, y1);
			} else
				unknown_command(image);
		} else if (sscanf(line, "%s", command) == 1) {
			if (strcmp(command, "EQUALIZE") == 0) {
				equalize_image(copy_image ? copy_image : image);
			} else if (strcmp(command, "CROP") == 0) {
				handle_crop_command(&image, &copy_image);
			} else if (strcmp(command, "EXIT") == 0) {
				if (!check_image_loaded(image))
					continue;
				goto exit_loop;
			} else
				unknown_command(image);
		}
	}
exit_loop:
	free_images(&copy_image, &image);
	return 0;
}
