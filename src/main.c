#include <stdio.h>
#include <wfc.h>


#define WIDTH 5
#define HEIGHT 5
#define BOX_SIZE 3



int main(int argc,const char** argv){
	const wfc_color_t image_data[WIDTH*HEIGHT]={
		0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
		0xffffffff,0x5a5a5aff,0x5a5a5aff,0x5a5a5aff,0xffffffff,
		0xffffffff,0x5a5a5aff,0xe55a5aff,0x5a5a5aff,0xffffffff,
		0xffffffff,0x5a5a5aff,0x5a5a5aff,0x5a5a5aff,0xffffffff,
		0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
	};
	wfc_image_t image={
		WIDTH,
		HEIGHT,
		image_data
	};
	wfc_table_t table;
	wfc_build_table(&image,BOX_SIZE,WFC_FLAG_EXTEND_X|WFC_FLAG_EXTEND_Y,&table);
	wfc_print_table(&table,&image);
	wfc_free_table(&table);
	return 0;
}
