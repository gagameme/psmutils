#include <stdio.h>
#include <string.h>

#include "mc.h"
#include "mcb.h"
#include "actions.h"
#include "error.h"


static void write_infoframe_empty(void *mem, int block_n, int used_num);
static void write_datablock_empty(void *mem, int block_n, int used_num);



/*
 * 空データ作成
 */
void psmem_newdata(struct Mem_ *output, void *base_mem, int block_n, int len)
{

	if(block_n <= 0 || NUM_BLOCK <= block_n)
		error_exit("block number");
	if((len < 0) || (block_n + len > NUM_BLOCK))
		error_exit("block length");

	// ブロック数0の場合、何もしない
	if(len >= 1) {
		/* 管理ブロック書き換え */
		write_infoframe_empty(base_mem, block_n, len);

		/* データブロック書き換え */
		write_datablock_empty(base_mem, block_n, len);
	}

	/* 出力用のメモリをbase_memとする */
	output->mem = base_mem;
	output->size = SIZE_MEM;
}


/*
 * 管理ブロックのブロック情報フレームを書き換える
 */
static void write_infoframe_empty(void *mem, int block_n, int used_num)
{
	int i;
	const char *name = NAME_DEFAULT;

	/* 1ブロックのみの場合 */
	if(used_num == 1) {
		mc_write_infoframe(mem, block_n, DF_USED_HEAD, used_num * SIZE_BLOCK, NEXT_NOT_EXIST, name);
		return;
	}

	/* 複数ブロックの場合 */
	mc_write_infoframe(mem, block_n, DF_USED_HEAD, used_num * SIZE_BLOCK, block_n, name);
	for(i = 1; i < used_num - 1; i++)
		mc_write_infoframe(mem, block_n + i, DF_USED_MIDDLE, 0, block_n + i, "");
	mc_write_infoframe(mem, block_n + i, DF_USED_TAIL, 0, NEXT_NOT_EXIST, "");
}


/*
 * データブロックを書き換える
 */
static void write_datablock_empty(void *mem, int block_n, int used_num)
{
	int i;

	// 0埋め
	for( i = 0; i < used_num; i++) {
		unsigned char *block = mem + OFFSET_BLOCK(block_n + i);
		memset(block, 0, SIZE_BLOCK);
	}
}



