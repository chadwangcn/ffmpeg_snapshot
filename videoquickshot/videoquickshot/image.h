#ifndef __IMAGE_H
#define __IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


enum ImageFormat
{
    
    IMAGE_FORMAT_PPM = 0,
    IMAGE_FORMAT_JPEG,
    IMAGE_FORMAT_COUNT
};

int read_JPEG_file (char * filename);

int
image_save(const char* filename, uint8_t* rgb_buffer, int width, int height, enum ImageFormat format);


char *
image_get_suffix(enum ImageFormat image_format);


enum ImageFormat
image_get_format(const char* format);


char *
image_get_supported_string();


#ifdef __cplusplus
}
#endif


#endif 