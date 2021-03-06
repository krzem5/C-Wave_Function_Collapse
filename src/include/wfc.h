#ifndef __WFC_H__
#define __WFC_H__ 1
#include <stdint.h>



#define WFC_FLAG_WRAP_X 1
#define WFC_FLAG_WRAP_OUTPUT_X 2
#define WFC_FLAG_WRAP_Y 4
#define WFC_FLAG_WRAP_OUTPUT_Y 8
#define WFC_FLAG_ROTATE 16
#define WFC_FLAG_FLIP 32



typedef uint32_t wfc_box_size_t;



typedef uint32_t wfc_color_t;



typedef uint8_t wfc_flags_t;



typedef uint32_t wfc_queue_size_t;



typedef uint64_t wfc_tile_hash_t;



typedef uint16_t wfc_tile_index_t;



typedef uint32_t wfc_size_t;



typedef uint32_t wfc_weight_t;



typedef struct _WFC_IMAGE{
	wfc_size_t width;
	wfc_size_t height;
	wfc_color_t* data;
} wfc_image_t;



typedef struct _WFC_TILE{
	wfc_tile_hash_t hash;
	wfc_color_t* data;
	uint64_t* connections;
} wfc_tile_t;



typedef struct _WFC_TABLE{
	wfc_tile_index_t tile_count;
	wfc_tile_t* tiles;
	wfc_box_size_t box_size;
	wfc_flags_t flags;
	wfc_tile_index_t data_elem_size;
} wfc_table_t;



typedef struct _WFC_PRNG{
	uint32_t data[64];
	uint32_t count;
} wfc_prng_t;



typedef struct _WFC_QUEUE{
	wfc_size_t* data;
	wfc_queue_size_t length;
} wfc_queue_t;



typedef struct _WFC_STATE{
	wfc_prng_t prng;
	uint64_t* data;
	uint64_t* bitmap;
	wfc_size_t bitmap_size;
	wfc_size_t length;
	wfc_queue_t* queues;
	wfc_queue_size_t queue_size;
	wfc_weight_t* weights;
	wfc_tile_index_t tile_count;
	wfc_tile_index_t data_elem_size;
	wfc_size_t pixel_count;
	wfc_size_t width;
} wfc_state_t;



void wfc_build_table(const wfc_image_t* image,wfc_box_size_t box_size,wfc_flags_t flags,wfc_table_t* out);



void wfc_free_state(wfc_state_t* state);



void wfc_free_table(wfc_table_t* table);



void wfc_generate_image(const wfc_table_t* table,const wfc_state_t* state,wfc_image_t* out);



void wfc_init_state(const wfc_table_t* table,const wfc_image_t* image,wfc_state_t* out);



void wfc_print_image(const wfc_image_t* image);



void wfc_print_table(const wfc_table_t* table);



void wfc_solve(const wfc_table_t* table,wfc_state_t* state);



#endif
