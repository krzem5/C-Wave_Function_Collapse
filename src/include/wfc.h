#ifndef _WFC_H_
#define _WFC_H_ 1
#include <stdint.h>



#define WFC_FLAG_FLIP 1
#define WFC_FLAG_ROTATE 2
#define WFC_FLAG_WRAP_X 4
#define WFC_FLAG_WRAP_Y 8
#define WFC_FLAG_WRAP_OUTPUT_X 16
#define WFC_FLAG_WRAP_OUTPUT_Y 32
#define WFC_FLAG_BLEND_CORNER 64
#define WFC_FLAG_BLEND_PIXEL 128
#define WFC_FLAG_AVERAGE_SCALING 256

#define WFC_TILE_ROTATED 0x20000000
#define WFC_TILE_FLIPPED 0x80000000
#define WFC_TILE_GET_ROTATION(t) (((t)->x>>29)&3)
#define WFC_TILE_GET_X(t) ((t)->x&0x1fffffff)



typedef uint32_t wfc_box_size_t;



typedef uint32_t wfc_color_t;



typedef uint32_t wfc_color_diffrence_t;



typedef uint16_t wfc_flags_t;



typedef uint32_t wfc_palette_color_index_t;



typedef uint32_t wfc_palette_size_t;



typedef uint32_t wfc_queue_size_t;



typedef uint64_t wfc_tile_hash_t;



typedef uint32_t wfc_tile_index_t;



typedef uint32_t wfc_size_t;



typedef uint32_t wfc_weight_t;



typedef uint32_t wfc_fast_mask_counter_t;



typedef uint32_t wfc_delete_count_t;



typedef struct _WFC_IMAGE{
	wfc_size_t width;
	wfc_size_t height;
	wfc_color_t* data;
} wfc_image_t;



typedef struct _WFC_TILE{
	wfc_tile_hash_t hash;
	wfc_color_t* data;
	uint64_t* connections;
	wfc_color_t* upscaled_data;
	wfc_size_t x;
	wfc_size_t y;
	wfc_size_t _upscaled_data_count;
} wfc_tile_t;



typedef struct _WFC_TABLE{
	wfc_tile_index_t tile_count;
	wfc_tile_t* tiles;
	wfc_tile_index_t data_elem_size;
	wfc_size_t downscale_factor;
	uint64_t* _connection_data;
} wfc_table_t;



typedef struct _WFC_PRNG{
	uint32_t data[64];
	uint32_t count;
} wfc_prng_t;



typedef struct _WFC_QUEUE{
	wfc_size_t* data;
	wfc_queue_size_t length;
} wfc_queue_t;



typedef struct _WFC_QUEUE_LOCATION{
	wfc_tile_index_t queue_index;
	wfc_delete_count_t delete_count;
	wfc_size_t index;
} wfc_queue_location_t;



typedef struct _WFC_FAST_MASK{
	uint64_t key;
	uint64_t data[4];
	uint32_t offset;
	wfc_fast_mask_counter_t counter;
} wfc_fast_mask_t;



typedef struct _WFC_STATE{
	wfc_prng_t prng;
	uint64_t* data;
	uint64_t* bitmap;
	wfc_size_t bitmap_size;
	wfc_size_t length;
	wfc_queue_t* queues;
	wfc_queue_size_t queue_size;
	wfc_weight_t* weights;
	wfc_queue_location_t* queue_indicies;
	wfc_size_t* update_stack;
	wfc_size_t* delete_stack;
	wfc_fast_mask_t* fast_mask;
	wfc_fast_mask_t* fast_mask_cache;
	wfc_size_t pixel_count;
	wfc_size_t width;
} wfc_state_t;



typedef struct _WFC_COLOR_RANGE{
	uint8_t min[4];
	uint8_t max[4];
	uint8_t diff[4];
} wfc_color_range_t;



typedef struct _WFC_PALETTE_RANGE{
	wfc_palette_color_index_t* indicies;
	wfc_palette_size_t size;
	wfc_color_range_t range;
} wfc_palette_range_t;



typedef struct _WFC_CONFIG{
	wfc_box_size_t box_size;
	wfc_flags_t flags;
	wfc_palette_size_t palette_max_size;
	wfc_color_diffrence_t max_color_diff;
	wfc_size_t downscale_factor;
	wfc_box_size_t propagation_distance;
	wfc_box_size_t delete_size;
	wfc_delete_count_t max_delete_count;
	wfc_fast_mask_counter_t fast_mask_counter_init;
	wfc_fast_mask_counter_t fast_mask_cache_counter_init;
	wfc_fast_mask_counter_t fast_mask_counter_max;
} wfc_config_t;



typedef struct _WFC_STATS{
	uint64_t total_cache_checks;
	uint64_t cache_hits;
	uint64_t fast_cache_hits;
	uint64_t deleted_tiles;
	uint64_t restarts;
	uint64_t steps;
	uint64_t propagation_steps;
} wfc_stats_t;



typedef void (*wfc_callback_t)(const wfc_table_t*,const wfc_state_t*,void*);



void wfc_pick_parameters(const wfc_image_t* image,wfc_config_t* config);



void wfc_build_table(const wfc_image_t* image,const wfc_config_t* config,wfc_table_t* out);



void wfc_free_state(const wfc_table_t* table,wfc_state_t* state);



void wfc_free_table(wfc_table_t* table);



void wfc_generate_image(const wfc_table_t* table,const wfc_state_t* state,wfc_image_t* out);



void wfc_generate_full_scale_image(const wfc_table_t* table,const wfc_state_t* state,wfc_image_t* out);



void wfc_init_state(const wfc_table_t* table,const wfc_image_t* image,wfc_state_t* out);



void wfc_print_image(const wfc_image_t* image);



void wfc_print_table(const wfc_table_t* table,const wfc_config_t* config,_Bool print_upscaled_data);



void wfc_save_image(const wfc_image_t* image,const char* path);



void wfc_solve(const wfc_table_t* table,wfc_state_t* state,const wfc_config_t* config,wfc_callback_t callback,void* ctx,wfc_stats_t* out);



#endif
