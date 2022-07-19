#ifdef _MSC_VER
#include <intrin.h>
#else
#include <immintrin.h>
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

#define MODULO(a,b) ((a)>=(b)?(a)%(b):(a))



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
	return MODULO(state->prng.data[state->prng.count],n);
}



static FORCE_INLINE wfc_tile_index_t _find_first_bit(const uint64_t* data){
	wfc_tile_index_t out=0;
	while (!(*data)){
		data++;
		out+=64;
	}
	return out+FIND_FIRST_SET_BIT(*data);
}



void wfc_build_table(const wfc_image_t* image,wfc_box_size_t box_size,wfc_flags_t flags,wfc_table_t* out){
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
	out->data_elem_size=((out->tile_count+255)>>6)&0xfffffffc;
	__m256i zero=_mm256_setzero_si256();
	for (wfc_tile_index_t i=0;i<out->tile_count;i++){
		uint64_t* data=malloc(4*out->data_elem_size*sizeof(uint64_t));
		for (wfc_tile_index_t j=0;j<out->data_elem_size;j++){
			_mm256_storeu_si256((__m256i*)(data+(j<<2)),zero);
		}
		(out->tiles+i)->connections=data;
		const wfc_color_t* base_tile_data=(out->tiles+i)->data;
		for (unsigned int j=0;j<4;j++){
			wfc_box_size_t extra_div=((j&1)?box_size-1:0xffffffff);
			wfc_box_size_t offset=(!j)*box_size+(j==3);
			const wfc_color_t* tile_data=base_tile_data+(j==1)+(j==2)*box_size;
			for (wfc_tile_index_t k=0;k<out->tile_count;k++){
				const wfc_color_t* tile2_data=(out->tiles+k)->data+offset;
				for (wfc_box_size_t m=0;m<box_size*(box_size-1);m++){
					wfc_box_size_t n=m+m/extra_div;
					if (tile_data[n]!=tile2_data[n]){
						goto _skip_tile;
					}
				}
				data[k>>6]|=1ull<<(k&63);
_skip_tile:;
			}
			data+=out->data_elem_size;
		}
	}
}



void wfc_free_state(wfc_state_t* state){
	free(state->data);
	state->data=NULL;
	free(state->bitmap);
	state->bitmap=NULL;
	state->bitmap_size=0;
	state->length=0;
	for (wfc_tile_index_t i=0;i<state->tile_count;i++){
		free((state->queues+i)->data);
	}
	free(state->queues);
	state->queues=NULL;
	state->queue_size=0;
	free(state->weights);
	state->weights=NULL;
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
	const uint64_t* data=state->data;
	wfc_color_t* ptr=out->data;
	for (wfc_size_t i=0;i<state->pixel_count;i++){
		*ptr=((state->bitmap[i>>6]&(1ull<<(i&63)))?(table->tiles+_find_first_bit(data))->data[0]:0xff00ffff);
		data+=state->data_elem_size;
		ptr++;
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
	const char* direction_strings[4]={"N","E","S","W"};
	printf("Tiles: (%u)\n",table->tile_count);
	const wfc_tile_t* tile=table->tiles;
	for (wfc_tile_index_t i=0;i<table->tile_count;i++){
		printf(" [%u]\n",i);
		printf("  Hash: %.16lx\n",tile->hash);
		const uint64_t* connection_data=tile->connections;
		for (unsigned int j=0;j<4;j++){
			printf("  %s:",direction_strings[j]);
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



void wfc_solve(const wfc_table_t* table,wfc_state_t* state){
	wfc_size_t direction_offsets[4]={-state->width,1,state->width,-1};
	wfc_size_t direction_offset_adjustment[4]={state->pixel_count,-state->width,-state->pixel_count,state->width};
	uint8_t no_wrap=(!(table->flags&WFC_FLAG_WRAP_OUTPUT_Y))*5+(!(table->flags&WFC_FLAG_WRAP_OUTPUT_X))*10;
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
_restart_loop:;
	__m256i ones=_mm256_set1_epi8(0xff);
	__m256i mask=_mm256_srlv_epi32(ones,_mm256_subs_epu16(_mm256_set_epi32(256,224,192,160,128,96,64,32),_mm256_set1_epi32(state->tile_count&255)));
	__m256i* ptr=(__m256i*)(state->data);
	for (wfc_size_t i=0;i<state->pixel_count;i++){
		for (wfc_tile_index_t j=0;j<(state->data_elem_size>>2)-1;j++){
			_mm256_storeu_si256(ptr,ones);
			ptr++;
		}
		_mm256_storeu_si256(ptr,mask);
		ptr++;
	}
	__m256i zero=_mm256_setzero_si256();
	ptr=(__m256i*)(state->bitmap);
	for (wfc_size_t i=0;i<state->bitmap_size;i++){
		_mm256_storeu_si256(ptr,zero);
		ptr++;
	}
	for (wfc_tile_index_t i=0;i<state->tile_count-1;i++){
		(state->queues+i)->length=0;
		state->weights[i]=state->pixel_count;
	}
	(state->queues+state->tile_count-1)->length=state->pixel_count;
	state->weights[state->tile_count-1]=state->pixel_count;
	__m256i counter=_mm256_set_epi32(0,1,2,3,4,5,6,7);
	__m256i increment=_mm256_set1_epi32(8);
	ptr=(__m256i*)((state->queues+state->tile_count-1)->data);
	for (wfc_queue_size_t i=0;i<state->queue_size;i++){
		_mm256_storeu_si256(ptr,counter);
		counter=_mm256_add_epi32(counter,increment);
		ptr++;
	}
	while (1){
		wfc_queue_t* queue=state->queues;
		wfc_tile_index_t qi=0;
		for (;qi<state->tile_count&&!queue->length;qi++){
			queue++;
		}
		if (qi==state->tile_count){
			return;
		}
		wfc_size_t offset;
		wfc_tile_index_t tile_index=0;
		if (!qi){
			queue->length--;
			offset=queue->data[queue->length];
			tile_index=_find_first_bit(state->data+offset*state->data_elem_size);
		}
		else{
			wfc_queue_size_t index=(queue->length>1?_get_random(state,queue->length):1);
			offset=queue->data[index];
			queue->length--;
			queue->data[index]=queue->data[queue->length];
			if (state->bitmap[offset>>6]&(1ull<<(offset&63))){
				continue;
			}
			wfc_weight_t weight_sum=0;
			uint64_t* data=state->data+offset*state->data_elem_size;
			for (wfc_tile_index_t i=0;i<state->data_elem_size;i++){
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
			}
			data[tile_index>>6]|=1ull<<(tile_index&63);
		}
		state->bitmap[offset>>6]|=1ull<<(offset&63);
		wfc_weight_t weight=state->weights[tile_index];
		state->weights[tile_index]=(weight<=state->tile_count?1:weight-state->tile_count);
		wfc_size_t x=(offset*mult)>>32;
		x=offset-((((offset-x)>>1)+x)>>shift)*state->width;
		uint8_t bounds=(offset<state->width)|((x==state->width-1)<<1)|((offset>=state->pixel_count-state->width)<<2)|((!x)<<3);
		const uint64_t* mask=(table->tiles+tile_index)->connections;
		for (unsigned int i=0;i<4;i++){
			wfc_size_t neightbour_offset=offset+direction_offsets[i];
			if (bounds&(1<<i)){
				if (no_wrap&(1<<i)){
					mask+=state->data_elem_size;
					continue;
				}
				neightbour_offset+=direction_offset_adjustment[i];
			}
			if (state->bitmap[neightbour_offset>>6]&(1ull<<(neightbour_offset&63))){
				mask+=state->data_elem_size;
				continue;
			}
			uint64_t* target=state->data+neightbour_offset*state->data_elem_size;
			uint64_t change=0;
			wfc_tile_index_t sum=0;
			for (wfc_tile_index_t j=0;j<state->data_elem_size;j++){
				uint64_t old_value=*target;
				uint64_t value=old_value&(*mask);
				change|=value^old_value;
				sum+=POPULATION_COUNT(value);
				*target=value;
				mask++;
				target++;
			}
			if (!change){
				continue;
			}
			if (!sum){
				goto _restart_loop;
			}
			queue=state->queues+sum-1;
			queue->data[queue->length]=neightbour_offset;
			queue->length++;
		}
	}
}
