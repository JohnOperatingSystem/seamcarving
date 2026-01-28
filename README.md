# Seamcarving Image Resizer

## Description
This project implements **Seam Carving** for content-aware image resizing. The program removes low-energy vertical seams from an image to reduce its width while keeping important features intact.

**Steps:**
1. Compute **energy map** using dual-gradient (RGB differences).
2. Use **dynamic programming** to find the minimum-energy vertical seam.
3. Remove the seam from the image.
4. Repeat for a specified number of seams.

## Files
- `seamcarving.c / seamcarving.h` – Core functions:
  - `calc_energy()`
  - `dynamic_seam()`
  - `recover_path()`
  - `remove_seam()`
- `c_img.c / c_img.h` – Image handling:
  - `create_img()`, `destroy_image()`
  - `read_in_img()`, `write_img()`
  - `get_pixel()`, `set_pixel()`
- `main.c` – Program entry point: loops over seam removal iterations.

## Input & Output
- **Input image**: Raw binary format containing:
  - 2-byte height
  - 2-byte width
  - RGB raster data (3 bytes per pixel)  
  Example: `"sample.bin"`  

- **Output image**: Raw binary file `"final_image.bin"`
