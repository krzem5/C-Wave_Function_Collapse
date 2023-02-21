#ifdef _MSC_VER
#include <intrin.h>
#else
#include <immintrin.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wfc.h>



#ifdef _MSC_VER
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse)
#define FORCE_INLINE __inline __forceinline
static FORCE_INLINE unsigned long FIND_FIRST_SET_BIT(unsigned __int64 m){
	unsigned long o;
	_BitScanForward64(&o,m);
	return o;
}
static FORCE_INLINE unsigned long FIND_LAST_SET_BIT(unsigned long m){
	unsigned long o;
	_BitScanReverse(&o,m);
	return o;
}
#define POPULATION_COUNT(m) __popcnt64((m))
#else
#define FORCE_INLINE inline __attribute__((always_inline))
#define FIND_FIRST_SET_BIT(m) (__builtin_ffsll((m))-1)
#define FIND_LAST_SET_BIT(m) (31-__builtin_clz((m)))
#define POPULATION_COUNT(m) __builtin_popcountll((m))
#endif



#define FNV_OFFSET_BASIS 0xcbf29ce484222325
#define FNV_PRIME 0x100000001b3

#define WEIGHT_RANDOMNESS_SHIFT 3
#define QUEUE_INDEX_COLLAPSED 0xffff
#define MAX_ALLOWED_REMOVALS 1024
#define FAST_MASK_COUNTER_INIT 256
#define FAST_MASK_MAX_COUNTER 1024

#define DIVMOD_WIDTH(number,div,mod) \
	do{ \
		wfc_size_t __number=(number); \
		wfc_size_t __div=(__number*mult)>>32; \
		__div=(((__number-__div)>>1)+__div)>>shift; \
		div=__div; \
		mod=__number-__div*state->width; \
	} while (0)

#define INIT_RANGE(range) \
	do{ \
		wfc_color_range_t* __range=(range); \
		*((uint32_t*)(__range->min))=0xffffffff; \
		*((uint32_t*)(__range->max))=0; \
	} while (0)
#define UPDATE_RANGE(range,color) \
	do{ \
		wfc_color_range_t* __range=(range); \
		wfc_color_t __color=(color); \
		for (unsigned int __i=0;__i<4;__i++){ \
			uint8_t __value=__color>>(__i<<3); \
			if (__value>__range->max[__i]){ \
				__range->max[__i]=__value; \
			} \
			if (__value<__range->min[__i]){ \
				__range->min[__i]=__value; \
			} \
		} \
	} while (0)
#define FINISH_RANGE(range) \
	do{ \
		wfc_color_range_t* __range=(range); \
		*((uint32_t*)(__range->diff))=(*((uint32_t*)(__range->max)))-(*((uint32_t*)(__range->min))); \
	} while (0)

#define BMP_HEADER_SIZE 54
#define DIB_HEADER_SIZE 40
#define BI_RGB 0

#define COMMAND(a,b) ((((unsigned int)(a))<<8)|(b))
#define MAX_EDIT_INDEX 20
#define ADJUST_VALUE_AT_EDIT_INDEX(var,offset,width,new_digit) \
	unsigned int pow=powers_of_ten[edit_index-offset+8-width]; \
	unsigned int digit=(var/pow)%10; \
	var+=((new_digit)-digit)*pow;
#define ADJUST_FLAG_AT_INDEX(state,flag) \
	config->flags&=~flag; \
	if (state){ \
		config->flags|=flag; \
	}



static const unsigned int powers_of_ten[8]={10000000,1000000,100000,10000,1000,100,10,1};



static FORCE_INLINE unsigned int _color_diffrence(wfc_color_t x,wfc_color_t y){
	int r=(x>>24)-(y>>24);
	int g=((x>>16)&0xff)-((y>>16)&0xff);
	int b=((x>>8)&0xff)-((y>>8)&0xff);
	int a=(x&0xff)-(y&0xff);
	return (r<0?-r:r)+(g<0?-g:g)+(b<0?-b:b)+(a<0?-a:a);
}



static void _quicksort_palette(const wfc_color_t* palette,uint32_t* data,wfc_palette_size_t length,wfc_color_t mask){
	wfc_color_t last_color=palette[data[length]]&mask;
	wfc_palette_size_t i=0;
	for (wfc_palette_size_t j=0;j<length;j++){
		if ((palette[data[j]]&mask)<last_color){
			uint32_t tmp=*(data+i);
			*(data+i)=*(data+j);
			*(data+j)=tmp;
			i++;
		}
	}
	uint32_t tmp=*(data+i);
	*(data+i)=*(data+length);
	*(data+length)=tmp;
	if (i>1){
		_quicksort_palette(palette,data,i-1,mask);
	}
	i++;
	if (i<length){
		_quicksort_palette(palette,data+i,length-i,mask);
	}
}



static _Bool _add_tile(wfc_table_t* table,const wfc_config_t* config,wfc_color_t* data,wfc_color_t* upscaled_data){
	wfc_tile_hash_t hash=FNV_OFFSET_BASIS;
	for (wfc_box_size_t i=0;i<config->box_size*config->box_size;i++){
		hash=(hash^data[i])*FNV_PRIME;
	}
	wfc_tile_t* tile=table->tiles;
	for (wfc_tile_index_t i=0;i<table->tile_count;i++){
		if (tile->hash==hash&&!memcmp(tile->data,data,config->box_size*config->box_size*sizeof(wfc_color_t))){
			wfc_color_t* src_a=tile->upscaled_data;
			const wfc_color_t* src_b=upscaled_data;
			for (wfc_size_t j=0;j<table->downscale_factor*table->downscale_factor;j++){
				unsigned int r=(((*src_a)>>24)*tile->_upscaled_data_count+((*src_b)>>24))/(tile->_upscaled_data_count+1);
				unsigned int g=((((*src_a)>>16)&0xff)*tile->_upscaled_data_count+(((*src_b)>>16)&0xff))/(tile->_upscaled_data_count+1);
				unsigned int b=((((*src_a)>>8)&0xff)*tile->_upscaled_data_count+(((*src_b)>>8)&0xff))/(tile->_upscaled_data_count+1);
				unsigned int a=(((*src_a)&0xff)*tile->_upscaled_data_count+((*src_b)&0xff))/(tile->_upscaled_data_count+1);
				*src_a=(r<<24)|(g<<16)|(b<<8)|a;
				src_a++;
				src_b++;
			}
			tile->_upscaled_data_count++;
			return 0;
		}
		tile++;
	}
	table->tile_count++;
	table->tiles=realloc(table->tiles,table->tile_count*sizeof(wfc_tile_t));
	tile=table->tiles+table->tile_count-1;
	tile->hash=hash;
	tile->data=data;
	tile->upscaled_data=upscaled_data;
	tile->_upscaled_data_count=1;
	return 1;
}



static FORCE_INLINE uint32_t _get_random(wfc_state_t* state,uint32_t n){
	if (!state->prng.count){
		__m256i* ptr=(__m256i*)(state->prng.data);
		__m256i permute_a=_mm256_set_epi32(4,3,2,1,0,7,6,5);
		__m256i permute_b=_mm256_set_epi32(2,1,0,7,6,5,4,3);
		__m256i s0=_mm256_lddqu_si256(ptr+0);
		__m256i s1=_mm256_lddqu_si256(ptr+1);
		__m256i s2=_mm256_lddqu_si256(ptr+2);
		__m256i s3=_mm256_lddqu_si256(ptr+3);
		__m256i u0=_mm256_srli_epi64(s0,1);
		__m256i u1=_mm256_srli_epi64(s1,3);
		__m256i u2=_mm256_srli_epi64(s2,1);
		__m256i u3=_mm256_srli_epi64(s3,3);
		__m256i t0=_mm256_permutevar8x32_epi32(s0,permute_a);
		__m256i t1=_mm256_permutevar8x32_epi32(s1,permute_b);
		__m256i t2=_mm256_permutevar8x32_epi32(s2,permute_a);
		__m256i t3=_mm256_permutevar8x32_epi32(s3,permute_b);
		s0=_mm256_add_epi64(t0,u0);
		s1=_mm256_add_epi64(t1,u1);
		s2=_mm256_add_epi64(t2,u2);
		s3=_mm256_add_epi64(t3,u3);
		_mm256_storeu_si256(ptr+0,s0);
		_mm256_storeu_si256(ptr+1,s1);
		_mm256_storeu_si256(ptr+2,s2);
		_mm256_storeu_si256(ptr+3,s3);
		_mm256_storeu_si256(ptr+4,_mm256_xor_si256(u0,t1));
		_mm256_storeu_si256(ptr+5,_mm256_xor_si256(u2,t3));
		_mm256_storeu_si256(ptr+6,_mm256_xor_si256(s0,s3));
		_mm256_storeu_si256(ptr+7,_mm256_xor_si256(s2,s1));
		state->prng.count=64;
	}
	state->prng.count--;
	return state->prng.data[state->prng.count]%n;
}



static FORCE_INLINE wfc_tile_index_t _find_first_bit(const uint64_t* data){
	wfc_tile_index_t out=0;
	while (!(*data)){
		data++;
		out+=64;
	}
	return out+FIND_FIRST_SET_BIT(*data);
}



static void _print_integer(unsigned int value,unsigned int width,unsigned int edit_index){
	const unsigned int* powers=powers_of_ten+8-width;
	_Bool is_leading_zero=1;
	for (unsigned int i=0;i<width;i++){
		unsigned int digit=(value/(*powers))%10;
		if (digit||i==width-1){
			is_leading_zero=0;
		}
		if (i==edit_index){
			printf("\x1b[38;2;63;153;255m");
		}
		else if (is_leading_zero){
			printf("\x1b[38;2;140;83;119m");
		}
		else{
			printf("\x1b[38;2;240;143;219m");
		}
		putchar(digit+48);
		powers++;
	}
}



static void _print_flag(_Bool value,const char* name,unsigned int edit_index){
	if (!edit_index){
		printf("\x1b[38;2;63;153;255m");
	}
	else if (!value){
		printf("\x1b[38;2;140;83;119m");
	}
	else{
		printf("\x1b[38;2;240;143;219m");
	}
	while (*name){
		putchar(*name+(!value)*32);
		name++;
	}
}



void wfc_pick_parameters(const wfc_image_t* image,wfc_config_t* config){
#ifndef _MSC_VER
	unsigned int edit_index=3+(config->box_size<10);
	struct winsize window_size;
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&window_size);
	unsigned int width=window_size.ws_col;
	unsigned int height=window_size.ws_row;
	struct termios old_terminal_config;
	tcgetattr(STDOUT_FILENO,&old_terminal_config);
	struct termios terminal_config=old_terminal_config;
	terminal_config.c_iflag=(terminal_config.c_iflag&(~(INLCR|IGNBRK)))|ICRNL;
	terminal_config.c_lflag=(terminal_config.c_lflag&(~(ICANON|ECHO)))|ISIG|IEXTEN;
	tcsetattr(STDIN_FILENO,TCSANOW,&terminal_config);
	int scrolled_lines=0;
	unsigned int tile_columns=0;
	unsigned int tile_column_buffer=0;
	unsigned int tile_rows=0;
	unsigned int max_scroll_height=0;
	wfc_box_size_t table_box_size=config->box_size;
	char line_buffer[4096];
	unsigned int changes=2;
	char command[4];
	wfc_table_t table;
	printf("\x1b[?25l");
	for (unsigned int i=0;i<height;i++){
		putchar('\n');
	}
	while (1){
		_Bool update_grid=0;
		if (changes==2){
			if (!config->box_size){
				config->box_size=1;
			}
			changes=0;
			wfc_build_table(image,config,&table);
			table_box_size=config->box_size;
			update_grid=1;
		}
		ioctl(STDOUT_FILENO,TIOCGWINSZ,&window_size);
		if (window_size.ws_col!=width){
			width=window_size.ws_col;
			update_grid=1;
		}
		height=window_size.ws_row;
		if (update_grid){
			tile_columns=width/(2*config->box_size+2);
			tile_column_buffer=width-tile_columns*(2*config->box_size+2)+2;
			tile_rows=(table.tile_count+tile_columns-1)/tile_columns;
			max_scroll_height=(table_box_size+1)*(tile_rows-1);
		}
		printf("\x1b[H\x1b[48;2;66;67;63m\x1b[38;2;245;245;245mDs: ");
		_print_integer(config->downscale_factor,3,edit_index);
		printf("\x1b[38;2;245;245;245m, B: ");
		_print_integer(config->box_size,2,edit_index-3);
		printf("\x1b[38;2;245;245;245m, P: ");
		_print_integer(config->palette_max_size,4,edit_index-5);
		printf("\x1b[38;2;245;245;245m, Ss: ");
		_print_integer(config->max_color_diff,6,edit_index-9);
		printf("\x1b[38;2;245;245;245m, F: ");
		_print_flag(!!(config->flags&WFC_FLAG_FLIP),"F",edit_index-15);
		putchar(' ');
		_print_flag(!!(config->flags&WFC_FLAG_ROTATE),"R",edit_index-16);
		putchar(' ');
		_print_flag(!!(config->flags&WFC_FLAG_WRAP_X),"WX",edit_index-17);
		putchar(' ');
		_print_flag(!!(config->flags&WFC_FLAG_WRAP_Y),"WY",edit_index-18);
		putchar(' ');
		_print_flag(!!(config->flags&WFC_FLAG_WRAP_OUTPUT_X),"WOX",edit_index-19);
		putchar(' ');
		_print_flag(!!(config->flags&WFC_FLAG_WRAP_OUTPUT_Y),"ROY",edit_index-20);
		for (unsigned int i=57;i<width;i++){
			putchar(' ');
		}
		wfc_box_size_t row=scrolled_lines%(table_box_size+1);
		unsigned int index=scrolled_lines/(table_box_size+1)*tile_columns;
		unsigned int vertical_scroll_index=(max_scroll_height?(height-3)*2*scrolled_lines/max_scroll_height:0xffffffff);
		for (unsigned int i=0;i<height-2;i++){
			printf("\n\x1b[48;2;30;31;25m");
			if (row==table_box_size){
				for (unsigned int j=1;j<width;j++){
					putchar(' ');
				}
			}
			else{
				for (unsigned int j=index;j<index+tile_columns;j++){
					unsigned int space=j<index+tile_columns-1;
					if (j>=table.tile_count){
						space+=table_box_size;
					}
					else{
						for (unsigned int k=0;k<table_box_size;k++){
							wfc_color_t color=(table.tiles+j)->data[row*table_box_size+k];
							printf("\x1b[48;2;%u;%u;%um  ",color>>24,(color>>16)&0xff,(color>>8)&0xff);
						}
						printf("\x1b[48;2;30;31;25m");
					}
					while (space){
						space--;
						putchar(' ');
						putchar(' ');
					}
				}
				for (unsigned int j=1;j<tile_column_buffer;j++){
					putchar(' ');
				}
			}
			if (vertical_scroll_index&1){
				if ((vertical_scroll_index>>1)==i){
					printf("\x1b[38;2;154;153;150m▄");
				}
				else if (((vertical_scroll_index+1)>>1)==i){
					printf("\x1b[38;2;154;153;150m▀");
				}
				else{
					putchar(' ');
				}
			}
			else if ((vertical_scroll_index>>1)==i){
				printf("\x1b[48;2;154;153;150m ");
			}
			else{
				putchar(' ');
			}
			if (row==table_box_size){
				row=0;
				index+=tile_columns;
			}
			else{
				row++;
			}
		}
		if (changes){
			printf("\n\x1b[48;2;165;29;45m");
		}
		else{
			printf("\n\x1b[48;2;66;67;63m");
		}
		snprintf(line_buffer,4096,"%u \x1b[38;2;245;245;245mtile%c",table.tile_count,(table.tile_count==1?' ':'s'));
		printf("\x1b[38;2;143;240;164m%*s",width+19,line_buffer);
		fflush(stdout);
		int count=read(STDIN_FILENO,command,4);
		command[1]=(count>2?command[2]:0);
		switch (COMMAND(command[0],command[1])){
			case COMMAND(27,0):
				goto _return;
			case COMMAND(27,72):
				scrolled_lines=0;
				break;
			case COMMAND(27,70):
				scrolled_lines=max_scroll_height-height+table_box_size+2;
				if (scrolled_lines<0){
					scrolled_lines=0;
				}
_apply_scroll_limit:
				if (scrolled_lines>=max_scroll_height){
					scrolled_lines=max_scroll_height;
				}
				break;
			case COMMAND(27,53):
				scrolled_lines=(scrolled_lines>table_box_size+1?scrolled_lines-table_box_size-1:0);
				break;
			case COMMAND(27,54):
				scrolled_lines+=table_box_size+1;
				goto _apply_scroll_limit;
			case COMMAND(87,0):
			case COMMAND(119,0):
				if (scrolled_lines){
					scrolled_lines--;
				}
				break;
			case COMMAND(83,0):
			case COMMAND(115,0):
				scrolled_lines++;
				goto _apply_scroll_limit;
			case COMMAND(27,67):
_next_index:
				edit_index=(edit_index==MAX_EDIT_INDEX?0:edit_index+1);
				break;
			case COMMAND(27,68):
				edit_index=(edit_index?edit_index-1:MAX_EDIT_INDEX);
				break;
			case COMMAND(27,65):
				if (edit_index<3){
					ADJUST_VALUE_AT_EDIT_INDEX(config->downscale_factor,0,3,(digit<9?digit+1:0));
				}
				else if (edit_index<5){
					ADJUST_VALUE_AT_EDIT_INDEX(config->box_size,3,2,(digit<9?digit+1:0));
				}
				else if (edit_index<9){
					ADJUST_VALUE_AT_EDIT_INDEX(config->palette_max_size,5,4,(digit<9?digit+1:0));
				}
				else if (edit_index<15){
					ADJUST_VALUE_AT_EDIT_INDEX(config->max_color_diff,9,6,(digit<9?digit+1:0));
				}
				else if (edit_index==15){
					config->flags^=WFC_FLAG_FLIP;
				}
				else if (edit_index==16){
					config->flags^=WFC_FLAG_ROTATE;
				}
				else if (edit_index==17){
					config->flags^=WFC_FLAG_WRAP_X;
				}
				else if (edit_index==18){
					config->flags^=WFC_FLAG_WRAP_Y;
				}
				else if (edit_index==19){
					config->flags^=WFC_FLAG_WRAP_OUTPUT_X;
				}
				else{
					config->flags^=WFC_FLAG_WRAP_OUTPUT_Y;
				}
				changes=1;
				break;
			case COMMAND(27,66):
				if (edit_index<3){
					ADJUST_VALUE_AT_EDIT_INDEX(config->downscale_factor,0,3,(digit?digit-1:9));
				}
				else if (edit_index<5){
					ADJUST_VALUE_AT_EDIT_INDEX(config->box_size,3,2,(digit?digit-1:9));
				}
				else if (edit_index<9){
					ADJUST_VALUE_AT_EDIT_INDEX(config->palette_max_size,5,4,(digit?digit-1:9));
				}
				else if (edit_index<15){
					ADJUST_VALUE_AT_EDIT_INDEX(config->max_color_diff,9,6,(digit?digit-1:9));
				}
				else if (edit_index==15){
					config->flags^=WFC_FLAG_FLIP;
				}
				else if (edit_index==16){
					config->flags^=WFC_FLAG_ROTATE;
				}
				else if (edit_index==17){
					config->flags^=WFC_FLAG_WRAP_X;
				}
				else if (edit_index==18){
					config->flags^=WFC_FLAG_WRAP_Y;
				}
				else if (edit_index==19){
					config->flags^=WFC_FLAG_WRAP_OUTPUT_X;
				}
				else{
					config->flags^=WFC_FLAG_WRAP_OUTPUT_Y;
				}
				changes=1;
				break;
			case COMMAND(48,0):
			case COMMAND(49,0):
			case COMMAND(50,0):
			case COMMAND(51,0):
			case COMMAND(52,0):
			case COMMAND(53,0):
			case COMMAND(54,0):
			case COMMAND(55,0):
			case COMMAND(56,0):
			case COMMAND(57,0):
				if (edit_index<3){
					ADJUST_VALUE_AT_EDIT_INDEX(config->downscale_factor,0,3,command[0]-48);
				}
				else if (edit_index<5){
					ADJUST_VALUE_AT_EDIT_INDEX(config->box_size,3,2,command[0]-48);
				}
				else if (edit_index<9){
					ADJUST_VALUE_AT_EDIT_INDEX(config->palette_max_size,5,4,command[0]-48);
				}
				else if (edit_index<15){
					ADJUST_VALUE_AT_EDIT_INDEX(config->max_color_diff,9,6,command[0]-48);
				}
				else if (edit_index==15){
					ADJUST_FLAG_AT_INDEX(command[0]!=48,WFC_FLAG_FLIP);
				}
				else if (edit_index==16){
					ADJUST_FLAG_AT_INDEX(command[0]!=48,WFC_FLAG_ROTATE);
				}
				else if (edit_index==17){
					ADJUST_FLAG_AT_INDEX(command[0]!=48,WFC_FLAG_WRAP_X);
				}
				else if (edit_index==18){
					ADJUST_FLAG_AT_INDEX(command[0]!=48,WFC_FLAG_WRAP_Y);
				}
				else if (edit_index==19){
					ADJUST_FLAG_AT_INDEX(command[0]!=48,WFC_FLAG_WRAP_OUTPUT_X);
				}
				else{
					ADJUST_FLAG_AT_INDEX(command[0]!=48,WFC_FLAG_WRAP_OUTPUT_Y);
				}
				changes=1;
				if (edit_index!=2&&edit_index!=4&&edit_index!=8&&edit_index<15){
					goto _next_index;
				}
				break;
			case COMMAND(10,0):
				if (changes){
					wfc_free_table(&table);
					changes=2;
				}
				break;
		}
	}
_return:
	wfc_free_table(&table);
	printf("\x1b[0m\x1b[?25h\x1b[H\x1b[0JBox size: %u\nFlags:%s%s%s%s%s%s\nPalette size: %u\nSimilarity score: %u\n",config->box_size,((config->flags&WFC_FLAG_FLIP)?" WFC_FLAG_FLIP":""),((config->flags&WFC_FLAG_ROTATE)?" WFC_FLAG_ROTATE":""),((config->flags&WFC_FLAG_WRAP_X)?" WFC_FLAG_WRAP_X":""),((config->flags&WFC_FLAG_WRAP_Y)?" WFC_FLAG_WRAP_Y":""),((config->flags&WFC_FLAG_WRAP_OUTPUT_X)?" WFC_FLAG_WRAP_OUTPUT_X":""),((config->flags&WFC_FLAG_WRAP_OUTPUT_Y)?" WFC_FLAG_WRAP_OUTPUT_Y":""),config->palette_max_size,config->max_color_diff);
	tcsetattr(STDOUT_FILENO,TCSANOW,&old_terminal_config);
#endif
}



void wfc_build_table(const wfc_image_t* image,const wfc_config_t* config,wfc_table_t* out){
	if (!config->box_size){
		return;
	}
	wfc_size_t downscale_factor=config->downscale_factor;
	if (!downscale_factor){
		downscale_factor=1;
	}
	wfc_size_t width=(image->width+downscale_factor-1)/downscale_factor;
	wfc_size_t height=(image->height+downscale_factor-1)/downscale_factor;
	if (!width||!height){
		downscale_factor=1;
		width=image->width;
		height=image->height;
	}
	wfc_color_t* image_palette=malloc(width*height*sizeof(wfc_color_t));
	wfc_color_t* palette=NULL;
	wfc_palette_size_t palette_size=0;
	wfc_color_range_t color_range;
	INIT_RANGE(&color_range);
	wfc_size_t idx=0;
	for (wfc_size_t y=0;y<image->height;y+=downscale_factor){
		for (wfc_size_t x=0;x<image->width;x+=downscale_factor){
			wfc_color_t color=image->data[y*image->width+x];
			UPDATE_RANGE(&color_range,color);
			wfc_color_t j=0;
			while (j<palette_size&&palette[j]!=color){
				j++;
			}
			image_palette[idx]=j;
			idx++;
			if (j==palette_size){
				palette_size++;
				palette=realloc(palette,palette_size*sizeof(wfc_color_t));
				palette[palette_size-1]=color;
			}
		}
	}
	if (config->palette_max_size>1&&palette_size>config->palette_max_size){
		FINISH_RANGE(&color_range);
		uint32_t* indicies=malloc(palette_size*sizeof(wfc_palette_color_index_t));
		for (wfc_palette_size_t i=0;i<palette_size;i++){
			indicies[i]=i;
		}
		wfc_palette_range_t* ranges=malloc(config->palette_max_size*sizeof(wfc_palette_range_t));
		ranges->data=indicies;
		ranges->size=palette_size;
		ranges->range=color_range;
		wfc_palette_range_t* new_range=ranges+1;
		for (wfc_palette_size_t i=1;i<config->palette_max_size;i++){
			wfc_palette_size_t max_index=0;
			uint8_t max_offset=0;
			uint8_t max_diff=0;
			for (wfc_palette_size_t j=0;j<i;j++){
				const wfc_palette_range_t* range=ranges+j;
				if (range->size<2){
					continue;
				}
				for (uint8_t offset=0;offset<4;offset++){
					if (range->range.diff[offset]>max_diff){
						max_index=j;
						max_offset=offset;
						max_diff=range->range.diff[offset];
					}
				}
			}
			wfc_palette_range_t* range=ranges+max_index;
			wfc_palette_size_t size=range->size;
			_quicksort_palette(palette,range->data,size-1,0xff<<(max_offset<<3));
			range->size>>=1;
			new_range->data=range->data+range->size;
			new_range->size=size-range->size;
			INIT_RANGE(&color_range);
			for (wfc_palette_size_t j=0;j<size;j++){
				if (j==range->size){
					FINISH_RANGE(&color_range);
					range->range=color_range;
					INIT_RANGE(&color_range);
				}
				UPDATE_RANGE(&color_range,palette[range->data[j]]);
			}
			FINISH_RANGE(&color_range);
			new_range->range=color_range;
			new_range++;
		}
		for (wfc_palette_size_t i=0;i<config->palette_max_size;i++){
			const wfc_palette_range_t* range=ranges+i;
			wfc_color_t color=(*((uint32_t*)(range->range.min)))+(((*((uint32_t*)(range->range.diff)))&0xfefefefe)>>1);
			for (wfc_palette_size_t j=0;j<range->size;j++){
				palette[range->data[j]]=color;
			}
		}
		free(ranges);
		free(indicies);
	}
	for (wfc_size_t i=0;i<width*height;i++){
		image_palette[i]=palette[image_palette[i]];
	}
	free(palette);
	out->tile_count=0;
	out->tiles=NULL;
	out->downscale_factor=downscale_factor;
	wfc_color_t* buffer=malloc(config->box_size*config->box_size*sizeof(wfc_color_t));
	wfc_color_t* upscaled_data=malloc(downscale_factor*downscale_factor*sizeof(wfc_color_t));
	for (wfc_size_t x=0;x<width-((config->flags&WFC_FLAG_WRAP_X)?0:config->box_size-1);x++){
		for (wfc_size_t y=0;y<height-((config->flags&WFC_FLAG_WRAP_Y)?0:config->box_size-1);y++){
			wfc_color_t* ptr=buffer;
			wfc_size_t tx=x;
			wfc_size_t ty=y;
			for (wfc_box_size_t i=0;i<config->box_size*config->box_size;i++){
				*ptr=image_palette[(tx%width)+(ty%height)*width];
				ptr++;
				tx++;
				if ((i%config->box_size)==config->box_size-1){
					tx-=config->box_size;
					ty++;
				}
			}
			wfc_size_t k=0;
			for (wfc_size_t i=0;i<downscale_factor;i++){
				for (wfc_size_t j=0;j<downscale_factor;j++){
					wfc_size_t qi=(y*downscale_factor+i)%image->height;
					wfc_size_t qj=(x*downscale_factor+j)%image->width;
					upscaled_data[k]=image->data[qi*image->width+qj];
					k++;
				}
			}
			if (_add_tile(out,config,buffer,upscaled_data)){
				(out->tiles+out->tile_count-1)->_x=x*downscale_factor;
				(out->tiles+out->tile_count-1)->_y=y*downscale_factor;
				buffer=malloc(config->box_size*config->box_size*sizeof(wfc_color_t));
				upscaled_data=malloc(downscale_factor*downscale_factor*sizeof(wfc_color_t));
			}
		}
	}
	free(image_palette);
	wfc_size_t adjusted_upscaled_data_size=config->box_size*downscale_factor-1;
	if (config->flags&WFC_FLAG_ROTATE){
		for (wfc_tile_index_t i=0;i<out->tile_count;i++){
			const wfc_tile_t* tile=out->tiles+i;
			if (_WFC_TILE_GET_ROTATION(tile)==3){
				continue;
			}
			wfc_color_t* ptr=buffer;
			for (wfc_box_size_t y=0;y<config->box_size;y++){
				wfc_box_size_t x=config->box_size*config->box_size;
				while (x){
					x-=config->box_size;
					*ptr=tile->data[x+y];
					ptr++;
				}
			}
			wfc_size_t ox=_WFC_TILE_GET_POS(tile);
			wfc_size_t oy=tile->_y;
			wfc_size_t k=0;
			switch (_WFC_TILE_GET_ROTATION(tile)){
				case 0:
					for (wfc_size_t i=0;i<downscale_factor;i++){
						for (wfc_size_t j=0;j<downscale_factor;j++){
							wfc_size_t qi=(oy+adjusted_upscaled_data_size-j)%image->height;
							wfc_size_t qj=(ox+i)%image->width;
							upscaled_data[k]=image->data[qi*image->width+qj];
							k++;
						}
					}
					break;
				case 1:
					for (wfc_size_t i=0;i<downscale_factor;i++){
						for (wfc_size_t j=0;j<downscale_factor;j++){
							wfc_size_t qi=(oy+adjusted_upscaled_data_size-i)%image->height;
							wfc_size_t qj=(ox+adjusted_upscaled_data_size-j)%image->width;
							upscaled_data[k]=image->data[qi*image->width+qj];
							k++;
						}
					}
					break;
				case 2:
					for (wfc_size_t i=0;i<downscale_factor;i++){
						for (wfc_size_t j=0;j<downscale_factor;j++){
							wfc_size_t qi=(oy+j)%image->height;
							wfc_size_t qj=(ox+adjusted_upscaled_data_size-i)%image->width;
							upscaled_data[k]=image->data[qi*image->width+qj];
							k++;
						}
					}
					break;
			}
			if (_add_tile(out,config,buffer,upscaled_data)){
				(out->tiles+out->tile_count-1)->_x=(out->tiles+i)->_x+_WFC_TILE_ROTATED;
				(out->tiles+out->tile_count-1)->_y=(out->tiles+i)->_y;
				buffer=malloc(config->box_size*config->box_size*sizeof(wfc_color_t));
				upscaled_data=malloc(downscale_factor*downscale_factor*sizeof(wfc_color_t));
			}
		}
	}
	if (config->flags&WFC_FLAG_FLIP){
		for (wfc_tile_index_t i=0;i<out->tile_count;i++){
			const wfc_tile_t* tile=out->tiles+i;
			if (tile->_x&_WFC_TILE_FLIPPED){
				continue;
			}
			wfc_color_t* ptr=buffer;
			for (wfc_box_size_t y=0;y<config->box_size*config->box_size;y+=config->box_size){
				wfc_box_size_t x=config->box_size;
				while (x){
					x--;
					*ptr=tile->data[x+y];
					ptr++;
				}
			}
			wfc_size_t ox=_WFC_TILE_GET_POS(tile);
			wfc_size_t oy=tile->_y;
			wfc_size_t k=0;
			switch (_WFC_TILE_GET_ROTATION(tile)){
				case 0:
					for (wfc_size_t i=0;i<downscale_factor;i++){
						for (wfc_size_t j=0;j<downscale_factor;j++){
							wfc_size_t qi=(oy+i)%image->height;
							wfc_size_t qj=(ox+adjusted_upscaled_data_size-j)%image->width;
							upscaled_data[k]=image->data[qi*image->width+qj];
							k++;
						}
					}
					break;
				case 1:
					for (wfc_size_t i=0;i<downscale_factor;i++){
						for (wfc_size_t j=0;j<downscale_factor;j++){
							wfc_size_t qi=(oy+j)%image->height;
							wfc_size_t qj=(ox+i)%image->width;
							upscaled_data[k]=image->data[qi*image->width+qj];
							k++;
						}
					}
					break;
				case 2:
					for (wfc_size_t i=0;i<downscale_factor;i++){
						for (wfc_size_t j=0;j<downscale_factor;j++){
							wfc_size_t qi=(oy+adjusted_upscaled_data_size-i)%image->height;
							wfc_size_t qj=(ox+j)%image->width;
							upscaled_data[k]=image->data[qi*image->width+qj];
							k++;
						}
					}
					break;
				case 3:
					for (wfc_size_t i=0;i<downscale_factor;i++){
						for (wfc_size_t j=0;j<downscale_factor;j++){
							wfc_size_t qi=(oy+adjusted_upscaled_data_size-j)%image->height;
							wfc_size_t qj=(ox+adjusted_upscaled_data_size-i)%image->width;
							upscaled_data[k]=image->data[qi*image->width+qj];
							k++;
						}
					}
					break;
			}
			if (_add_tile(out,config,buffer,upscaled_data)){
				(out->tiles+out->tile_count-1)->_x=(out->tiles+i)->_x|_WFC_TILE_FLIPPED;
				(out->tiles+out->tile_count-1)->_y=(out->tiles+i)->_y;
				buffer=malloc(config->box_size*config->box_size*sizeof(wfc_color_t));
				upscaled_data=malloc(downscale_factor*downscale_factor*sizeof(wfc_color_t));
			}
		}
	}
	free(buffer);
	free(upscaled_data);
	if (out->tile_count<2){
		out->data_elem_size=((out->tile_count+255)>>6)&0xfffffffc;
		out->_connection_data=calloc(4*out->tile_count*out->data_elem_size*sizeof(uint64_t),1);
		if (out->tile_count){
			out->tiles->connections=out->_connection_data;
		}
		return;
	}
	if (config->max_color_diff){
		const wfc_tile_t* src_tile=out->tiles+1;
		wfc_tile_index_t i=1;
		while (i<out->tile_count){
			const wfc_color_t* tile_data=src_tile->data;
			*(out->tiles+i)=*src_tile;
			src_tile++;
			for (wfc_tile_index_t j=0;j<i;j++){
				const wfc_color_t* tile2_data=(out->tiles+j)->data;
				wfc_color_diffrence_t diff=0;
				for (wfc_box_size_t k=0;k<config->box_size*config->box_size;k++){
					diff+=_color_diffrence(tile_data[k],tile2_data[k]);
				}
				if (diff<config->max_color_diff){
					goto _delete_tile;
				}
			}
			i++;
			continue;
_delete_tile:
			free((src_tile-1)->data);
			out->tile_count--;
		}
	}
	out->data_elem_size=((out->tile_count+255)>>6)&0xfffffffc;
	out->_connection_data=malloc(4*out->tile_count*out->data_elem_size*sizeof(uint64_t));
	__m256i zero=_mm256_setzero_si256();
	__m256i* base_target=(__m256i*)(out->_connection_data);
	for (wfc_tile_index_t i=0;i<out->tile_count;i++){
		uint64_t* data=malloc(4*out->data_elem_size*sizeof(uint64_t));
		__m256i* ptr=(__m256i*)data;
		for (wfc_tile_index_t j=0;j<out->data_elem_size;j++){
			_mm256_storeu_si256(ptr,zero);
			ptr++;
		}
		(out->tiles+i)->connections=data;
		const wfc_color_t* base_tile_data=(out->tiles+i)->data;
		for (unsigned int j=0;j<4;j++){
			wfc_box_size_t extra_div=((j&1)?config->box_size-1:0xffffffff);
			wfc_box_size_t offset=(!j)*config->box_size+(j==3);
			const wfc_color_t* tile_data=base_tile_data+(j==1)+(j==2)*config->box_size;
			for (wfc_tile_index_t k=0;k<out->tile_count;k++){
				const wfc_color_t* tile2_data=(out->tiles+k)->data+offset;
				wfc_color_diffrence_t diff=0;
				for (wfc_box_size_t m=0;m<config->box_size*(config->box_size-1);m++){
					wfc_box_size_t n=m+m/extra_div;
					diff+=_color_diffrence(tile_data[n],tile2_data[n]);
					if (diff>config->max_color_diff){
						goto _skip_tile;
					}
				}
				data[k>>6]|=1ull<<(k&63);
_skip_tile:;
			}
			data+=out->data_elem_size;
		}
		ptr=(__m256i*)((out->tiles+i)->connections);
		__m256i* target=base_target;
		for (wfc_tile_index_t j=0;j<out->data_elem_size;j++){
			_mm256_storeu_si256(target,_mm256_lddqu_si256(ptr));
			target+=out->tile_count;
			ptr++;
		}
		base_target++;
	}
}



void wfc_free_state(const wfc_table_t* table,wfc_state_t* state){
	free(state->data);
	state->data=NULL;
	free(state->bitmap);
	state->bitmap=NULL;
	state->bitmap_size=0;
	state->length=0;
	for (wfc_tile_index_t i=0;i<table->tile_count;i++){
		free((state->queues+i)->data);
	}
	free(state->queues);
	state->queues=NULL;
	state->queue_size=0;
	free(state->weights);
	state->weights=NULL;
	free(state->queue_indicies);
	state->queue_indicies=NULL;
	free(state->update_stack);
	state->update_stack=NULL;
	free(state->delete_stack);
	state->delete_stack=NULL;
	free(state->fast_mask);
	state->fast_mask=NULL;
	state->pixel_count=0;
	state->width=0;
}



void wfc_free_table(wfc_table_t* table){
	while (table->tile_count){
		table->tile_count--;
		free((table->tiles+table->tile_count)->data);
		free((table->tiles+table->tile_count)->connections);
		free((table->tiles+table->tile_count)->upscaled_data);
	}	free(table->tiles);
	table->tiles=NULL;
	free(table->_connection_data);
	table->_connection_data=NULL;
}



void wfc_generate_image(const wfc_table_t* table,const wfc_state_t* state,wfc_image_t* out){
	const uint64_t* data=state->data;
	const wfc_queue_location_t* location=state->queue_indicies;
	wfc_color_t* ptr=out->data;
	for (wfc_size_t i=0;i<state->pixel_count;i++){
		if (location->queue_index==QUEUE_INDEX_COLLAPSED){
			*ptr=(table->tiles+_find_first_bit(data))->data[0];
			data+=table->data_elem_size;
		}
		else{
			wfc_tile_index_t count=table->tile_count+1;
			for (wfc_tile_index_t i=0;i<table->data_elem_size;i++){
				count-=POPULATION_COUNT(*data);
				data++;
			}
			*ptr=255+16777472*(count*255/(table->tile_count+1));
		}
		location++;
		ptr++;
	}
}



void wfc_generate_full_scale_image(const wfc_table_t* table,const wfc_state_t* state,wfc_image_t* out){
	const uint64_t* data=state->data;
	for (wfc_size_t y=0;y<out->height;y+=table->downscale_factor){
		for (wfc_size_t x=0;x<out->width;x+=table->downscale_factor){
			const wfc_tile_t* tile=table->tiles+_find_first_bit(data);
			data+=table->data_elem_size;
			wfc_color_t* ptr=out->data+x+y*out->width;
			for (wfc_size_t i=0;i<table->downscale_factor;i++){
				for (wfc_size_t j=0;j<table->downscale_factor;j++){
					*ptr=tile->upscaled_data[j+i*table->downscale_factor];
					ptr++;
				}
				ptr+=out->width-table->downscale_factor;
			}
		}
	}
}



void wfc_init_state(const wfc_table_t* table,const wfc_image_t* image,wfc_state_t* out){
	wfc_size_t pixel_count=image->width*image->height;
	uint8_t* prng_data=(uint8_t*)(out->prng.data);
	for (unsigned int i=0;i<256;i++){
		*prng_data=rand()&255;
		prng_data++;
	}
	out->prng.count=64;
	out->length=pixel_count*table->data_elem_size;
	out->data=malloc(out->length*sizeof(uint64_t));
	out->bitmap_size=(pixel_count+255)>>8;
	out->bitmap=malloc(out->bitmap_size<<5);
	out->queues=malloc(table->tile_count*sizeof(wfc_queue_t));
	out->queue_size=(pixel_count*sizeof(wfc_size_t)+31)>>5;
	for (wfc_tile_index_t i=0;i<table->tile_count;i++){
		(out->queues+i)->data=malloc(out->queue_size<<5);
	}
	out->weights=malloc(table->tile_count*sizeof(wfc_weight_t));
	out->queue_indicies=malloc((pixel_count*sizeof(wfc_queue_location_t)+31)&0xffffffffffffffe0ull);
	out->update_stack=malloc(pixel_count*sizeof(wfc_size_t));
	out->delete_stack=malloc(pixel_count*sizeof(wfc_size_t));
	out->fast_mask=malloc(262144*sizeof(wfc_fast_mask_t));
	out->pixel_count=pixel_count;
	out->width=image->width;
}



void wfc_print_image(const wfc_image_t* image){
	const wfc_color_t* ptr=image->data;
	for (wfc_size_t y=0;y<image->height;y++){
		for (wfc_size_t x=0;x<image->width;x++){
			printf("\x1b[48;2;%u;%u;%um  ",(*ptr)>>24,((*ptr)>>16)&0xff,((*ptr)>>8)&0xff);
			ptr++;
		}
		printf("\x1b[0m\n");
	}
}



void wfc_print_table(const wfc_table_t* table,const wfc_config_t* config){
	printf("Tiles: (%u)\n",table->tile_count);
	const wfc_tile_t* tile=table->tiles;
	for (wfc_tile_index_t i=0;i<table->tile_count;i++){
		printf(" [%u]\n",i);
		const uint64_t* connection_data=tile->connections;
		for (unsigned int j=0;j<4;j++){
			printf("  %c:","NESW"[j]);
			for (wfc_tile_index_t k=0;k<table->data_elem_size;k++){
				uint64_t tmp=*connection_data;
				connection_data++;
				while (tmp){
					printf(" %u",(k<<6)+FIND_FIRST_SET_BIT(tmp));
					tmp&=tmp-1;
				}
			}
			putchar('\n');
		}
		printf("  Transformation:\n    X: %u\n    Y: %u\n    Rotation: %u\n    Flip: %s\n  Data:\n",_WFC_TILE_GET_POS(tile),tile->_y,_WFC_TILE_GET_ROTATION(tile)*90,((tile->_x&_WFC_TILE_FLIPPED)?"Vertical":""));
		const wfc_color_t* data=tile->data;
		for (wfc_box_size_t j=0;j<config->box_size;j++){
			printf("   ");
			for (wfc_box_size_t k=0;k<config->box_size;k++){
				printf("\x1b[48;2;%u;%u;%um  ",(*data)>>24,((*data)>>16)&0xff,((*data)>>8)&0xff);
				data++;
			}
			printf("\x1b[0m\n");
		}
		printf("  Upscaled data:\n");
		data=tile->upscaled_data;
		for (wfc_box_size_t j=0;j<table->downscale_factor;j++){
			printf("   ");
			for (wfc_box_size_t k=0;k<table->downscale_factor;k++){
				printf("\x1b[48;2;%u;%u;%um  ",(*data)>>24,((*data)>>16)&0xff,((*data)>>8)&0xff);
				data++;
			}
			printf("\x1b[0m\n");
		}
		tile++;
	}
}



void wfc_save_image(const wfc_image_t* image,const char* path){
	FILE* fh=fopen(path,"wb");
	if (!fh){
		return;
	}
	uint32_t row_size=(image->width*3+3)&0xfffffffc;
	uint32_t data_size=row_size*image->height;
	uint32_t size=data_size+BMP_HEADER_SIZE;
	int32_t inv_height=-((int32_t)(image->height));
	uint8_t header[BMP_HEADER_SIZE]={'B','M',size&0xff,(size>>8)&0xff,(size>>16)&0xff,size>>24,0,0,0,0,BMP_HEADER_SIZE,0,0,0,DIB_HEADER_SIZE,0,0,0,image->width&0xff,(image->width>>8)&0xff,(image->width>>16)&0xff,image->width>>24,inv_height&0xff,(inv_height>>8)&0xff,(inv_height>>16)&0xff,inv_height>>24,1,0,24,0,BI_RGB,0,0,0,data_size&0xff,(data_size>>8)&0xff,(data_size>>16)&0xff,data_size>>24,0x13,0x0b,0,0,0x13,0x0b,0,0,0,0,0,0,0,0,0,0};
	fwrite(header,sizeof(uint8_t),BMP_HEADER_SIZE,fh);
	uint8_t* row_buffer=malloc(row_size*sizeof(uint8_t));
	const wfc_color_t* ptr=image->data;
	for (wfc_size_t i=0;i<image->height;i++){
		uint8_t* buffer_ptr=row_buffer;
		for (wfc_size_t j=0;j<image->width;j++){
			wfc_color_t color=*ptr;
			*buffer_ptr=color>>8;
			*(buffer_ptr+1)=color>>16;
			*(buffer_ptr+2)=color>>24;
			buffer_ptr+=3;
			ptr++;
		}
		fwrite(row_buffer,sizeof(uint8_t),row_size,fh);
	}
	free(row_buffer);
	fclose(fh);
}



float wfc_solve(const wfc_table_t* table,wfc_state_t* state,const wfc_config_t* config,wfc_callback_t callback,void* ctx){
	wfc_size_t direction_offsets[4]={-state->width,1,state->width,-1};
	wfc_size_t direction_offset_adjustment[4]={state->pixel_count,-state->width,-state->pixel_count,state->width};
	uint8_t no_wrap=(!(config->flags&WFC_FLAG_WRAP_OUTPUT_Y))*5+(!(config->flags&WFC_FLAG_WRAP_OUTPUT_X))*10;
	wfc_size_t height=state->pixel_count/state->width;
	uint64_t mult=0;
	uint8_t shift=FIND_LAST_SET_BIT(state->width);
	if (state->width&(state->width-1)){
		uint64_t n=1ull<<(shift+32);
		mult=n/state->width;
		uint32_t rem=n-mult*state->width;
		mult+=mult+1;
		uint32_t rem2=rem<<1;
		if (rem2>=state->width||rem2<rem){
			mult+=1;
		}
		mult&=0xffffffff;
	}
	else{
		shift--;
	}
	__m256i ones=_mm256_set1_epi8(0xff);
	__m256i data_mask=_mm256_srlv_epi32(ones,_mm256_subs_epu16(_mm256_set_epi32(256,224,192,160,128,96,64,32),_mm256_set1_epi32(table->tile_count&255)));
	__m256i zero=_mm256_setzero_si256();
	__m256i increment=_mm256_set1_epi32(8);
	__m256i popcnt_table=_mm256_setr_epi8(0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4);
	__m256i popcnt_low_mask=_mm256_set1_epi8(15);
	__m256i* ptr=(__m256i*)(state->fast_mask);
	for (wfc_size_t i=0;i<(sizeof(wfc_fast_mask_t)<<12);i++){
		_mm256_storeu_si256(ptr,zero);
		ptr++;
	}
	uint64_t cache_check_count=0;
	uint64_t cache_hit_count=0;
_retry_from_start:;
	ptr=(__m256i*)(state->data);
	for (wfc_size_t i=0;i<state->pixel_count;i++){
		for (wfc_tile_index_t j=0;j<(table->data_elem_size>>2)-1;j++){
			_mm256_storeu_si256(ptr,ones);
			ptr++;
		}
		_mm256_storeu_si256(ptr,data_mask);
		ptr++;
		(state->queue_indicies+i)->queue_index=table->tile_count-1;
		(state->queue_indicies+i)->delete_count=0;
		(state->queue_indicies+i)->index=i;
	}
	for (wfc_tile_index_t i=0;i<table->tile_count-1;i++){
		(state->queues+i)->length=0;
		state->weights[i]=state->pixel_count;
	}
	(state->queues+table->tile_count-1)->length=state->pixel_count;
	state->weights[table->tile_count-1]=state->pixel_count;
	__m256i counter=_mm256_set_epi32(7,6,5,4,3,2,1,0);
	ptr=(__m256i*)((state->queues+table->tile_count-1)->data);
	for (wfc_queue_size_t i=0;i<state->queue_size;i++){
		_mm256_storeu_si256(ptr,counter);
		counter=_mm256_add_epi32(counter,increment);
		ptr++;
	}
	while (1){
		if (callback){
			callback(table,state,ctx);
		}
		wfc_queue_t* queue=state->queues;
		wfc_tile_index_t qi=0;
		for (;qi<table->tile_count&&!queue->length;qi++){
			queue++;
		}
		if (qi==table->tile_count){
			return ((float)cache_hit_count)/cache_check_count;
		}
		wfc_size_t offset;
		wfc_tile_index_t tile_index=0;
		if (!qi){
			queue->length--;
			offset=queue->data[queue->length];
			tile_index=_find_first_bit(state->data+offset*table->data_elem_size);
		}
		else{
			wfc_queue_size_t index=(queue->length>1?_get_random(state,queue->length):0);
			offset=queue->data[index];
			queue->length--;
			queue->data[index]=queue->data[queue->length];
			(state->queue_indicies+queue->data[index])->index=index;
			wfc_weight_t weight_sum=0;
			uint64_t* data=state->data+offset*table->data_elem_size;
			for (wfc_tile_index_t i=0;i<table->data_elem_size;i++){
				uint64_t value=*data;
				*data=0;
				while (value){
					wfc_tile_index_t j=(i<<6)|FIND_FIRST_SET_BIT(value);
					value&=value-1;
					wfc_weight_t w=state->weights[j];
					weight_sum+=w;
					if ((_get_random(state,(weight_sum+1)<<WEIGHT_RANDOMNESS_SHIFT)>>WEIGHT_RANDOMNESS_SHIFT)<=w){
						tile_index=j;
					}
				}
				data++;
			}
			(*(state->data+offset*table->data_elem_size+(tile_index>>6)))|=1ull<<(tile_index&63);
		}
		(state->queue_indicies+offset)->queue_index=QUEUE_INDEX_COLLAPSED;
		wfc_weight_t weight=state->weights[tile_index];
		state->weights[tile_index]=(weight<=table->tile_count?1:weight-1);
		wfc_size_t update_stack_size=1;
		wfc_size_t delete_stack_size=0;
		state->update_stack[0]=offset;
		wfc_size_t root_x;
		wfc_size_t root_y;
		DIVMOD_WIDTH(offset,root_y,root_x);
		ptr=(__m256i*)(state->bitmap);
		for (wfc_size_t i=0;i<state->bitmap_size;i++){
			_mm256_storeu_si256(ptr,zero);
			ptr++;
		}
		while (update_stack_size){
			update_stack_size--;
			offset=state->update_stack[update_stack_size];
			wfc_size_t x;
			wfc_size_t y;
			DIVMOD_WIDTH(offset,y,x);
			uint8_t bounds=((!y)<<1)|((x==state->width-1)<<2)|((y==height-1)<<3)|((!x)<<4);
			const uint64_t* state_data_base=state->data+offset*table->data_elem_size;
			const __m256i* base_mask_data=(const __m256i*)(table->_connection_data);
			for (unsigned int i=0;i<4;i++){
				const __m256i* mask_data=base_mask_data;
				base_mask_data+=(table->data_elem_size*table->tile_count)>>2;
				wfc_size_t neightbour_offset=offset+direction_offsets[i];
				bounds>>=1;
				if (bounds&1){
					if (no_wrap&(1<<i)){
						continue;
					}
					neightbour_offset+=direction_offset_adjustment[i];
				}
				if ((state->queue_indicies+neightbour_offset)->queue_index==QUEUE_INDEX_COLLAPSED){
					continue;
				}
				__m256i* target=(__m256i*)(state->data+neightbour_offset*table->data_elem_size);
				__m256i sum_vector=_mm256_setzero_si256();
				__m256i mask=_mm256_undefined_si256();
				__m256i sub_mask=_mm256_undefined_si256();
				for (wfc_tile_index_t j=0;j<table->data_elem_size;j+=4){
					mask=_mm256_xor_si256(mask,mask);
					__m256i data=_mm256_lddqu_si256(target);
					const uint64_t* state_data=state_data_base;
					for (wfc_tile_index_t k=0;k<table->data_elem_size;k++){
						uint64_t value=*state_data;
						state_data++;
						if (!value){
							mask_data+=64;
							continue;
						}
						uint32_t fast_mask_offset=j*table->data_elem_size+k;
						uint32_t key_wide=value^(value>>32)^fast_mask_offset;
						wfc_fast_mask_t* fast_mask_data=state->fast_mask+((key_wide^(key_wide>>16))&0xffff)+(i<<16);
						cache_check_count++;
						if (fast_mask_data->offset==fast_mask_offset&&fast_mask_data->key==value){
							cache_hit_count++;
							sub_mask=_mm256_lddqu_si256((const __m256i*)(fast_mask_data->data));
							if (fast_mask_data->counter<FAST_MASK_MAX_COUNTER){
								fast_mask_data->counter++;
							}
						}
						else{
							sub_mask=_mm256_xor_si256(sub_mask,sub_mask);
							uint64_t fast_mask_data_key=value;
							do{
								sub_mask=_mm256_or_si256(sub_mask,_mm256_lddqu_si256(mask_data+FIND_FIRST_SET_BIT(value)));
								value&=value-1;
							} while (value);
							if (fast_mask_data->counter){
								fast_mask_data->counter--;
							}
							else{
								fast_mask_data->key=fast_mask_data_key;
								_mm256_storeu_si256((__m256i*)(fast_mask_data->data),sub_mask);
								fast_mask_data->offset=fast_mask_offset;
								fast_mask_data->counter=FAST_MASK_COUNTER_INIT;
							}
						}
						mask=_mm256_or_si256(mask,sub_mask);
						mask_data+=64;
					}
					data=_mm256_and_si256(data,mask);
					_mm256_storeu_si256(target,data);
					sum_vector=_mm256_add_epi64(sum_vector,_mm256_sad_epu8(_mm256_add_epi8(_mm256_shuffle_epi8(popcnt_table,_mm256_and_si256(data,popcnt_low_mask)),_mm256_shuffle_epi8(popcnt_table,_mm256_and_si256(_mm256_srli_epi32(data,4),popcnt_low_mask))),zero));
					mask_data-=(table->data_elem_size<<6)-table->tile_count;
					target++;
				}
				__m128i sum_vector_half=_mm_add_epi32(_mm256_castsi256_si128(sum_vector),_mm256_extractf128_si256(sum_vector,1));
				wfc_tile_index_t sum=_mm_cvtsi128_si32(_mm_add_epi64(sum_vector_half,_mm_unpackhi_epi64(sum_vector_half,sum_vector_half)));
				if (!sum){
					state->delete_stack[delete_stack_size]=neightbour_offset;
					delete_stack_size++;
					continue;
				}
				sum--;
				wfc_queue_location_t* location=state->queue_indicies+neightbour_offset;
				if (location->queue_index==sum){
					continue;
				}
				queue=state->queues+location->queue_index;
				queue->length--;
				queue->data[location->index]=queue->data[queue->length];
				(state->queue_indicies+queue->data[location->index])->index=location->index;
				queue=state->queues+sum;
				queue->data[queue->length]=neightbour_offset;
				location->queue_index=sum;
				location->index=queue->length;
				queue->length++;
				if (!(state->bitmap[neightbour_offset>>6]&(1ull<<(neightbour_offset&63)))){
					int32_t dx;
					int32_t dy;
					DIVMOD_WIDTH(neightbour_offset,dy,dx);
					dx-=root_x;
					dy-=root_y;
					if (dx<0){
						dx=-dx;
					}
					if (dy<0){
						dy=-dy;
					}
					if (dx+dy>config->propagation_distance){
						continue;
					}
					state->bitmap[neightbour_offset>>6]|=1ull<<(neightbour_offset&63);
					state->update_stack[update_stack_size]=neightbour_offset;
					update_stack_size++;
				}
			}
		}
		while (delete_stack_size){
			delete_stack_size--;
			offset=state->delete_stack[delete_stack_size];
			(state->queue_indicies+offset)->delete_count++;
			if ((state->queue_indicies+offset)->delete_count==MAX_ALLOWED_REMOVALS){
				goto _retry_from_start;
			}
			int32_t base_x;
			int32_t base_y;
			DIVMOD_WIDTH(offset,base_y,base_x);
			base_x+=_get_random(state,(config->delete_size<<1)+1)-config->delete_size;
			base_y+=_get_random(state,(config->delete_size<<1)+1)-config->delete_size;
			if (base_x<0){
				base_x=0;
			}
			else if (base_x>=state->width){
				base_x=state->width-1;
			}
			if (base_y<0){
				base_y=0;
			}
			else if (base_y>=height){
				base_y=height-1;
			}
			for (int32_t y=-config->delete_size;y<=((int32_t)(config->delete_size));y++){
				int32_t y_off=base_y+y;
				if (y_off<0){
					if (no_wrap&1){
						continue;
					}
					y_off+=height;
				}
				else if (y_off>=height){
					if (no_wrap&4){
						continue;
					}
					y_off-=height;
				}
				y_off*=state->width;
				for (int32_t x=-config->delete_size;x<=((int32_t)(config->delete_size));x++){
					int32_t x_off=base_x+x;
					if (x_off<0){
						if (no_wrap&8){
							continue;
						}
						x_off+=state->width;
					}
					else if (x_off>=state->width){
						if (no_wrap&2){
							continue;
						}
						x_off-=state->width;
					}
					offset=x_off+y_off;
					ptr=(__m256i*)(state->data+offset*table->data_elem_size);
					for (wfc_tile_index_t i=0;i<(table->data_elem_size>>2)-1;i++){
						_mm256_storeu_si256(ptr,ones);
						ptr++;
					}
					_mm256_storeu_si256(ptr,data_mask);
					wfc_queue_location_t* location=state->queue_indicies+offset;
					if (location->queue_index!=QUEUE_INDEX_COLLAPSED){
						queue=state->queues+location->queue_index;
						queue->length--;
						queue->data[location->index]=queue->data[queue->length];
						(state->queue_indicies+queue->data[location->index])->index=location->index;
					}
					queue=state->queues+table->tile_count-1;
					queue->data[queue->length]=offset;
					location->queue_index=table->tile_count-1;
					location->index=queue->length;
					queue->length++;
				}
			}
			uint8_t bounds=~(((base_y<config->delete_size+1)|((base_x>=state->width-config->delete_size-1)<<1)|((base_y>=height-config->delete_size-1)<<2)|((base_x<config->delete_size+1)<<3))&no_wrap);
			int32_t boundary_tiles[8];
			if (bounds&1){
				int32_t y=base_y-config->delete_size;
				if (y<0){
					y+=height;
				}
				boundary_tiles[0]=y*state->width;
				y--;
				if (y<0){
					y+=height;
				}
				boundary_tiles[1]=y*state->width;
			}
			if (bounds&2){
				int32_t x=base_x+config->delete_size;
				if (x>=state->width){
					x-=state->width;
				}
				boundary_tiles[2]=x;
				x++;
				if (x>=state->width){
					x-=state->width;
				}
				boundary_tiles[3]=x;
			}
			if (bounds&4){
				int32_t y=base_y+config->delete_size;
				if (y>=height){
					y-=height;
				}
				boundary_tiles[4]=y*state->width;
				y++;
				if (y>=height){
					y-=height;
				}
				boundary_tiles[5]=y*state->width;
			}
			if (bounds&8){
				int32_t x=base_x-config->delete_size;
				if (x<0){
					x+=state->width;
				}
				boundary_tiles[6]=x;
				x--;
				if (x<0){
					x+=state->width;
				}
				boundary_tiles[7]=x;
			}
			for (int32_t delta=-config->delete_size;delta<=((int32_t)(config->delete_size));delta++){
				int32_t delta_adj=delta;
				if (bounds&5){
					delta_adj+=base_x;
					if (delta_adj<0){
						if (no_wrap&2){
							goto _skip_x_loop;
						}
						delta_adj+=state->width;
					}
					else if (delta_adj>=state->width){
						if (no_wrap&8){
							goto _skip_x_loop;
						}
						delta_adj-=state->width;
					}
					if (bounds&1){
						offset=boundary_tiles[1]+delta_adj;
						if ((state->queue_indicies+offset)->queue_index==QUEUE_INDEX_COLLAPSED){
							const uint64_t* mask=(table->tiles+_find_first_bit(state->data+offset*table->data_elem_size))->connections+2*table->data_elem_size;
							offset=boundary_tiles[0]+delta_adj;
							uint64_t* data=state->data+offset*table->data_elem_size;
							wfc_tile_index_t count=0;
							for (wfc_tile_index_t j=0;j<table->data_elem_size;j++){
								uint64_t value=(*data)&(*mask);
								count+=POPULATION_COUNT(value);
								*data=value;
								data++;
								mask++;
							}
							if (!count){
								state->delete_stack[delete_stack_size]=offset;
								delete_stack_size++;
							}
							else{
								count--;
								wfc_queue_location_t* location=state->queue_indicies+offset;
								if (count!=location->queue_index){
									if (location->queue_index!=QUEUE_INDEX_COLLAPSED){
										queue=state->queues+location->queue_index;
										queue->length--;
										queue->data[location->index]=queue->data[queue->length];
										(state->queue_indicies+queue->data[location->index])->index=location->index;
									}
									queue=state->queues+count;
									queue->data[queue->length]=offset;
									location->queue_index=count;
									location->index=queue->length;
									queue->length++;
								}
							}
						}
					}
					if (bounds&4){
						offset=boundary_tiles[5]+delta_adj;
						if ((state->queue_indicies+offset)->queue_index==QUEUE_INDEX_COLLAPSED){
							const uint64_t* mask=(table->tiles+_find_first_bit(state->data+offset*table->data_elem_size))->connections;
							offset=boundary_tiles[4]+delta_adj;
							uint64_t* data=state->data+offset*table->data_elem_size;
							wfc_tile_index_t count=0;
							for (wfc_tile_index_t j=0;j<table->data_elem_size;j++){
								uint64_t value=(*data)&(*mask);
								count+=POPULATION_COUNT(value);
								*data=value;
								data++;
								mask++;
							}
							if (!count){
								state->delete_stack[delete_stack_size]=offset;
								delete_stack_size++;
							}
							else{
								count--;
								wfc_queue_location_t* location=state->queue_indicies+offset;
								if (count!=location->queue_index){
									if (location->queue_index!=QUEUE_INDEX_COLLAPSED){
										queue=state->queues+location->queue_index;
										queue->length--;
										queue->data[location->index]=queue->data[queue->length];
										(state->queue_indicies+queue->data[location->index])->index=location->index;
									}
									queue=state->queues+count;
									queue->data[queue->length]=offset;
									location->queue_index=count;
									location->index=queue->length;
									queue->length++;
								}
							}
						}
					}
				}
_skip_x_loop:;
				delta_adj=delta;
				if (bounds&10){
					delta_adj+=base_y;
					if (delta_adj<0){
						if (no_wrap&1){
							continue;
						}
						delta_adj+=height;
					}
					else if (delta_adj>=height){
						if (no_wrap&4){
							continue;
						}
						delta_adj-=height;
					}
					delta_adj*=state->width;
					if (bounds&2){
						offset=boundary_tiles[3]+delta_adj;
						if ((state->queue_indicies+offset)->queue_index==QUEUE_INDEX_COLLAPSED){
							const uint64_t* mask=(table->tiles+_find_first_bit(state->data+offset*table->data_elem_size))->connections+3*table->data_elem_size;
							offset=boundary_tiles[2]+delta_adj;
							uint64_t* data=state->data+offset*table->data_elem_size;
							wfc_tile_index_t count=0;
							for (wfc_tile_index_t j=0;j<table->data_elem_size;j++){
								uint64_t value=(*data)&(*mask);
								count+=POPULATION_COUNT(value);
								*data=value;
								data++;
								mask++;
							}
							if (!count){
								state->delete_stack[delete_stack_size]=offset;
								delete_stack_size++;
							}
							else{
								count--;
								wfc_queue_location_t* location=state->queue_indicies+offset;
								if (count!=location->queue_index){
									if (location->queue_index!=QUEUE_INDEX_COLLAPSED){
										queue=state->queues+location->queue_index;
										queue->length--;
										queue->data[location->index]=queue->data[queue->length];
										(state->queue_indicies+queue->data[location->index])->index=location->index;
									}
									queue=state->queues+count;
									queue->data[queue->length]=offset;
									location->queue_index=count;
									location->index=queue->length;
									queue->length++;
								}
							}
						}
					}
					if (bounds&8){
						offset=boundary_tiles[7]+delta_adj;
						if ((state->queue_indicies+offset)->queue_index==QUEUE_INDEX_COLLAPSED){
							const uint64_t* mask=(table->tiles+_find_first_bit(state->data+offset*table->data_elem_size))->connections+table->data_elem_size;
							offset=boundary_tiles[6]+delta_adj;
							uint64_t* data=state->data+offset*table->data_elem_size;
							wfc_tile_index_t count=0;
							for (wfc_tile_index_t j=0;j<table->data_elem_size;j++){
								uint64_t value=(*data)&(*mask);
								count+=POPULATION_COUNT(value);
								*data=value;
								data++;
								mask++;
							}
							if (!count){
								state->delete_stack[delete_stack_size]=offset;
								delete_stack_size++;
							}
							else{
								count--;
								wfc_queue_location_t* location=state->queue_indicies+offset;
								if (count!=location->queue_index){
									if (location->queue_index!=QUEUE_INDEX_COLLAPSED){
										queue=state->queues+location->queue_index;
										queue->length--;
										queue->data[location->index]=queue->data[queue->length];
										(state->queue_indicies+queue->data[location->index])->index=location->index;
									}
									queue=state->queues+count;
									queue->data[queue->length]=offset;
									location->queue_index=count;
									location->index=queue->length;
									queue->length++;
								}
							}
						}
					}
				}
			}
		}
	}
}
