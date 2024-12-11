#ifndef BILINEARINTERPOLATION_H
#define BILINEARINTERPOLATION_H

#include "tiffio.h"

uint8_t bilinear_interpolate(uint8_t* pixels, uint32_t width, uint32_t height, float x, float y);

#endif