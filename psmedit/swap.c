#include <stdio.h>
#include <string.h>

#include "mc.h"
#include "actions.h"
#include "error.h"


static void swap_frame(void *mem, int src_block, int dst_block);
static void update_next(void *mem, int src_block, int dst_block);
static void swap_block(void *mem, int src_block, int dst_block);



/*
 * ブロックの位置の交換
 */
void psmem_swap(struct Mem_ *output, void *base_mem, int src_block, int dst_block)
{
	if(src_block <= 0 || NUM_BLOCK <= src_block)
		error_exit("src block number");

	if(dst_block <= 0 || NUM_BLOCK <= dst_block)
		error_exit("dst block number");

	swap_frame(base_mem, src_block, dst_block);
	update_next(base_mem, src_block, dst_block);
	swap_block(base_mem, src_block, dst_block);

	/* 出力用のメモリをbase_memとする */
	output->mem = base_mem;
	output->size = SIZE_MEM;
}


/*
 * 管理ブロックのフレームを交換する
 */
static void swap_frame(void *mem, int src_block, int dst_block)
{
	unsigned char tmp_frame[SIZE_FRAME] = {0};
	unsigned char *src_frame = mem + OFFSET_BLOCK(0) + OFFSET_FRAME(src_block);
	unsigned char *dst_frame = mem + OFFSET_BLOCK(0) + OFFSET_FRAME(dst_block);

	memcpy(tmp_frame, src_frame, SIZE_FRAME);
	memcpy(src_frame, dst_frame, SIZE_FRAME);
	memcpy(dst_frame, tmp_frame, SIZE_FRAME);
}


/*
 * 次のブロック番号を更新する
 */
static void update_next(void *mem, int src_block, int dst_block)
{
	int i = 0;
	struct Blkinfo bi;

	for(i = 1; i < NUM_BLOCK; i++) {
		mc_read_blockinfo(mem, i, &bi);
		// 変更前のブロック番号だった場合、番号を書き換える
		if(bi.next == src_block - 1)
			mc_write_infoframe(mem, i, -1, -1, dst_block - 1, NULL);
		else if(bi.next == dst_block - 1)
			mc_write_infoframe(mem, i, -1, -1, src_block - 1, NULL);
	}
}


/*
 * ブロックを交換する
 */
static void swap_block(void *mem, int src_block, int dst_block)
{
	unsigned char tmp[SIZE_BLOCK] = {0};
	unsigned char *src = mem + OFFSET_BLOCK(src_block);
	unsigned char *dst = mem + OFFSET_BLOCK(dst_block);

	memcpy(tmp, src, SIZE_BLOCK);
	memcpy(src, dst, SIZE_BLOCK);
	memcpy(dst, tmp, SIZE_BLOCK);
}


