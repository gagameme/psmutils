#include <stdio.h>
#include <string.h>

#include "mc.h"
#include "mcb.h"
#include "actions.h"
#include "error.h"


static void write_infoframe_mcb(void *mem, int block_n, void *mcb, int data_size);
static void write_datablock_mcb(void *mem, int block_n, void *mcb, int data_size);



/*
 * ブロックの書き換え
 */
void psmem_write(struct Mem_ *output, void *base_mem, int block_n, const char *fname)
{
	unsigned char m_mcb[SIZE_MEM] = {0};
	int size_mcb;
	int size_mcb_data = 0;

	if(block_n <= 0 || NUM_BLOCK <= block_n)
		error_exit("block number");

	size_mcb = mcb_read_file(fname, m_mcb);

	/* ヘッダ以外のデータ部分のサイズ */
	size_mcb_data = size_mcb - SIZE_MCB_HEAD;

	/* 管理ブロック書き換え */
	write_infoframe_mcb(base_mem, block_n, m_mcb, size_mcb_data);

	/* データブロック書き換え */
	write_datablock_mcb(base_mem, block_n, m_mcb, size_mcb_data);

	/* 出力用のメモリをbase_memとする */
	output->mem = base_mem;
	output->size = SIZE_MEM;
}


/*
 * 管理ブロックのブロック情報フレームを書き換える
 */
static void write_infoframe_mcb(void *mem, int block_n, void *mcb, int data_size)
{
	int i;
	int used_num = REQUIRED_BLOCK(data_size);
	const char *name = mcb;

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
static void write_datablock_mcb(void *mem, int block_n, void *mcb, int data_size)
{
	void *mcb_data;

	mcb_data = mcb_get_data(mcb);
	
	mc_write_data(mem, block_n, mcb_data, data_size);

	// TODO データ形式変換を正しく行えるツールを作成した後は、この処理は削除する
	/* 先頭ブロックの使用ブロック数を正しい値に書き換える */
	{
		struct Blkinfo bi;
		mc_init_blockinfo(&bi);
		bi.used_num = REQUIRED_BLOCK(data_size);
		mc_write_blockinfo(mem, block_n, &bi);
	}
}



