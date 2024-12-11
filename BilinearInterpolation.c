#include "tiffio.h"

uint8_t bilinear_interpolate(uint8_t* pixels, uint32_t width, uint32_t height, float x, float y){
    uint32_t x1, x2, y1, y2;
    x1 = (uint32_t) x;
    x2 = x1 + 1;
    y1 = (uint32_t) y;
    y2 = y1 + 1;
    if (x2 >= width) x2 = width - 1;
    if (y2 >= height) y2 = height - 1;
    
    return (
        (uint8_t)(pixels[y1 * width + x1] * (x2 - x) * (y2 - y) +
        pixels[y1 * width + x2] * (x - x1) * (y2 - y) +
        pixels[y2 * width + x1] * (x2 - x) * (y - y1) +
        pixels[y2 * width + x2] * (x - x1) * (y - y1))

    );
}
