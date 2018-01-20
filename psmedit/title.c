#include <stdio.h>
#include <string.h>

#include "mc.h"
#include "actions.h"
#include "error.h"



/*
 * データの名前を変更する
 */
void psmem_title(struct Mem_ *output, void *base_mem, int block_n, char *name)
{
	if(block_n <= 0 || NUM_BLOCK <= block_n)
		error_exit("block number");

	mc_write_infoframe(base_mem, block_n, -1, -1, -1, name);

	/* 出力用のメモリをbase_memとする */
	output->mem = base_mem;
	output->size = SIZE_MEM;
}


