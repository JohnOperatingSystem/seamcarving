#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "seamcarving.h"
#include "c_img.h"
#include <stdio.h>

/*function computes the dual gradient energy function and places it in struct rgb_img *grad*/
void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    int height = im->height;
    int width = im->width;
    create_img(grad, height, width); // creates image with height & width
    int xr = 0 , xg = 0, xb = 0, yr = 0, yg = 0, yb = 0;
    // loop through every pixel to compute gradient
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++ ){
                int left = (x-1+width) % (width); // x coord of pixel left of (x,y)
                int right = (x+1) % (width); // x coord of pixel right of (x,y)
                int up = (y-1+height) % (height); // y coord of row above (x,y)
                int down = (y+1) % (height); // y coord of row below (x,y)

                // compute the change of each color rgb in each direction
                xr = get_pixel(im, y, left, 0) - get_pixel(im, y, right, 0);
                xg = get_pixel(im, y, left, 1) - get_pixel(im, y, right, 1);
                xb = get_pixel(im, y, left, 2) - get_pixel(im, y, right, 2);
                yr = get_pixel(im, down, x, 0) - get_pixel(im, up, x, 0);
                yg = get_pixel(im, down, x, 1) - get_pixel(im, up, x, 1);
                yb = get_pixel(im, down, x, 2) - get_pixel(im, up, x, 2);
            float temp = xr*xr + xg*xg + xb*xb + yr*yr + yg*yg + yb*yb; // raw energy of pixel
            uint8_t e_10 = (int)sqrt(temp)/10; // get energy and store in array
            set_pixel(*grad, y, x, e_10, e_10, e_10);
        }
    }
}

/*function allocates and computes dynamic array *best_arr. (*best_arr)[i*width+j] contains minimum cost of seam from top of grad to point (i,j)*/
void dynamic_seam(struct rgb_img *grad, double **best_arr){
    int height = grad->height;
    int width = grad->width;
    int left = 0, right = 0, down = 0, up = 0;
    // allocate memory for array of size number of pixels. with indices being i*width+j
    *best_arr = (double *)malloc(sizeof(double)*height*width);
    for(int y = 0; y<height; y++){
        for(int x = 0; x<width; x++){
            if(y==0){
                (*best_arr)[y*(width)+x] = get_pixel(grad, y, x, 0); // top row: no pixel above, so best seam energy is the pixel itself
            }
            else{
                if(x==0){ // if pixel on left edge, must be pixel itself, else is pixel on left
                    left = x;
                }else{
                    left = x-1;
                }
                if(x==width-1){ // if pixel on right edge, must be pixel itself, else is pixel on right
                    right = x;
                }else{
                    right = x+1;
                }
                up = y-1; // the row above current row

                double down_left = (*best_arr)[up*width + left];
                double down_middle = (*best_arr)[up*width + x];
                double down_right = (*best_arr)[up*width + right];

                // compute current pixel's cumulative minimum energy and store in array
                (*best_arr)[y*width + x] = (double)fmin(down_left, fmin(down_middle, down_right)) + (double)get_pixel(grad, y, x, 0);
            }
        }
    }
}

/*function that returns the path of the minimum energy seam*/
void recover_path(double *best, int height, int width, int **path){
    int left = 0, right = 0, down = 0, min_x = 0;
    
    // Find minimum in last row
    for(int x = 0; x < width; x++){
        if(best[(height-1)*width + x] < best[(height-1)*width + min_x]){
            min_x = x;
        }
    }
    // allocate an array with same amount of terms as rows in image 
    *path = (int *)malloc(sizeof(int) * height);
    (*path)[height - 1] = min_x;

    // Backtrack seam path from bottom to top
    for(int y = height - 2; y >= 0; y--){ // start one row above from bottom
        int x = (*path)[y + 1];
        left = (x == 0) ? x : x - 1; // if pixel on left edge, must be pixel itself, else is pixel on left
        right = (x == width - 1) ? x : x + 1; // if pixel on right edge, must be pixel itself, else is pixel on right
        
        // cumulative energies of seams at the 3 pixels above the pixel
        double down_left = best[y * width + left];
        double down_middle = best[y * width + x];
        double down_right = best[y * width + right];

        // choose minimum energy pixel
        if(down_left <= down_middle && down_left <= down_right){
            (*path)[y] = left;
        }
        else if(down_middle <= down_left && down_middle <= down_right){
            (*path)[y] = x;
        }
        else{
            (*path)[y] = right;
        }
    }
}

// Remove the identified seam from the image
void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
    int height = src->height;
    int width = src->width;
    create_img(dest, height, width-1); // create image with same height but width reduced by 1 pixel

    // loop over each row
    for(int y = 0; y<height; y++){
        int ind_remove = path[y]; // column index of the seam pixel to remove in this row
        // copy all the pixels before the pixel that is being removed
        for(int x = 0; x<ind_remove; x++){
            uint8_t r = get_pixel(src, y, x, 0);
            uint8_t g = get_pixel(src, y, x, 1);
            uint8_t b = get_pixel(src, y, x, 2);
            set_pixel(*dest, y, x, r, g, b);
        }
        // copy all the pixels after the pixel that is being removed
        for(int x = ind_remove+1; x<width; x++){
            uint8_t r = get_pixel(src, y, x, 0);
            uint8_t g = get_pixel(src, y, x, 1);
            uint8_t b = get_pixel(src, y, x, 2);
            set_pixel(*dest, y, x-1, r, g, b); // shift left to remove seam
        }
    }
}
// testing 
int main() {
    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    // Read initial image
    read_in_img(&im, "6x5.bin"); // insert file name

    int iterations = 5;

    for (int i = 0; i < iterations; i++) {
        printf("Iteration: %d\n", i);

        // Calculate energy map (grad is allocated inside)
        calc_energy(im, &grad);

        // Compute DP seam table (best allocated inside)
        dynamic_seam(grad, &best);

        // Recover minimum-energy seam path (path allocated inside)
        recover_path(best, grad->height, grad->width, &path);

        // Remove the seam
        remove_seam(im, &cur_im, path);

        // Free old image and temporary arrays
        destroy_image(im);   // old image
        destroy_image(grad); // energy map
        free(best);          // DP table
        free(path);          // seam path

        // Update pointer for next iteration
        im = cur_im;
    }

    // Save final image
    write_img(im, "final_image.bin");

    // Free final image
    destroy_image(im);

    return 0;
}
