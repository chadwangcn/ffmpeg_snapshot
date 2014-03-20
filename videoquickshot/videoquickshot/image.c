#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif



#include <jpeglib.h>
#include "image.h"
#include <setjmp.h>


#define return_if(arg, retval) do {if (arg) return retval; } while (0);

static int 
save_image_jpeg(const char* filename, uint8_t* rgb_buffer, int width, int height)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE* outfile;
    JSAMPROW row_pointer[1];
	JDIMENSION jpg_ret;
    int row_stride = width * 3;	

#ifdef WIN32
    outfile = _wfopen ( (wchar_t*)filename, L"wb");
#else
	outfile = fopen ( filename, "wb");
#endif
    return_if(NULL == outfile, -1); 
    cinfo.err = jpeg_std_error(&jerr);    
    jpeg_create_compress(&cinfo);
	
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
	
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 75, TRUE);    
    jpeg_stdio_dest(&cinfo, outfile);   
    jpeg_start_compress(&cinfo, TRUE);

    while (cinfo.next_scanline < cinfo.image_height)
    {		
        row_pointer[0] =(JSAMPROW)&rgb_buffer[cinfo.next_scanline * row_stride];		
        jpg_ret = jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
	fclose(outfile);

    return 0;
}

static int
save_image_ppm(const char* filename, uint8_t* rgb_buffer, int width, int height)
{
    FILE* file;
    int line;

#ifdef WIN32
    file = _wfopen ( (wchar_t*)filename, L"wb");
#else
	file = fopen ( filename, "wb");
#endif    
    
    return_if(NULL == file, -1);

    
    fprintf(file, "P6\n%d %d\n255\n", width, height);

  
    for (line = 0; line < height; ++line)
    {
        fwrite(rgb_buffer + line * width * 3, 1, width * 3, file);
    }

    fclose(file);
    return 0;
}

int
image_save(const char* filename, uint8_t* rgb_buffer, int width, int height, enum ImageFormat format)
{
    return_if(NULL == rgb_buffer, -1);
    return_if(NULL == filename, -1);
    switch (format)
    {
        case IMAGE_FORMAT_PPM:
            return save_image_ppm(filename, rgb_buffer, width, height);

        case IMAGE_FORMAT_JPEG:
            return save_image_jpeg(filename, rgb_buffer, width, height);

        default:
            fprintf(stderr, "Unsupported image format %d\n", format);
            return -1;
    }

    return 0;
}

char *
image_get_suffix(enum ImageFormat image_format)
{
    switch (image_format)
    {
        case IMAGE_FORMAT_PPM:
            return "ppm";

        case IMAGE_FORMAT_JPEG:
            return "jpg";

        default:
            return "bin";
    }
}

enum ImageFormat
image_get_format(const char* format)
{
    if (!strcmp(format, "ppm"))
    {
        return IMAGE_FORMAT_PPM;
    }

    else if (!strcmp(format, "jpg"))
    {
        return IMAGE_FORMAT_JPEG;
    }


    return -1;
}

char *
image_get_supported_string()
{
    return "ppm|jpg";
}


#ifdef __cplusplus
}
#endif
