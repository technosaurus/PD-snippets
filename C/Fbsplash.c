#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <image_path>\n", argv[0]);
        return 1;
    }

    // 1. Open the Linux Framebuffer device node
    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd < 0) {
        perror("Error opening /dev/fb0");
        return 1;
    }

    // 2. Query target variable hardware screenspace information
    struct fb_var_screeninfo vinfo;
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        perror("Error reading variable information");
        close(fb_fd);
        return 1;
    }

    // 3. Load the image asset via stb_image
    int img_w, img_h, img_channels;
    unsigned char* img_data = stbi_load(argv[1], &img_w, &img_h, &img_channels, 4); // Force 4 channels (RGBA)
    if (!img_data) {
        fprintf(stderr, "Error: Could not load image %s\n", argv[1]);
        close(fb_fd);
        return 1;
    }

    // 4. Calculate total memory footprint and memory-map (mmap) the framebuffer 
    size_t screensize = vinfo.yres_virtual * vinfo.xres_virtual * (vinfo.bits_per_pixel / 8);
    unsigned char* fb_ptr = (unsigned char*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if ((intptr_t)fb_ptr == -1) {
        perror("Error memory mapping the framebuffer device");
        stbi_image_free(img_data);
        close(fb_fd);
        return 1;
    }

    // 5. Draw the image with bounds clipping to match the real screen configuration
    int render_w = (img_w < vinfo.xres) ? img_w : vinfo.xres;
    int render_h = (img_h < vinfo.yres) ? img_h : vinfo.yres;
    int bpp = vinfo.bits_per_pixel;

    for (int y = 0; y < render_h; y++) {
        for (int x = 0; x < render_w; x++) {
            // Source index in stb_image RGBA flat memory array
            long img_idx = (y * img_w + x) * 4;
            unsigned char r = img_data[img_idx];
            unsigned char g = img_data[img_idx + 1];
            unsigned char b = img_data[img_idx + 2];

            // Target byte offset mapping inside the system framebuffer memory
            long fb_idx = (x + vinfo.xoffset) * (bpp / 8) + (y + vinfo.yoffset) * vinfo.line_length;

            if (bpp == 32) {
                // Handle common TrueColor configurations (BGRA or RGBA shift matrices)
                fb_ptr[fb_idx + (vinfo.red.offset / 8)]   = r;
                fb_ptr[fb_idx + (vinfo.green.offset / 8)] = g;
                fb_ptr[fb_idx + (vinfo.blue.offset / 8)]  = b;
                fb_ptr[fb_idx + (vinfo.transp.offset / 8)] = 255;
            } 
            else if (bpp == 16) {
                // Handle RGB565 high-color targets
                unsigned short r5 = (r >> 3) & 0x1F;
                unsigned short g6 = (g >> 2) & 0x3F;
                unsigned short b5 = (b >> 3) & 0x1F;
                unsigned short rgb565 = (r5 << 11) | (g6 << 5) | b5;
                *((unsigned short*)(fb_ptr + fb_idx)) = rgb565;
            }
        }
    }

    // Clean up allocations
    munmap(fb_ptr, screensize);
    stbi_image_free(img_data);
    close(fb_fd);
    return 0;
}
