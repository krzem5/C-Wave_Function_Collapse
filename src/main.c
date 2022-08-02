#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wfc.h>



#define IMAGE(w,h,...) \
	wfc_color_t __img_##__LINE__[]={__VA_ARGS__}; \
	wfc_image_t input_image={w,h,__img_##__LINE__}



#define INPUT_WIDTH 4
#define INPUT_HEIGHT 4
#define BOX_SIZE 2
#define OUTPUT_WIDTH 960
#define OUTPUT_HEIGHT 540



int main(int argc,const char** argv){
#ifdef _MSC_VER
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleMode(GetStdHandle(-11),7);
#endif
	srand(time(NULL));
	// IMAGE(
	// 	4,4,
	// 	0xffffffff,0xffffffff,0xffffffff,0xffffffff,
	// 	0xffffffff,0x5a5a5aff,0x5a5a5aff,0x5a5a5aff,
	// 	0xffffffff,0x5a5a5aff,0xe55a5aff,0x5a5a5aff,
	// 	0xffffffff,0x5a5a5aff,0x5a5a5aff,0x5a5a5aff,
	// );
	IMAGE(
		8,7,
		0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,
		0x252525ff,0xe55a5aff,0xe55a5aff,0xe55a5aff,0x252525ff,0x5a5ae5ff,0x5a5ae5ff,0x252525ff,
		0x252525ff,0xe55a5aff,0xe55a5aff,0xe55a5aff,0x252525ff,0x5a5ae5ff,0x5a5ae5ff,0x252525ff,
		0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x5a5ae5ff,0x5a5ae5ff,0x252525ff,
		0x252525ff,0x5ae55aff,0x5ae55aff,0x5ae55aff,0x252525ff,0x5a5ae5ff,0x5a5ae5ff,0x252525ff,
		0x252525ff,0x5ae55aff,0x5ae55aff,0x5ae55aff,0x252525ff,0x5a5ae5ff,0x5a5ae5ff,0x252525ff,
		0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,
	);
	wfc_color_t output_image_data[OUTPUT_WIDTH*OUTPUT_HEIGHT];
	wfc_image_t output_image={
		OUTPUT_WIDTH,
		OUTPUT_HEIGHT,
		output_image_data
	};
	wfc_print_image(&input_image);
	wfc_table_t table;
	wfc_build_table(&input_image,BOX_SIZE,WFC_FLAG_WRAP_X|WFC_FLAG_WRAP_Y|WFC_FLAG_WRAP_OUTPUT_X|WFC_FLAG_WRAP_OUTPUT_Y|WFC_FLAG_ROTATE|WFC_FLAG_FLIP,&table);
	wfc_print_table(&table);
	wfc_state_t state;
	wfc_init_state(&table,&output_image,&state);
	fflush(stdout);
	wfc_solve(&table,&state);
	wfc_generate_image(&table,&state,&output_image);
	wfc_free_state(&state);
	wfc_free_table(&table);
	putchar('\n');
	wfc_save_image(&output_image,"build/export.bmp");
	return 0;
}
