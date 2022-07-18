#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wfc.h>


#define INPUT_WIDTH 4
#define INPUT_HEIGHT 4
#define BOX_SIZE 2
#define OUTPUT_WIDTH 40
#define OUTPUT_HEIGHT 20



int main(int argc,const char** argv){
#ifdef _MSC_VER
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleMode(GetStdHandle(-11),7);
#endif
	srand(time(NULL));
	wfc_color_t input_image_data[INPUT_WIDTH*INPUT_HEIGHT]={
		0xffffffff,0xffffffff,0xffffffff,0xffffffff,
		0xffffffff,0x5a5a5aff,0x5a5a5aff,0x5a5a5aff,
		0xffffffff,0x5a5a5aff,0xe55a5aff,0x5a5a5aff,
		0xffffffff,0x5a5a5aff,0x5a5a5aff,0x5a5a5aff,
	};
	wfc_color_t output_image_data[OUTPUT_WIDTH*OUTPUT_HEIGHT];
	wfc_image_t input_image={
		INPUT_WIDTH,
		INPUT_HEIGHT,
		input_image_data
	};
	wfc_image_t output_image={
		OUTPUT_WIDTH,
		OUTPUT_HEIGHT,
		output_image_data
	};
	wfc_table_t table;
	wfc_build_table(&input_image,BOX_SIZE,WFC_FLAG_WRAP_X|WFC_FLAG_WRAP_Y|WFC_FLAG_WRAP_OUTPUT_X|WFC_FLAG_WRAP_OUTPUT_Y|WFC_FLAG_ROTATE|WFC_FLAG_FLIP,&table);
	wfc_print_table(&table);
	wfc_state_t state;
	wfc_init_state(&table,&output_image,&state);
	wfc_solve(&table,&state);
	wfc_generate_image(&table,&state,&output_image);
	wfc_free_state(&state);
	wfc_free_table(&table);
	putchar('\n');
	wfc_print_image(&output_image);
	return 0;
}
