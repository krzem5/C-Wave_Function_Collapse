#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif
#include <preloaded_images.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wfc.h>



#define STRATEGY WFC_STATE_DATA_ACCESS_STRATEGY_RAW

#define PICK_PARAMETERS 0
#define GENERATE_IMAGE 1
#define GENERATION_LOOP 0
#define CUT_OUT_SHAPES 0
#define CUT_OUT_SHAPE_BACKGROUND 0x000000ff
#define IMAGE_NAME "duck"

#define PROGRESS_FRAME_INTERVAL 0.05



static const char* _flag_strings[9]={"Flip","Rotate","Wrap X","Wrap Y","Wrap output X","Wrap output Y","Blend (corner)","Blend (pixel)","Average scaling"};



static char _format_int_data_buffer[4096];
static unsigned int _format_int_data_buffer_offset=0;



static void _get_terminal_size(wfc_size_t* width,wfc_size_t* height){
#ifdef _MSC_VER
	CONSOLE_SCREEN_BUFFER_INFO cbsi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&csbi);
	*width=csbi.srWindow.Right-csbi.srWindow.Left+1;
	*height=csbi.srWindow.Bottom-csbi.srWindow.Top+1;
#else
	struct winsize window_size;
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&window_size);
	*width=window_size.ws_col;
	*height=window_size.ws_row;
#endif
}



static unsigned long long int _current_time(void){
#ifdef _MSC_VER
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	return ((((unsigned long long int)ft.dwHighDateTime)<<32)|ft.dwLowDateTime)*100-11644473600000000000;
#else
	struct timespec tm;
	clock_gettime(CLOCK_REALTIME,&tm);
	return tm.tv_sec*1000000000+tm.tv_nsec;
#endif
}



static void _progress_callback(const wfc_table_t* table,const wfc_state_t* state,void* ctx){
	static unsigned long long int _last_time=0;
	unsigned long long int current_time=_current_time();
	if (current_time-_last_time<(unsigned long long int)(PROGRESS_FRAME_INTERVAL*1e9)){
		return;
	}
	_last_time=current_time;
	wfc_image_t* image=ctx;
	wfc_generate_image(table,state,image);
	static _Bool first=1;
	if (!first){
		printf("\x1b[%uA",image->height);
	}
	first=0;
	wfc_print_image(image);
}



static const char* _format_int(unsigned long int x){
	char* out=_format_int_data_buffer+_format_int_data_buffer_offset;
	_format_int_data_buffer_offset+=27;
	if (_format_int_data_buffer_offset>=4096){
		return "<out of memory>";
	}
	unsigned int i=26;
	do{
		i--;
		if (0x444444&(1<<i)){
			out[i]=',';
			i--;
		}
		out[i]=(x%10)+48;
		x/=10;
	} while (x);
	out[26]=0;
	return out+i;
}



int main(int argc,const char** argv){
#ifdef _MSC_VER
	SetConsoleOutputCP(CP_UTF8);
	HANDLE stdout_handle=GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD old_stdout_mode=GetConsoleMode(stdout_handle);
	SetConsoleMode(stdout_handle,ENABLE_PROCESSED_OUTPUT|ENABLE_WRAP_AT_EOL_OUTPUT|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
	const image_config_t* image_config=preloaded_images;
	while (image_config->name&&strcmp(image_config->name,IMAGE_NAME)){
		image_config++;
	}
	if (!image_config->name){
		printf("Image '"IMAGE_NAME"' not found\n");
		return 1;
	}
	wfc_config_t config=image_config->config;
	if (PICK_PARAMETERS){
#ifdef _MSC_VER
		HANDLE stdin_handle=GetStdHandle(STD_INPUT_HANDLE);
		DWORD old_stdin_mode=GetConsoleMode(stdin_handle);
		SetConsoleMode(stdin_handle,ENABLE_VIRTUAL_TERMINAL_INPUT);
#else
		struct termios old_terminal_config;
		tcgetattr(STDOUT_FILENO,&old_terminal_config);
		struct termios terminal_config=old_terminal_config;
		terminal_config.c_iflag=(terminal_config.c_iflag&(~(INLCR|IGNBRK)))|ICRNL;
		terminal_config.c_lflag=(terminal_config.c_lflag&(~(ICANON|ECHO)))|ISIG|IEXTEN;
		tcsetattr(STDIN_FILENO,TCSANOW,&terminal_config);
#endif
		wfc_pick_parameters(&(image_config->image),&config,_get_terminal_size);
#ifdef _MSC_VER
		SetConsoleMode(stdin_handle,old_stdin_mode);
#else
		tcsetattr(STDOUT_FILENO,TCSANOW,&old_terminal_config);
#endif
	}
	if (GENERATE_IMAGE){
		unsigned int output_width;
		unsigned int output_height;
		_get_terminal_size(&output_width,&output_height);
		output_width>>=1;
		wfc_color_t* output_image_data=malloc(output_width*output_height*sizeof(wfc_color_t));
		wfc_image_t output_image={
			output_width,
			output_height,
			output_image_data
		};
		wfc_print_image(&(image_config->image));
		unsigned long int time_start=_current_time();
		wfc_table_t table;
		wfc_build_table(&(image_config->image),&config,&table);
		unsigned long int table_creation_time=_current_time()-time_start;
		wfc_print_table(&table,&config,1);
_regenerate_image:
		srand(_current_time()&0xffffffff);
		unsigned char seed[256];
		for (unsigned int i=0;i<256;i++){
			seed[i]=rand()&255;
		}
		output_image.width/=table.downscale_factor;
		output_image.height/=table.downscale_factor;
		wfc_state_t state;
		time_start=_current_time();
		wfc_init_state(&table,&output_image,seed,STRATEGY,&state);
		unsigned long int state_creation_time=_current_time()-time_start;
		time_start=_current_time();
		wfc_stats_t stats;
		fflush(stdout);
		wfc_solve(&table,&state,&config,_progress_callback,&output_image,&stats);
		unsigned long int generation_time=_current_time()-time_start;
		if (CUT_OUT_SHAPES){
			printf("\x1b[0m\x1b[?25h\x1b[%uA\x1b[0J",output_image.height);
			output_image.width*=table.downscale_factor;
			output_image.height*=table.downscale_factor;
			wfc_generate_full_scale_image(&table,&state,&output_image);
			wfc_shapes_t shapes;
			wfc_extract_shapes(&output_image,CUT_OUT_SHAPE_BACKGROUND,&shapes);
			for (wfc_shape_count_t i=0;i<shapes.length;i++){
				putchar('\n');
				wfc_print_image(&((shapes.data+i)->image));
				if (!GENERATION_LOOP){
					char path[256];
					snprintf(path,256,"build/export-%.4u.bmp",i);
					wfc_save_image(&((shapes.data+i)->image),path);
				}
			}
			wfc_free_shapes(&shapes);
		}
		else{
			printf("\x1b[0m\x1b[?25h\x1b[%uA\n",output_image.height);
			wfc_generate_image(&table,&state,&output_image);
			wfc_print_image(&output_image);
			output_image.width*=table.downscale_factor;
			output_image.height*=table.downscale_factor;
			wfc_generate_full_scale_image(&table,&state,&output_image);
			putchar('\n');
			wfc_print_image(&output_image);
		}
		if (GENERATION_LOOP){
			wfc_free_state(&table,&state);
			while (getchar()!='\n');
			printf("\x1b[0H\x1b[0J");
			goto _regenerate_image;
		}
		const unsigned long long int* seed64=(unsigned long long int*)seed;
		printf("Seed:\n  %.16llx%.16llx%.16llx%.16llx\n  %.16llx%.16llx%.16llx%.16llx\n  %.16llx%.16llx%.16llx%.16llx\n  %.16llx%.16llx%.16llx%.16llx\nConfig:\n  Box size: %s\n  Flags:",
			seed64[0],
			seed64[1],
			seed64[2],
			seed64[3],
			seed64[4],
			seed64[5],
			seed64[6],
			seed64[7],
			seed64[8],
			seed64[9],
			seed64[10],
			seed64[11],
			seed64[12],
			seed64[13],
			seed64[14],
			seed64[15],
			_format_int(config.box_size)
		);
		_Bool first=1;
		for (unsigned int i=0;config.flags>>i;i++){
			if (!((config.flags>>i)&1)){
				continue;
			}
			if (!first){
				putchar(',');
			}
			first=0;
			printf(" %s",_flag_strings[i]);
		}
		printf("\n  Palette size: %s\n  Similarity score: %s\n  Downscale factor: %s\n  Propagation distance: %s\n  Delete size: %s\n  Max delete count: %s\n  Fast mask counter initial value: %s\n  Fast mask counter cache initial value: %s\n  Fast mask counter maximal value: %s\nTable:\n  Tile count: %s\n  Tile element size: %s B\n  Tile data: %s kB\n  Neighbours: %s kB\n  Upscaled data: %s kB\nTable creation time: %.3lfs\nState:\n  Bitmap: %s kB\n  Pixel tile data: %s kB\n  Queues: %s kB\n  Weights: %s kB\n  Stacks: %s kB\n  Cache: %s kB\n  Precalculated masks: %s kB\nState creation time: %.3lfs\nSimulation:\n  Updates:\n    Collapse: %s\n    Propagation: %s\n  Removals:\n    Pixels: %s\n    Restarts: %s\n  Data access:\n",
			_format_int(config.palette_max_size),
			_format_int(config.max_color_diff),
			_format_int(config.downscale_factor),
			_format_int(config.propagation_distance),
			_format_int(config.delete_size),
			_format_int(config.max_delete_count),
			_format_int(config.fast_mask_counter_init),
			_format_int(config.fast_mask_cache_counter_init),
			_format_int(config.fast_mask_counter_max),
			_format_int(table.tile_count),
			_format_int(table.data_elem_size*sizeof(uint64_t)),
			_format_int((table.tile_count*config.box_size*config.box_size*sizeof(wfc_color_t)+1023)>>10),
			_format_int((8*table.tile_count*table.data_elem_size*sizeof(uint64_t)+1023)>>10),
			_format_int((table.tile_count*table.downscale_factor*table.downscale_factor*sizeof(wfc_color_t)+1023)>>10),
			table_creation_time*1e-9,
			_format_int((state.bitmap_size+31)>>5),
			_format_int((state.length*sizeof(uint64_t)+1023)>>10),
			_format_int((table.tile_count*(sizeof(wfc_queue_t)+state.queue_size*32)+1023)>>10),
			_format_int((table.tile_count*sizeof(wfc_weight_t)+1023)>>10),
			_format_int((2*state.pixel_count*sizeof(wfc_size_t)+1023)>>10),
			(state.data_access_strategy==WFC_STATE_DATA_ACCESS_STRATEGY_FAST_MASK?_format_int((262208*sizeof(wfc_fast_mask_t)+1023)>>10):"0"),
			(state.data_access_strategy==WFC_STATE_DATA_ACCESS_STRATEGY_PRECALCULATED_MASK?_format_int((((table.data_elem_size-1)*(table.data_elem_size-1)*8192ul+(table.data_elem_size-1)*1024+3*256+256)*4*sizeof(uint64_t)+1023)>>10):"0"),
			state_creation_time*1e-9,
			_format_int(stats.step_count),
			_format_int(stats.propagation_step_count),
			_format_int(stats.delete_count),
			_format_int(stats.restart_count)
		);
		if (state.data_access_strategy==WFC_STATE_DATA_ACCESS_STRATEGY_FAST_MASK){
			printf("    Precalculated masks: 0\n    Fast cache: %.3f%% (%s)\n    Cache: %.3f%% (%s)\n    Raw: %.3f%% (%s)\n    Total: %s\n",
				((float)stats.cache_hit_fast_count)/stats.access_count*100,
				_format_int(stats.cache_hit_fast_count),
				((float)stats.cache_hit_count)/stats.access_count*100,
				_format_int(stats.cache_hit_count),
				((float)(stats.access_count-stats.cache_hit_count-stats.cache_hit_fast_count))/stats.access_count*100,
				_format_int(stats.access_count-stats.cache_hit_count-stats.cache_hit_fast_count),
				_format_int(stats.access_count)
			);
		}
		else if (state.data_access_strategy==WFC_STATE_DATA_ACCESS_STRATEGY_PRECALCULATED_MASK){
			printf("    Precalculated masks: %s\n    Fast cache: 0\n    Cache: 0\n    Raw: 0\n    Total: %s\n",
				_format_int(stats.access_count),
				_format_int(stats.access_count)
			);
		}
		else{
			printf("    Precalculated masks: 0\n    Fast cache: 0\n    Cache: 0\n    Raw: %s\n    Total: %s\n",
				_format_int(stats.access_count),
				_format_int(stats.access_count)
			);
		}
		printf("Simulation time: %.3lfs\n",
			generation_time*1e-9
		);
		wfc_free_state(&table,&state);
		wfc_free_table(&table);
		wfc_save_image(&output_image,"build/export.bmp");
		free(output_image_data);
	}
#ifdef _MSC_VER
	SetConsoleMode(stdout_handle,old_stdout_mode);
#endif
	return 0;
}
