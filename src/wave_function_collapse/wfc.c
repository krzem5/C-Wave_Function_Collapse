#include <stdio.h>
#include <stdlib.h>
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



void wfc_build_table(const wfc_image_t* image,wfc_box_size_t box_size,wfc_flags_t flags,wfc_table_t* out){
	wfc_box_size_t half=(box_size-1)>>1;
	out->tile_count=0;
	out->tiles=NULL;
	wfc_tile_index_t* tiles=malloc(image->width*image->height*sizeof(wfc_tile_index_t));
	for (wfc_size_t x=0;x<image->width;x++){
		for (wfc_size_t y=0;y<image->height;y++){
			wfc_tile_hash_t hash=_hash_tile(image,box_size,x-half,y-half,flags);
			wfc_tile_index_t i=0;
			for (;i<out->tile_count;i++){
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
				break;
_not_correct_tile:;
			}
			if (i==out->tile_count){
				out->tile_count++;
				out->tiles=realloc(out->tiles,out->tile_count*sizeof(wfc_tile_t));
				(out->tiles+i)->x=x;
				(out->tiles+i)->y=y;
				(out->tiles+i)->hash=hash;
			}
			tiles[x+y*image->width]=i;
		}
	}
	free(tiles);
	out->box_size=box_size;
	out->flags=flags;
}



void wfc_free_table(wfc_table_t* table){
	table->tile_count=0;
	free(table->tiles);
	table->tiles=NULL;
	table->box_size=0;
	table->flags=0;
}



void wfc_print_image(const wfc_image_t* image){
	const wfc_color_t* ptr=image->data;
	for (wfc_size_t x=0;x<image->width;x++){
		for (wfc_size_t y=0;y<image->height;y++){
			wfc_color_t c=*ptr;
			ptr++;
			printf("\x1b[48;2;%u;%u;%um  ",c>>24,(c>>16)&0xff,(c>>8)&0xff);
		}
		printf("\x1b[0m\n");
	}
}



void wfc_print_table(const wfc_table_t* table,const wfc_image_t* image){
	wfc_flags_t flags=table->flags;
	wfc_box_size_t half=(table->box_size-1)>>1;
	printf("Tiles: (%u)\n",table->tile_count);
	int width=1;
	wfc_tile_index_t tmp=table->tile_count;
	while (tmp>9){
		tmp/=10;
		width++;
	}
	for (wfc_tile_index_t i=0;i<table->tile_count;i++){
		printf("  [%*u]: (%.16lx)\n",width,i,(table->tiles+i)->hash);
		int32_t x=(table->tiles+i)->x-half;
		int32_t y=(table->tiles+i)->y-half;
		for (wfc_box_size_t j=0;j<table->box_size;j++){
			printf("    ");
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
	}
}
