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
#define QUEUE_INDEX_COLLAPSED 0xffff
#define MAX_ALLOWED_REMOVALS 128

#define DIVMOD_WIDTH(number,div,mod) \
	do{ \
		wfc_size_t __number=(number); \
		wfc_size_t __div=(__number*mult)>>32; \
		__div=(((__number-__div)>>1)+__div)>>shift; \
		div=__div; \
		mod=__number-__div*state->width; \
	} while (0)


#define BMP_HEADER_SIZE 54
#define DIB_HEADER_SIZE 40
#define BI_RGB 0



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
	free(state->queue_indicies);
	state->queue_indicies=NULL;
	free(state->update_stack);
	state->update_stack=NULL;
	free(state->delete_stack);
	state->delete_stack=NULL;
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
	const wfc_queue_location_t* location=state->queue_indicies;
	wfc_color_t* ptr=out->data;
	for (wfc_size_t i=0;i<state->pixel_count;i++){
		if (location->queue_index==QUEUE_INDEX_COLLAPSED){
			*ptr=(table->tiles+_find_first_bit(data))->data[0];
			data+=state->data_elem_size;
		}
		else{
			wfc_tile_index_t count=table->tile_count+1;
			for (wfc_tile_index_t i=0;i<state->data_elem_size;i++){
				count-=POPULATION_COUNT(*data);
				data++;
			}
			*ptr=255+16777472*(count*255/(table->tile_count+1));
		}
		location++;
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
	out->queue_indicies=malloc((pixel_count*sizeof(wfc_queue_location_t)+31)&0xffffffffffffffe0ull);
	out->update_stack=malloc(pixel_count*sizeof(wfc_size_t));
	out->delete_stack=malloc(pixel_count*sizeof(wfc_size_t));
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



void wfc_solve(const wfc_table_t* table,wfc_state_t* state,wfc_callback_t callback,void* ctx){
	wfc_size_t direction_offsets[4]={-state->width,1,state->width,-1};
	wfc_size_t direction_offset_adjustment[4]={state->pixel_count,-state->width,-state->pixel_count,state->width};
	uint8_t no_wrap=(!(table->flags&WFC_FLAG_WRAP_OUTPUT_Y))*5+(!(table->flags&WFC_FLAG_WRAP_OUTPUT_X))*10;
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
	__m256i data_mask=_mm256_srlv_epi32(ones,_mm256_subs_epu16(_mm256_set_epi32(256,224,192,160,128,96,64,32),_mm256_set1_epi32(state->tile_count&255)));
	__m256i zero=_mm256_setzero_si256();
	__m256i increment=_mm256_set1_epi32(8);
	__m256i popcnt_table=_mm256_setr_epi8(0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4);
	__m256i popcnt_low_mask=_mm256_set1_epi8(15);
_retry_from_start:;
	__m256i* ptr=(__m256i*)(state->data);
	for (wfc_size_t i=0;i<state->pixel_count;i++){
		for (wfc_tile_index_t j=0;j<(state->data_elem_size>>2)-1;j++){
			_mm256_storeu_si256(ptr,ones);
			ptr++;
		}
		_mm256_storeu_si256(ptr,data_mask);
		ptr++;
		(state->queue_indicies+i)->queue_index=table->tile_count-1;
		(state->queue_indicies+i)->delete_count=0;
		(state->queue_indicies+i)->index=i;
	}
	for (wfc_tile_index_t i=0;i<state->tile_count-1;i++){
		(state->queues+i)->length=0;
		state->weights[i]=state->pixel_count;
	}
	(state->queues+state->tile_count-1)->length=state->pixel_count;
	state->weights[state->tile_count-1]=state->pixel_count;
	__m256i counter=_mm256_set_epi32(7,6,5,4,3,2,1,0);
	ptr=(__m256i*)((state->queues+state->tile_count-1)->data);
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
			wfc_queue_size_t index=(queue->length>1?_get_random(state,queue->length):0);
			offset=queue->data[index];
			queue->length--;
			queue->data[index]=queue->data[queue->length];
			(state->queue_indicies+queue->data[index])->index=index;
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
				data++;
			}
			(*(state->data+offset*state->data_elem_size+(tile_index>>6)))|=1ull<<(tile_index&63);
		}
		(state->queue_indicies+offset)->queue_index=QUEUE_INDEX_COLLAPSED;
		wfc_weight_t weight=state->weights[tile_index];
		state->weights[tile_index]=(weight<=state->tile_count?1:weight-1);
		wfc_size_t update_stack_size=1;
		wfc_size_t delete_stack_size=0;
		state->update_stack[0]=offset;
		while (update_stack_size){
			update_stack_size--;
			offset=state->update_stack[update_stack_size];
			state->bitmap[offset>>6]&=~(1ull<<(offset&63));
			wfc_size_t x;
			wfc_size_t y;
			DIVMOD_WIDTH(offset,y,x);
			uint8_t bounds=((!y)<<1)|((x==state->width-1)<<2)|((y==height-1)<<3)|((!x)<<4);
			const uint64_t* state_data_base=state->data+offset*state->data_elem_size;
			for (unsigned int i=0;i<4;i++){
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
				__m256i* target=(__m256i*)(state->data+neightbour_offset*state->data_elem_size);
				__m256i sum_vector=_mm256_setzero_si256();
				__m256i mask=_mm256_undefined_si256();
				for (wfc_tile_index_t j=0;j<(state->data_elem_size>>2);j++){
					mask=_mm256_xor_si256(mask,mask);
					__m256i data=_mm256_lddqu_si256(target);
					const uint64_t* state_data=state_data_base;
					for (wfc_tile_index_t k=0;k<state->data_elem_size;k++){
						uint64_t value=*state_data;
						while (value){
							mask=_mm256_or_si256(mask,_mm256_lddqu_si256((const __m256i*)((table->tiles+(k<<6)+FIND_FIRST_SET_BIT(value))->connections+i*state->data_elem_size+(j<<2))));
							value&=value-1;
						}
						state_data++;
					}
					data=_mm256_and_si256(data,mask);
					_mm256_storeu_si256(target,data);
					sum_vector=_mm256_add_epi64(sum_vector,_mm256_sad_epu8(_mm256_add_epi8(_mm256_shuffle_epi8(popcnt_table,_mm256_and_si256(data,popcnt_low_mask)),_mm256_shuffle_epi8(popcnt_table,_mm256_and_si256(_mm256_srli_epi32(data,4),popcnt_low_mask))),zero));
					mask++;
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
			wfc_size_t base_x;
			wfc_size_t base_y;
			DIVMOD_WIDTH(offset,base_y,base_x);
			base_x+=_get_random(state,(table->box_size<<1)+1)-table->box_size;
			base_y+=_get_random(state,(table->box_size<<1)+1)-table->box_size;
			for (int32_t y=-table->box_size;y<=((int32_t)(table->box_size));y++){
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
				for (int32_t x=-table->box_size;x<=((int32_t)(table->box_size));x++){
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
					ptr=(__m256i*)(state->data+offset*state->data_elem_size);
					for (wfc_tile_index_t i=0;i<(state->data_elem_size>>2)-1;i++){
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
			uint8_t bounds=~(((base_y<table->box_size+1)|((base_x>=state->width-table->box_size-1)<<1)|((base_y>=height-table->box_size-1)<<2)|((base_x<table->box_size+1)<<3))&no_wrap);
			wfc_size_t boundary_tiles[8];
			if (bounds&1){
				int32_t y=base_y-table->box_size;
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
				int32_t x=base_x+table->box_size;
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
				int32_t y=base_y+table->box_size;
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
				int32_t x=base_x-table->box_size;
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
			for (int32_t delta=-table->box_size;delta<=((int32_t)(table->box_size));delta++){
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
							const uint64_t* mask=(table->tiles+_find_first_bit(state->data+offset*state->data_elem_size))->connections+2*state->data_elem_size;
							offset=boundary_tiles[0]+delta_adj;
							uint64_t* data=state->data+offset*state->data_elem_size;
							wfc_tile_index_t count=0;
							for (wfc_tile_index_t j=0;j<state->data_elem_size;j++){
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
							const uint64_t* mask=(table->tiles+_find_first_bit(state->data+offset*state->data_elem_size))->connections;
							offset=boundary_tiles[4]+delta_adj;
							uint64_t* data=state->data+offset*state->data_elem_size;
							wfc_tile_index_t count=0;
							for (wfc_tile_index_t j=0;j<state->data_elem_size;j++){
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
							const uint64_t* mask=(table->tiles+_find_first_bit(state->data+offset*state->data_elem_size))->connections+3*state->data_elem_size;
							offset=boundary_tiles[2]+delta_adj;
							uint64_t* data=state->data+offset*state->data_elem_size;
							wfc_tile_index_t count=0;
							for (wfc_tile_index_t j=0;j<state->data_elem_size;j++){
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
							const uint64_t* mask=(table->tiles+_find_first_bit(state->data+offset*state->data_elem_size))->connections+state->data_elem_size;
							offset=boundary_tiles[6]+delta_adj;
							uint64_t* data=state->data+offset*state->data_elem_size;
							wfc_tile_index_t count=0;
							for (wfc_tile_index_t j=0;j<state->data_elem_size;j++){
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
