#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "BilinearInterpolation.h"
#include "tiffio.h"

int main(int argc, char *argv[]){
    if(argc != 4){
        fprintf(stderr, "Uso: %s <input_file> <output_file> <new_dpi>\n",argv[0]);
        return 1;
    }

    // O nome da foto original, o nome da foto redimensionado e a nova dpi devem ser informadas na chamada do programa
    const char *input_file = argv[1], *output_file = argv[2];
    float new_dpi = atof(argv[3]);

    if(new_dpi <= 0){
        fprintf(stderr,"DPI value invalid.\n");
        return 1;
    }

    ///////////////////////////////////////////////////////////
    // Begin image reading
    ///////////////////////////////////////////////////////////

    TIFF* tif = TIFFOpen(input_file, "r");
    if(!tif){
        fprintf(stderr,"Could not open the TIFF file.\n");
        return 1;
    }
    uint32_t width,height;
    float original_dpi_x, original_dpi_y;
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(tif, TIFFTAG_XRESOLUTION, &original_dpi_x);
    TIFFGetField(tif, TIFFTAG_YRESOLUTION, &original_dpi_y);

	uint32_t* raster = (uint32_t*) _TIFFmalloc(width * height * sizeof(uint32_t));
    if(raster == NULL) { fprintf(stderr,"Raster allocation error.\n"); return 1; }
	uint8_t* pixels = (uint8_t*) malloc(width * height * sizeof(uint8_t));
    if (!pixels) {
        fprintf(stderr, "Memory allocation error.\n");
        return 1;
    }
	if (!TIFFReadRGBAImage(tif, width, height, raster, 0)) {
        fprintf(stderr, "Failed to read TIFF image.\n");
        _TIFFfree(raster);
        TIFFClose(tif);
        return 1;
    }
	for (uint32_t row = 0; row < height; row++) {
		for (uint32_t col = 0; col < width; col++) {
			uint32_t pixel = raster[row * width + col];
			uint8_t r = TIFFGetR(pixel);
			uint8_t g = TIFFGetG(pixel);
			uint8_t b = TIFFGetB(pixel);
			pixels[row * width + col] = (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
		}
	}

    _TIFFfree(raster);
    TIFFClose(tif);

    ///////////////////////////////////////////////////////////
    // End image reading
    ///////////////////////////////////////////////////////////

    // Calcular fator de redimensionamento
    float x_scale = new_dpi / original_dpi_x;
    float y_scale = new_dpi / original_dpi_y;
    uint32_t new_width = (uint32_t)(width * x_scale);
    uint32_t new_height = (uint32_t)(height * y_scale);
    
    ///////////////////////////////////////////////////////////
    //  BEGIN BILINEAR INTERPOLATION
    ///////////////////////////////////////////////////////////
    //bilinear_interpolation(image, width, height, new_width, new_height, resized_image);

    uint8_t* newPixels = (uint8_t*) malloc(new_width * new_height * sizeof(uint8_t));
    if (!newPixels) {
        fprintf(stderr, "Falha ao alocar memória para a nova imagem.\n");
        free(pixels);
        return 1;
    }

    for(uint32_t j = 0; j < new_height; j++){
        for(uint32_t i = 0; i < new_width; i++){
            float src_x = i / x_scale;
            float src_y = j / y_scale;
            newPixels[j * new_width + i] = bilinear_interpolate(pixels, width, height, src_x, src_y);
        }
    }
    free(pixels);

    ///////////////////////////////////////////////////////////
    //  END BILINEAR INTERPOLATION
    ///////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////
    //  BEGIN WRITING NEW IMAGE
    ///////////////////////////////////////////////////////////

    TIFF *output = TIFFOpen(output_file, "w");
    if (!output) {
        fprintf(stderr, "Could not open output TIFF file.\n");
        return 1;
    }

    // Set TIFF tags
    TIFFSetField(output, TIFFTAG_IMAGEWIDTH, new_width);
    TIFFSetField(output, TIFFTAG_IMAGELENGTH, new_height);
    TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(output, TIFFTAG_ORIENTATION, ORIENTATION_BOTLEFT);
    TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    // Set new DPI
    TIFFSetField(output, TIFFTAG_XRESOLUTION, new_dpi);
    TIFFSetField(output, TIFFTAG_YRESOLUTION, new_dpi);
    TIFFSetField(output, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

    // Write the resized image
    for (uint32_t j = 0; j < new_height; j++) {
        TIFFWriteScanline(output, &newPixels[j * new_width], j, 0);
    }

    ///////////////////////////////////////////////////////////
    //  END WRITING NEW IMAGE
    ///////////////////////////////////////////////////////////

    TIFFClose(output);
    free(newPixels);

    printf("Output written to %s with %.2f DPI.\n", output_file, new_dpi);

    return 0;
}