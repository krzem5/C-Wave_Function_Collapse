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



#define BOX_SIZE 2
#define OUTPUT_WIDTH 96
#define OUTPUT_HEIGHT 26
#define DRAW_PROGRESS_IMAGES 1



void _progress_callback(const wfc_table_t* table,const wfc_state_t* state,void* ctx){
#if DRAW_PROGRESS_IMAGES
	static unsigned char index=0;
	index++;
	if (index){
		return;
	}
	wfc_image_t* image=ctx;
	wfc_generate_image(table,state,image);
	printf("\x1b[0;0H");
	wfc_print_image(image);
#endif
}



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
	// IMAGE(
	// 	8,7,
	// 	0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,
	// 	0x252525ff,0xe55a5aff,0xe55a5aff,0xe55a5aff,0x252525ff,0x5a5ae5ff,0x5a5ae5ff,0x252525ff,
	// 	0x252525ff,0xe55a5aff,0xe55a5aff,0xe55a5aff,0x252525ff,0x5a5ae5ff,0x5a5ae5ff,0x252525ff,
	// 	0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x5a5ae5ff,0x5a5ae5ff,0x252525ff,
	// 	0x252525ff,0x5ae55aff,0x5ae55aff,0x5ae55aff,0x252525ff,0x5a5ae5ff,0x5a5ae5ff,0x252525ff,
	// 	0x252525ff,0x5ae55aff,0x5ae55aff,0x5ae55aff,0x252525ff,0x5a5ae5ff,0x5a5ae5ff,0x252525ff,
	// 	0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,
	// );
	// IMAGE(
	// 	12,12,
	// 	0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,
	// 	0x252525ff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0x252525ff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0x252525ff,
	// 	0x252525ff,0xffffffff,0xed1c24ff,0xed1c24ff,0xed1c24ff,0xffffffff,0x252525ff,0xffffffff,0x3f48ccff,0x3f48ccff,0xffffffff,0x252525ff,
	// 	0x252525ff,0xffffffff,0xed1c24ff,0xed1c24ff,0xed1c24ff,0xffffffff,0x252525ff,0xffffffff,0x3f48ccff,0x3f48ccff,0xffffffff,0x252525ff,
	// 	0x252525ff,0xffffffff,0xed1c24ff,0xed1c24ff,0xed1c24ff,0xffffffff,0x252525ff,0xffffffff,0x3f48ccff,0x3f48ccff,0xffffffff,0x252525ff,
	// 	0x252525ff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0x252525ff,0xffffffff,0x3f48ccff,0x3f48ccff,0xffffffff,0x252525ff,
	// 	0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0xffffffff,0x3f48ccff,0x3f48ccff,0xffffffff,0x252525ff,
	// 	0x252525ff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0x252525ff,0xffffffff,0x3f48ccff,0x3f48ccff,0xffffffff,0x252525ff,
	// 	0x252525ff,0xffffffff,0x22b14cff,0x22b14cff,0x22b14cff,0xffffffff,0x252525ff,0xffffffff,0x3f48ccff,0x3f48ccff,0xffffffff,0x252525ff,
	// 	0x252525ff,0xffffffff,0x22b14cff,0x22b14cff,0x22b14cff,0xffffffff,0x252525ff,0xffffffff,0x3f48ccff,0x3f48ccff,0xffffffff,0x252525ff,
	// 	0x252525ff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0x252525ff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0x252525ff,
	// 	0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,0x252525ff,
	// );
	// IMAGE(
	// 	15,24,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xfff200ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xfff200ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xfff200ff,0x00aa00ff,0xfff200ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xfff200ff,0x00aa00ff,0xfff200ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xfff200ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xfff200ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xfff200ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,
	// 	0xfff200ff,0x00aa00ff,0xfff200ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xfff200ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xfff200ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xfff200ff,0x00aa00ff,0xfff200ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xfff200ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0x00aa00ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,
	// 	0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0x00aa00ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,0xbfe8f2ff,
	// 	0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x00aa00ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,
	// 	0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,
	// );
	IMAGE(
		72,32,
		0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,
		0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,
		0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,
		0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,
		0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x7f7f7fff,0x7f7f7fff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,
		0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,
		0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,
		0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0xb5e61dff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,
		0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,
		0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0x99d9eaff,0xb97a57ff,0xb97a57ff,
		0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0x3f48ccff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,
		0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,
		0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,0xb97a57ff,
	);
	wfc_color_t output_image_data[OUTPUT_WIDTH*OUTPUT_HEIGHT];
	wfc_image_t output_image={
		OUTPUT_WIDTH,
		OUTPUT_HEIGHT,
		output_image_data
	};
	wfc_print_image(&input_image);
	wfc_table_t table;
	wfc_build_table(&input_image,BOX_SIZE,0,&table);
	wfc_print_table(&table);
	wfc_state_t state;
	wfc_init_state(&table,&output_image,&state);
	fflush(stdout);
	wfc_solve(&table,&state,_progress_callback,&output_image);
	wfc_generate_image(&table,&state,&output_image);
	wfc_free_state(&state);
	wfc_free_table(&table);
	putchar('\n');
	wfc_print_image(&output_image);
	wfc_save_image(&output_image,"build/export.bmp");
	return 0;
}
