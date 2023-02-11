#ifndef _IMAGES_H_
#define _IMAGES_H_ 1
#include <wfc.h>



typedef struct _IMAGE_CONFIG{
	const char* name;
	wfc_size_t width;
	wfc_size_t height
	const wfc_pixel_t* data;
	wfc_box_size_t box_size;
	wfc_flags_t flags;
	wfc_palette_size_t palette_max_size;
	wfc_color_diffrence_t max_color_diff;
} image_config_t;



#endif
