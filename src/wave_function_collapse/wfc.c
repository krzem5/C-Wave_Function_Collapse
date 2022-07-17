#ifdef _MSC_VER
#include <intrin.h>
#else
#include <immintrin.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wfc.h>



#define RESOLVE_FLAGS(x,y) \
	do{ \
		if (x<0){ \
			x=((flags&WFC_FLAG_WRAP_X)?x+image->width:0); \
		} \
		if (y<0){ \
			y=((flags&WFC_FLAG_WRAP_Y)?y+image->height:0); \
		} \
		if (x>=image->width){ \
			x=((flags&WFC_FLAG_WRAP_X)?x-image->width:image->width-1); \
		} \
		if (y>=image->height){ \
			y=((flags&WFC_FLAG_WRAP_Y)?y-image->height:image->height-1); \
		} \
	} while (0)



#define FNV_OFFSET_BASIS 0xcbf29ce484222325
#define FNV_PRIME 0x100000001b3



#ifdef _MSC_VER
#pragma intrinsic(_BitScanForward64)
static __SLL_FORCE_INLINE __SLL_U32 FIND_FIRST_SET_BIT(__SLL_U64 m){
	unsigned long o;
	_BitScanForward64(&o,m);
	return o;
}
#define POPULATION_COUNT(m) __popcnt64((m))
#else
#define FIND_FIRST_SET_BIT(m) (__builtin_ffsll((m))-1)
#define POPULATION_COUNT(m) __builtin_popcountll((m))
#endif



const uint64_t _all_m256_bits[4]={0xffffffffffffffffull,0xffffffffffffffffull,0xffffffffffffffffull,0xffffffffffffffffull};



static wfc_tile_hash_t _hash_tile(const wfc_image_t* image,wfc_box_size_t box_size,int32_t x,int32_t y,wfc_flags_t flags){
	wfc_tile_hash_t out=FNV_OFFSET_BASIS;
	for (wfc_box_size_t i=0;i<box_size;i++){
		for (wfc_box_size_t j=0;j<box_size;j++){
			int32_t px=x;
			int32_t py=y;
			RESOLVE_FLAGS(px,py);
			out=(out^image->data[px+py*image->width])*FNV_PRIME;
			x++;
		}
		x-=box_size;
		y++;
	}
	return out;
}



static uint32_t _get_random(uint32_t n){
	if (n==1){
		return 0;
	}
	return rand()%n;
}



static wfc_tile_index_t _find_first_bit(const uint64_t* data){
	wfc_tile_index_t out=0;
	while (!(*data)){
		data++;
		out+=64;
	}
	return out+FIND_FIRST_SET_BIT(*data);
}



void wfc_build_table(const wfc_image_t* image,wfc_box_size_t box_size,wfc_flags_t flags,wfc_table_t* out){
	wfc_box_size_t half=(box_size-1)>>1;
	out->tile_count=0;
	out->tiles=NULL;
	for (wfc_size_t x=0;x<image->width;x++){
		for (wfc_size_t y=0;y<image->height;y++){
			wfc_tile_hash_t hash=_hash_tile(image,box_size,x-half,y-half,flags);
			for (wfc_tile_index_t i=0;i<out->tile_count;i++){
				if ((out->tiles+i)->hash!=hash){
					continue;
				}
				int32_t ix=x-half;
				int32_t iy=y-half;
				int32_t tx=(out->tiles+i)->x-half;
				int32_t ty=(out->tiles+i)->y-half;
				for (wfc_box_size_t j=0;j<box_size;j++){
					for (wfc_box_size_t k=0;k<box_size;k++){
						int32_t pix=ix;
						int32_t piy=iy;
						int32_t ptx=tx;
						int32_t pty=ty;
						RESOLVE_FLAGS(pix,piy);
						RESOLVE_FLAGS(ptx,pty);
						if (image->data[pix+piy*image->width]!=image->data[ptx+pty*image->width]){
							goto _not_correct_tile;
						}
					}
					ix-=box_size;
					iy++;
					tx-=box_size;
					ty++;
				}
				goto _tile_found;
_not_correct_tile:;
			}
			out->tile_count++;
			out->tiles=realloc(out->tiles,out->tile_count*sizeof(wfc_tile_t));
			(out->tiles+out->tile_count-1)->x=x;
			(out->tiles+out->tile_count-1)->y=y;
			(out->tiles+out->tile_count-1)->hash=hash;
_tile_found:;
		}
	}
	out->box_size=box_size;
	out->flags=flags;
	out->data_elem_size=(out->tile_count+63)>>6;
	for (wfc_tile_index_t i=0;i<out->tile_count;i++){
		wfc_tile_t* tile=out->tiles+i;
		uint64_t* data=calloc(4*out->data_elem_size,sizeof(uint64_t));
		int32_t tile_base_x=tile->x-half;
		int32_t tile_base_y=tile->y-half;
		tile->connections=data;
		for (unsigned int j=0;j<4;j++){
			wfc_box_size_t sx=(j==1);
			wfc_box_size_t sy=(j==2);
			wfc_box_size_t ex=box_size-(j==3);
			wfc_box_size_t ey=box_size-(!j);
			int32_t x_direction_offset=(j==1?-1:(j==3?1:0));
			int32_t y_direction_offset=(!j?1:(j==2?-1:0));
			printf("# %u [%u] -> (%d,%d) :  (%u,%u) : (%u,%u) : (%d,%d)\n",i,j,tile_base_x,tile_base_y,sx,sy,ex,ey,x_direction_offset,y_direction_offset);
			for (wfc_tile_index_t k=0;k<out->tile_count;k++){
				const wfc_tile_t* tile2=out->tiles+k;
				int32_t tile2_base_x=tile2->x-half+x_direction_offset;
				int32_t tile2_base_y=tile2->y-half+y_direction_offset;
				printf("  $ %u -> (%d,%d)\n",k,tile2_base_x,tile2_base_y);
				for (wfc_box_size_t y=sy;y<ey;y++){
					for (wfc_box_size_t x=sx;x<ex;x++){
						int32_t tile_x=tile_base_x+x;
						int32_t tile_y=tile_base_y+y;
						int32_t tile2_x=tile2_base_x+x;
						int32_t tile2_y=tile2_base_y+y;
						RESOLVE_FLAGS(tile_x,tile_y);
						RESOLVE_FLAGS(tile2_x,tile2_y);
						printf("    [%u %u] -> (%u,%u) : (%u,%u)\n",x,y,tile_x,tile_y,tile2_x,tile2_y);
						if (image->data[tile_x+tile_y*image->width]!=image->data[tile2_x+tile2_y*image->width]){
							goto _skip_tile;
						}
					}
				}
				data[k>>6]|=1ull<<(k&63);
_skip_tile:;
			}
			data+=out->data_elem_size;
		}
	}
}



void wfc_clear_state(wfc_state_t* state){
	uint64_t* tmp=malloc(state->data_elem_size*sizeof(uint64_t));
	uint64_t* bit_ptr=tmp;
	wfc_tile_index_t bit_count=state->tile_count;
	while (bit_count){
		if (bit_count>63){
			bit_count-=64;
			*bit_ptr=0xffffffffffffffff;
		}
		else{
			*bit_ptr=(1ull<<bit_count)-1;
			bit_count=0;
		}
		bit_ptr++;
	}
	uint64_t* data=state->data;
	for (wfc_size_t i=0;i<state->pixel_count;i++){
		for (wfc_tile_index_t j=0;j<state->data_elem_size;j++){
			*data=*(tmp+j);
			data++;
		}
	}
	free(tmp);
	for (wfc_tile_index_t i=0;i<state->tile_count-1;i++){
		(state->queues+i)->length=0;
	}
	for (wfc_queue_size_t i=0;i<state->pixel_count;i++){
		(state->queues+state->tile_count-1)->data[i]=i;
	}
	(state->queues+state->tile_count-1)->length=state->pixel_count;
}



void wfc_free_state(wfc_state_t* state){
	free(state->data);
	state->data=NULL;
	state->length=0;
	for (wfc_tile_index_t i=0;i<state->tile_count;i++){
		free((state->queues+i)->data);
	}
	state->queues=NULL;
	state->tile_count=0;
	state->data_elem_size=0;
	state->pixel_count=0;
	state->width=0;
}



void wfc_free_table(wfc_table_t* table){
	while (table->tile_count){
		table->tile_count--;
		free((table->tiles+table->tile_count)->connections);
	}
	free(table->tiles);
	table->tiles=NULL;
	table->box_size=0;
	table->flags=0;
}



void wfc_generate_image(const wfc_table_t* table,const wfc_state_t* state,const wfc_image_t* image,wfc_image_t* out){
	wfc_color_t* ptr=out->data;
	const uint64_t* data=state->data;
	for (wfc_size_t y=0;y<out->height;y++){
		for (wfc_size_t x=0;x<out->width;x++){
			const wfc_tile_t* tile=table->tiles+_find_first_bit(data);
			wfc_tile_index_t sum=0;
			for (wfc_tile_index_t i=0;i<state->data_elem_size;i++){
				sum+=POPULATION_COUNT(data[i]);
			}
			data+=state->data_elem_size;
			*ptr=(sum!=1?0x00010100*(80+7*sum)+0xff:image->data[tile->x+tile->y*image->width]);
			ptr++;
		}
	}
}



void wfc_init_state(const wfc_table_t* table,const wfc_image_t* image,wfc_state_t* out){
	wfc_size_t pixel_count=image->width*image->height;
	out->length=(pixel_count*table->data_elem_size+3)>>2;
	out->data=malloc(out->length<<5);
	out->queues=malloc(table->tile_count*sizeof(wfc_queue_t));
	for (wfc_tile_index_t i=0;i<table->tile_count;i++){
		(out->queues+i)->data=malloc(pixel_count*sizeof(wfc_size_t));
	}
	out->tile_count=table->tile_count;
	out->data_elem_size=table->data_elem_size;
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



void wfc_print_table(const wfc_table_t* table,const wfc_image_t* image){
	const char* direction_strings[4]={"  Up:","  Right:","  Down:","  Left:"};
	wfc_flags_t flags=table->flags;
	wfc_box_size_t half=(table->box_size-1)>>1;
	printf("Tiles: (%u)\n",table->tile_count);
	int width=1;
	wfc_tile_index_t tmp=table->tile_count;
	while (tmp>9){
		tmp/=10;
		width++;
	}
	const wfc_tile_t* tile=table->tiles;
	for (wfc_tile_index_t i=0;i<table->tile_count;i++){
		printf(" [%*u]\n",width,i);
		printf("  Hash: %.16lx\n",tile->hash);
		const uint64_t* data=tile->connections;
		for (unsigned int j=0;j<4;j++){
			fputs(direction_strings[j],stdout);
			for (wfc_tile_index_t k=0;k<table->data_elem_size;k++){
				uint64_t tmp=*data;
				data++;
				while (tmp){
					printf(" %u",(k<<6)+FIND_FIRST_SET_BIT(tmp));
					tmp&=tmp-1;
				}
			}
			putchar('\n');
		}
		printf("  Data:\n");
		int32_t x=tile->x-half;
		int32_t y=tile->y-half;
		for (wfc_box_size_t j=0;j<table->box_size;j++){
			printf("   ");
			for (wfc_box_size_t k=0;k<table->box_size;k++){
				int32_t px=x;
				int32_t py=y;
				RESOLVE_FLAGS(px,py);
				wfc_color_t c=image->data[px+py*image->width];
				printf("\x1b[48;2;%u;%u;%um  ",c>>24,(c>>16)&0xff,(c>>8)&0xff);
				x++;
			}
			x-=table->box_size;
			y++;
			printf("\x1b[0m\n");
		}
		tile++;
	}
}



_Bool wfc_solve(const wfc_table_t* table,wfc_state_t* state){
	wfc_size_t direction_offsets[4]={-state->width,1,state->width,-1};
	while (1){
		wfc_queue_t* queue=state->queues;
		wfc_tile_index_t qi=0;
		for (;!queue->length&&qi<state->tile_count;qi++){
			queue++;
		}
		if (qi==state->tile_count){
			return 1;
		}
		wfc_queue_size_t index=_get_random(queue->length);
		wfc_size_t offset=queue->data[index]*state->data_elem_size;
		queue->length--;
		queue->data[index]=queue->data[queue->length];
		uint64_t* data=state->data+offset;
		wfc_tile_index_t tile_index=0;
		if (!qi){
			tile_index=_find_first_bit(data);
		}
		else{
			wfc_tile_index_t bit_index=_get_random(qi+1);
			const uint64_t* tmp=data;
			wfc_tile_index_t sum=0;
			for (wfc_tile_index_t i=0;i<table->data_elem_size;i++){
				wfc_tile_index_t bit_cnt=POPULATION_COUNT(*tmp);
				sum+=bit_cnt;
				if (bit_cnt>bit_index){
					uint64_t value=*tmp;
					while (bit_index){
						tile_index++;
						if (value&1){
							bit_index--;
						}
						value>>=1;
					}
					break;
				}
				bit_index-=bit_cnt;
				tile_index+=64;
				tmp++;
			}
			if (sum==1){
				continue;
			}
			memset(data,0,state->data_elem_size*sizeof(uint64_t));
			data[tile_index>>6]=1ull<<(tile_index&63);
		}
		wfc_size_t x=offset%state->width;
		wfc_size_t y=offset/state->width;
		wfc_size_t height=state->pixel_count/state->width;
		const wfc_tile_t* tile=table->tiles+tile_index;
		for (unsigned int i=0;i<4;i++){
			int32_t tile_offset=direction_offsets[i];
			if (!i&&!y){
				if (table->flags&WFC_FLAG_WRAP_Y){
					tile_offset+=state->pixel_count;
				}
				else{
					continue;
				}
			}
			else if (i==1&&x==state->width-1){
				if (table->flags&WFC_FLAG_WRAP_X){
					tile_offset-=state->width;
				}
				else{
					continue;
				}
			}
			else if (i==2&&y==height-1){
				if (table->flags&WFC_FLAG_WRAP_Y){
					tile_offset-=state->pixel_count;
				}
				else{
					continue;
				}
			}
			else if (i==3&&!x){
				if (table->flags&WFC_FLAG_WRAP_X){
					tile_offset+=state->width;
				}
				else{
					continue;
				}
			}
			const uint64_t* mask=tile->connections+i*table->data_elem_size;
			uint64_t* target=data+tile_offset*state->data_elem_size;
			wfc_tile_index_t old_sum=0;
			wfc_tile_index_t new_sum=0;
			for (wfc_tile_index_t j=0;j<table->data_elem_size;j++){
				old_sum+=POPULATION_COUNT(*target);
				(*target)&=*mask;
				new_sum+=POPULATION_COUNT(*target);
				mask++;
				target++;
			}
			if (!new_sum){
				return 1;
			}
			if (old_sum!=1){
				new_sum--;
				(state->queues+new_sum)->data[(state->queues+new_sum)->length]=offset+tile_offset;
				(state->queues+new_sum)->length++;
			}
		}
	}

}
