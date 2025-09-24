#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "seamcarving.h"
#include "c_img.h"
#include <stdio.h>

// Calculate energy map of the image
void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    int height = im->height;
    int width = im->width;
    create_img(grad, height, width);
    int xr = 0;
    int xg = 0;
    int xb = 0;
    int yr = 0;
    int yg = 0;
    int yb = 0;
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++ ){
                int left = (x-1+width) % (width);
                int right = (x+1) % (width);
                int up = (y-1+height) % (height);
                int down = (y+1) % (height);     
                xr = get_pixel(im, y, left, 0) - get_pixel(im, y, right, 0);
                xg = get_pixel(im, y, left, 1) - get_pixel(im, y, right, 1);
                xb = get_pixel(im, y, left, 2) - get_pixel(im, y, right, 2);
                yr = get_pixel(im, down, x, 0) - get_pixel(im, up, x, 0);
                yg = get_pixel(im, down, x, 1) - get_pixel(im, up, x, 1);
                yb = get_pixel(im, down, x, 2) - get_pixel(im, up, x, 2);
            float temp = xr*xr + xg*xg + xb*xb + yr*yr + yg*yg + yb*yb;
            uint8_t e_10 = (int)sqrt(temp)/10;
            set_pixel(*grad, y, x, e_10, e_10, e_10);
        }
    }
}

// Compute dynamic programming table of best seam energies
void dynamic_seam(struct rgb_img *grad, double **best_arr){
    int height = grad->height;
    int width = grad->width;
    int left = 0;
    int right = 0;
    int down = 0;
    int up = 0;
    *best_arr = (double *)malloc(sizeof(double)*height*width);
    for(int y = 0; y<height; y++){
        for(int x = 0; x<width; x++){
            if(y==0){
                (*best_arr)[y*(width)+x] = get_pixel(grad, y, x, 0);
            }
            else{
                if(x==0){
                    left = x;
                }else{
                    left = x-1;
                }
                if(x==width-1){
                    right = x;
                }else{
                    right = x+1;
                }
                up = y-1;

                double down_left = (*best_arr)[up*width + left];
                double down_middle = (*best_arr)[up*width + x];
                double down_right = (*best_arr)[up*width + right];

                (*best_arr)[y*width + x] = (double)fmin(down_left, fmin(down_middle, down_right)) + (double)get_pixel(grad, y, x, 0);
            }
        }
    }
}

// Recover the path of the minimum-energy seam
void recover_path(double *best, int height, int width, int **path){
    int left = 0;
    int right = 0;
    int down = 0;
    int min_x = 0;
    
    // Find minimum in last row
    for(int x = 0; x < width; x++){
        if(best[(height-1)*width + x] < best[(height-1)*width + min_x]){
            min_x = x;
        }
    }

    *path = (int *)malloc(sizeof(int) * height);
    (*path)[height - 1] = min_x;

    // Backtrack seam path
    for(int y = height - 2; y >= 0; y--){
        int x = (*path)[y + 1];
        left = (x == 0) ? x : x - 1;
        right = (x == width - 1) ? x : x + 1;

        double down_left = best[y * width + left];
        double down_middle = best[y * width + x];
        double down_right = best[y * width + right];

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
    create_img(dest, height, width-1);
    for(int y = 0; y<height; y++){
        int ind_remove = path[y];
        for(int x = 0; x<ind_remove; x++){
            uint8_t r = get_pixel(src, y, x, 0);
            uint8_t g = get_pixel(src, y, x, 1);
            uint8_t b = get_pixel(src, y, x, 2);
            set_pixel(*dest, y, x, r, g, b);
        }
        for(int x = ind_remove+1; x<width; x++){
            uint8_t r = get_pixel(src, y, x, 0);
            uint8_t g = get_pixel(src, y, x, 1);
            uint8_t b = get_pixel(src, y, x, 2);
            set_pixel(*dest, y, x-1, r, g, b);
        }
    }
}

int main() {
    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    // Read initial image
    read_in_img(&im, "HJoceanSmall.bin");
    
    // Perform seam carving 100 times
    for(int i = 0; i < 100; i++) {
        printf("i = %d\n", i);
        calc_energy(im, &grad);               // Energy map
        dynamic_seam(grad, &best);            // DP seam table
        recover_path(best, grad->height, grad->width, &path); // Seam path
        remove_seam(im, &cur_im, path);       // Remove seam

        if(i == 99) {                         // Save final image
            char filename[200];
            sprintf(filename, "img%d.bin", i);
            write_img(cur_im, filename);
        }

        // Free memory
        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);

        im = cur_im; // Update for next iteration
    }

    destroy_image(im); // Clean up final image

    return 0;
}
