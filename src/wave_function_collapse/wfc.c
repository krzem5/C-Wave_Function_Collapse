#ifdef _MSC_VER
#include <intrin.h>
#else
#include <immintrin.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wfc.h>



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



static _Bool _add_tile(wfc_table_t* table,wfc_color_t* data){
	wfc_tile_hash_t hash=FNV_OFFSET_BASIS;
	for (wfc_box_size_t i=0;i<table->box_size*table->box_size;i++){
		hash=(hash^data[i])*FNV_PRIME;
	}
	for (wfc_tile_index_t i=0;i<table->tile_count;i++){
		if ((table->tiles+i)->hash==hash&&!memcmp((table->tiles+i)->data,data,table->box_size*table->box_size*sizeof(wfc_color_t))){
			return 0;
		}
	}
	table->tile_count++;
	table->tiles=realloc(table->tiles,table->tile_count*sizeof(wfc_tile_t));
	wfc_tile_t* tile=table->tiles+table->tile_count-1;
	tile->hash=hash;
	tile->data=data;
	return 1;
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
	const int32_t direction_offset[4]={box_size,-1,-box_size,1};
	out->box_size=box_size;
	out->flags=flags;
	out->tile_count=0;
	out->tiles=NULL;
	wfc_color_t* buffer=malloc(box_size*box_size*sizeof(wfc_color_t));
	for (wfc_size_t x=0;x<image->width-((flags&WFC_FLAG_WRAP_X)?0:box_size-1);x++){
		for (wfc_size_t y=0;y<image->height-((flags&WFC_FLAG_WRAP_Y)?0:box_size-1);y++){
			wfc_color_t* ptr=buffer;
			wfc_size_t tx=x;
			wfc_size_t ty=y;
			for (wfc_box_size_t i=0;i<box_size*box_size;i++){
				*ptr=image->data[(tx%image->width)+(ty%image->height)*image->width];
				ptr++;
				tx++;
				if ((i%box_size)==box_size-1){
					tx-=box_size;
					ty++;
				}
			}
			if (_add_tile(out,buffer)){
				buffer=malloc(box_size*box_size*sizeof(wfc_color_t));
			}
		}
	}
	if (flags&WFC_FLAG_ROTATE){
		for (wfc_tile_index_t i=0;i<out->tile_count;i++){
			const wfc_tile_t* tile=out->tiles+i;
			wfc_color_t* ptr=buffer;
			for (wfc_box_size_t y=0;y<box_size;y++){
				wfc_box_size_t x=box_size*box_size;
				while (x){
					x-=box_size;
					*ptr=tile->data[x+y];
					ptr++;
				}
			}
			if (_add_tile(out,buffer)){
				buffer=malloc(box_size*box_size*sizeof(wfc_color_t));
			}
		}
	}
	if (flags&WFC_FLAG_FLIP){
		for (wfc_tile_index_t i=0;i<out->tile_count;i++){
			const wfc_tile_t* tile=out->tiles+i;
			wfc_color_t* ptr=buffer;
			for (wfc_box_size_t y=0;y<box_size*box_size;y+=box_size){
				wfc_box_size_t x=box_size;
				while (x){
					x--;
					*ptr=tile->data[x+y];
					ptr++;
				}
			}
			if (_add_tile(out,buffer)){
				buffer=malloc(box_size*box_size*sizeof(wfc_color_t));
			}
		}
	}
	free(buffer);
	out->data_elem_size=(out->tile_count+63)>>6;
	for (wfc_tile_index_t i=0;i<out->tile_count;i++){
		uint64_t* data=calloc(4*out->data_elem_size,sizeof(uint64_t));
		(out->tiles+i)->connections=data;
		const wfc_color_t* tile_data=(out->tiles+i)->data;
		for (unsigned int j=0;j<4;j++){
			wfc_box_size_t sx=(j==1);
			wfc_box_size_t sy=(j==2)*box_size;
			wfc_box_size_t ex=box_size-(j==3);
			wfc_box_size_t ey=(box_size-(!j))*box_size;
			int32_t offset=direction_offset[j];
			for (wfc_tile_index_t k=0;k<out->tile_count;k++){
				const wfc_color_t* tile2_data=(out->tiles+k)->data+offset;
				for (wfc_box_size_t y=sy;y<ey;y+=box_size){
					for (wfc_box_size_t x=sx;x<ex;x++){
						if (tile_data[x+y]!=tile2_data[x+y]){
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
		free((table->tiles+table->tile_count)->data);
		free((table->tiles+table->tile_count)->connections);
	}
	free(table->tiles);
	table->tiles=NULL;
	table->box_size=0;
	table->flags=0;
}



void wfc_generate_image(const wfc_table_t* table,const wfc_state_t* state,wfc_image_t* out){
	wfc_color_t* ptr=out->data;
	const uint64_t* data=state->data;
	for (wfc_size_t y=0;y<out->height;y++){
		for (wfc_size_t x=0;x<out->width;x++){
			wfc_tile_index_t sum=0;
			for (wfc_tile_index_t i=0;i<state->data_elem_size;i++){
				sum+=POPULATION_COUNT(data[i]);
			}
			*ptr=(sum!=1?460544*sum+5263615:(table->tiles+_find_first_bit(data))->data[0]);
			data+=state->data_elem_size;
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



void wfc_print_table(const wfc_table_t* table){
	const char* direction_strings[4]={"  Up:","  Right:","  Down:","  Left:"};
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
		const uint64_t* connection_data=tile->connections;
		for (unsigned int j=0;j<4;j++){
			fputs(direction_strings[j],stdout);
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
		printf("  Data:\n");
		const wfc_color_t* data=tile->data;
		for (wfc_box_size_t j=0;j<table->box_size;j++){
			printf("   ");
			for (wfc_box_size_t k=0;k<table->box_size;k++){
				printf("\x1b[48;2;%u;%u;%um  ",(*data)>>24,((*data)>>16)&0xff,((*data)>>8)&0xff);
				data++;
			}
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
					tile_index+=_tzcnt_u64(_pdep_u64(1ull<<bit_index,*tmp));
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
				if (table->flags&WFC_FLAG_WRAP_OUTPUT_Y){
					tile_offset+=state->pixel_count;
				}
				else{
					continue;
				}
			}
			else if (i==1&&x==state->width-1){
				if (table->flags&WFC_FLAG_WRAP_OUTPUT_X){
					tile_offset-=state->width;
				}
				else{
					continue;
				}
			}
			else if (i==2&&y==height-1){
				if (table->flags&WFC_FLAG_WRAP_OUTPUT_Y){
					tile_offset-=state->pixel_count;
				}
				else{
					continue;
				}
			}
			else if (i==3&&!x){
				if (table->flags&WFC_FLAG_WRAP_OUTPUT_X){
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
				return 0;
			}
			if (old_sum!=1){
				new_sum--;
				(state->queues+new_sum)->data[(state->queues+new_sum)->length]=offset+tile_offset;
				(state->queues+new_sum)->length++;
			}
		}
	}

}
