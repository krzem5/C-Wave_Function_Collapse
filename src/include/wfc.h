#ifndef __WFC_H__
#define __WFC_H__ 1
#include <stdint.h>



#define WFC_FLAG_EXTEND_X 0
#define WFC_FLAG_WRAP_X 1

#define WFC_FLAG_EXTEND_Y 0
#define WFC_FLAG_WRAP_Y 2



typedef uint32_t wfc_box_size_t;



typedef uint32_t wfc_color_t;



typedef uint8_t wfc_flags_t;



typedef uint64_t wfc_tile_hash_t;



typedef uint16_t wfc_tile_index_t;



typedef uint32_t wfc_size_t;



typedef struct _WFC_IMAGE{
	wfc_size_t width;
	wfc_size_t height;
	const wfc_color_t* data;
} wfc_image_t;



typedef struct _WFC_TILE{
	wfc_size_t x;
	wfc_size_t y;
	wfc_tile_hash_t hash;
} wfc_tile_t;



typedef struct _WFC_TABLE{
	wfc_tile_index_t tile_count;
	wfc_tile_t* tiles;
	wfc_box_size_t box_size;
	wfc_flags_t flags;
} wfc_table_t;



void wfc_build_table(const wfc_image_t* image,wfc_box_size_t box_size,wfc_flags_t flags,wfc_table_t* out);



void wfc_free_table(wfc_table_t* table);



void wfc_print_table(const wfc_table_t* table,const wfc_image_t* image);



#endif
