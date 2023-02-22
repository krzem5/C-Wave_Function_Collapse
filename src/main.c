#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#endif
#include <images.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wfc.h>



#define PICK_PARAMETERS 1
#define GENERATE_IMAGE 1
#define IMAGE_NAME "hand_drawn_circle"

#define PROGRESS_FRAME_INTERVAL 0.05f



#if GENERATE_IMAGE
static unsigned long int get_time(void){
#ifdef _MSC_VER
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	return ((((unsigned long int)ft.dwHighDateTime)<<32)|ft.dwLowDateTime)*100-11644473600000000000;
#else
	struct timespec tm;
	clock_gettime(CLOCK_REALTIME,&tm);
	return tm.tv_sec*1000000000+tm.tv_nsec;
#endif
}



static void _progress_callback(const wfc_table_t* table,const wfc_state_t* state,void* ctx){
	static unsigned long int _last_time=0;
	unsigned long int current_time=get_time();
	if (current_time-_last_time<(unsigned long int)(PROGRESS_FRAME_INTERVAL*1e9)){
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
#endif



int main(int argc,const char** argv){
#ifdef _MSC_VER
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleMode(GetStdHandle(-11),7);
#endif
	const image_config_t* image_config=images;
	while (image_config->name&&strcmp(image_config->name,IMAGE_NAME)){
		image_config++;
	}
	if (!image_config->name){
		printf("Image '"IMAGE_NAME"' not found\n");
		return 1;
	}
#if PICK_PARAMETERS||GENERATE_IMAGE
	wfc_config_t config=image_config->config;
#endif
#if PICK_PARAMETERS
	wfc_pick_parameters(&(image_config->image),&config);
#endif
#if GENERATE_IMAGE
#ifdef _MSC_VER
	unsigned int output_width=90;
	unsigned int output_height=32;
#else
	struct winsize window_size;
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&window_size);
	unsigned int output_width=window_size.ws_col>>1;
	unsigned int output_height=window_size.ws_row-5;
#endif
	srand(time(NULL));
	wfc_color_t output_image_data[output_width*output_height];
	wfc_image_t output_image={
		output_width,
		output_height,
		output_image_data
	};
	wfc_print_image(&(image_config->image));
	unsigned long int time_start=get_time();
	wfc_table_t table;
	wfc_build_table(&(image_config->image),&config,&table);
	unsigned long int table_creation_time=get_time()-time_start;
	wfc_print_table(&table,&config);
	output_image.width/=table.downscale_factor;
	output_image.height/=table.downscale_factor;
	wfc_state_t state;
	wfc_init_state(&table,&output_image,&state);
	fflush(stdout);
	time_start=get_time();
	float cache=wfc_solve(&table,&state,&config,_progress_callback,&output_image);
	printf("\x1b[0m\x1b[?25h\x1b[%uA",output_image.height);
	unsigned long int generation_time=get_time()-time_start;
	wfc_generate_image(&table,&state,&output_image);
	putchar('\n');
	wfc_print_image(&output_image);
	output_image.width*=table.downscale_factor;
	output_image.height*=table.downscale_factor;
	wfc_generate_full_scale_image(&table,&state,&output_image);
	putchar('\n');
	wfc_print_image(&output_image);
	printf("Table size: %u (%lu kB)\nTable creation time: %.3lf\nGeneration time: %.3lf\nCache hits: %.3f%%\n",table.tile_count,(table.tile_count*(config.box_size*config.box_size+table.downscale_factor*table.downscale_factor)*sizeof(wfc_color_t)+512)>>10,table_creation_time*1e-9,generation_time*1e-9,cache*100);
	wfc_free_table(&table);
	wfc_free_state(&table,&state);
	wfc_save_image(&output_image,"build/export.bmp");
#endif
	return 0;
}
