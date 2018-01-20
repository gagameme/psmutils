#include <stdio.h>
#include <string.h>

#include "mc.h"
#include "actions.h"
#include "error.h"



/*
 * ヘッダのみの削除
 */
void psmem_delete(struct Mem_ *output, void *base_mem, int block_n)
{
	struct Blkinfo bi;
	int next;

	if(block_n <= 0 || NUM_BLOCK <= block_n)
		error_exit("block number");

	mc_read_blockinfo(base_mem, block_n, &bi);
	if(bi.data_flag != DF_USED_HEAD)
		error_exit("no head block");

	// ヘッダの初期化
	mc_write_infoframe(base_mem, block_n, DF_UNUSED, 0, NEXT_NOT_EXIST, "");

	// 続きがあればそれも初期化
	next = bi.next + 1;
	while(next < NUM_BLOCK) {
		mc_read_blockinfo(base_mem, next, &bi);
		mc_write_infoframe(base_mem, next, DF_UNUSED, 0, NEXT_NOT_EXIST, "");
		next = bi.next + 1;
	}

	/* 出力用のメモリをbase_memとする */
	output->mem = base_mem;
	output->size = SIZE_MEM;
}


