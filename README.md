// Copyright Traistaru Dragos-Andrei
# Image-Editor---HW3

## Code Structure and Key Functions

# 1. File: `tema3.c`
This is the main source file that contains the implementation of the image editor. It defines all commands and operations that can be performed on images.

The `utils.h` file is critical for modularity, ensuring that all constants and shared structures are accessible across different parts of the program.

- **`clamp`**: Ensures pixel values are within the range [0, 255].

# 2. Core Structures
`image_struct`
- Represents an image and contains metadata such as width, height, max color value, format, and a matrix of pixels (`pixel_struct`).
- The pixel matrix is dynamically allocated based on the dimensions of the image.

`pixel_struct`
- Represents a single pixel, with `red`, `green`, and `blue` values for color images. For grayscale images, the same structure is used with all channels having the same value.

# 3. Implementation Details
# Loading Images (`load_image` function)
- Handles the loading of images in NetPBM formats (P2, P3, P5, P6).
- Dynamically allocates memory for the image structure and its pixel matrix.
- Processes headers to extract format, dimensions, and color information.
- Reads pixel data depending on the format (binary or ASCII).
- Example:
  - For `P3` (ASCII RGB), it reads three integers per pixel.
  - For `P6` (Binary RGB), it reads three bytes per pixel.
- Includes validation to handle unsupported formats.

# Saving Images (`save_image` function)
- Saves the current image in the desired format (ASCII or binary).
- Handles conversion between formats, e.g., saving a `P5` (binary grayscale) as `P2` (ASCII grayscale).
- Writes the header followed by pixel data in the appropriate format.

# Selecting Regions (`select_pixels` function)
- Allows the user to select a rectangular region within the image.
- Dynamically allocates memory for a new image containing only the selected region.
- Before the selection the coordinates are validated in the main function to ensure the selected region is within bounds.

# Cropping (`CROP` command)
- Crops the image to the selected region by replacing the current image with the selected portion.
- Frees memory associated with the original image to prevent leaks.

# Applying Filters (`apply_filter` function)
- Applies 3x3 convolution kernels to the selected region of the image.
- Supports filters like `EDGE`, `SHARPEN`, `BLUR`, and `GAUSSIAN_BLUR`.
- Handles edge cases where pixels at the boundary of the image may not have sufficient neighbors. These pixels remain unchanged.
- Uses a temporary matrix to store intermediate results and avoids overwriting original pixel values during computation.
- After applying the filter, the `merge_images` function is used to update the original image (copy_image) with the filtered region. This ensures the changes are reflected in the overall image while preserving unselected areas.

# Rotating Images (`image_rotation` function)
- Rotates the selected region of the image by 90, 180, or 270 degrees.
- Requires the selected region to be square; otherwise, it prints an error message.
- Uses matrix transposition and column flipping to perform rotations.

# Rotating Images (`image_rotation` function)
- Rotates the selected region by 90, 180, or 270 degrees.
- First, the region is transposed (rows become columns).
- Depending on the angle, columns or rows are reversed to achieve the correct orientation.
- After rotation, the merge_images function is called to update the original image (copy_image) with the rotated selection. This ensures that only the selected region is rotated, leaving the rest of the image unchanged.
- Requires the selected region to be square; otherwise, it prints an error message.

# Histogram (`occurrences` function)
- A frequency array (`vf`) is used to count occurrences of each intensity value (0-255).
- The maximum frequency is determined to normalize histogram values.
- The intensity values are divided into intervals (x and y parameters).
- A scaled histogram is printed using * to represent the intensity frequencies.

# Equalization (`equalize` function)
- Count how many times each intensity value (0-255) appears in the image.
- For each intensity, calculate the cumulative sum of the histogram values.
- Normalize to ensure the intensity values span [0, 255].
- Replace each pixel's intensity in the matrix with its new value.

# Memory Management
- The program uses dynamic memory allocation for images and their pixel matrices.
- Functions like `free_image` are used to clean up resources when images are no longer needed, preventing memory leaks.
