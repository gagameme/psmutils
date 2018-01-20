#include <stdio.h>
#include <string.h>

#include "mc.h"
#include "mcb.h"
#include "actions.h"
#include "error.h"



/*
 * ブロックの取り出し
 */
void psmem_extract(struct Mem_ *output, void *base_mem, int block_n)
{
	int i;
	unsigned char *out_next = output->mem;
	unsigned char *infoframe = base_mem + OFFSET_BLOCK(0) + OFFSET_FRAME(block_n);
	unsigned char *block = base_mem + OFFSET_BLOCK(block_n);
	int block_num = 0; /* ブロック数 */
	int block_n_write = 0;
	int out_size = 0;

	/* データの先頭のブロックではない場合、エラー */
	if(infoframe[0] != DF_USED_HEAD)
		error_exit("no start block");

	block_num = block[0x03];

	/* MCBのヘッダ(データ名)を書き込む */
	memcpy(out_next, &infoframe[0x0A], SIZE_MCB_HEAD);
	out_next += SIZE_MCB_HEAD;
	out_size += SIZE_MCB_HEAD;

	block_n_write = block_n;
	for(i = 0; i < block_num; i++) {
		unsigned char *infoframe_write = base_mem + OFFSET_BLOCK(0) + OFFSET_FRAME(block_n_write);
		unsigned char *block_write = base_mem + OFFSET_BLOCK(block_n_write);
		if(block_n_write >= NUM_BLOCK)
			error_exit("bad info frame");
		memcpy(out_next, block_write, SIZE_BLOCK);
		out_next += SIZE_BLOCK;
		out_size += SIZE_BLOCK;
		block_n_write = infoframe_write[0x08] + 1;
	}

	output->size = out_size;
}


