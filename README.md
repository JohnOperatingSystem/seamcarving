# Seam Carving in C

This project implements content-aware image resizing using the seam carving algorithm. It reads binary RGB images, calculates pixel energy, finds minimum-energy vertical seams with dynamic programming, removes them, and repeats to reduce image width while preserving important content.

## Files
- **c_img.c / c_img.h**: Handles RGB image operations.
  - `create_img()`: Allocates memory for a new image.
  - `read_in_img()`: Reads a binary RGB image from a file.
  - `write_img()`: Writes an RGB image to a file.
  - `get_pixel()`: Returns the value of a pixel at a given coordinate and color channel.
  - `set_pixel()`: Sets the RGB values of a pixel.
  - `destroy_image()`: Frees allocated memory for an image.
  - `print_grad()`: Prints a gradient image (used for debugging).

- **seamcarving.c**: Implements seam carving logic.
  - `calc_energy()`: Computes the energy map of an image using the gradient magnitude of RGB channels.
  - `dynamic_seam()`: Uses dynamic programming to compute the cumulative minimum energy for vertical seams.
  - `recover_path()`: Backtracks the dynamic programming table to recover the lowest-energy vertical seam.
  - `remove_seam()`: Removes the identified vertical seam from the image.
  - `main()`: Reads the input image, iteratively removes 100 seams, and writes the final output.

## Usage
1. Compile:
   ```bash
   gcc seamcarving.c c_img.c -o seamcarving -lm
