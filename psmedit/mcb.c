/*
 * mcbフォーマットの操作
 */

#include <string.h>
/*
 * open() close() fstat()
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "mc.h"
#include "mcb.h"
#include "actions.h"
#include "error.h"


static void *mcb_get_data(const void *mcb);
static int mcb_read_file(const char *mcbfname, void *mem);
static void write_infoframe(void *mem, int block_n, void *mcb, int data_size);
static void write_datablock(void *mem, int block_n, void *mcb, int data_size);



/*
 * ブロックの取り出し
 */
void psmem_mcb_extract(struct Mem_ *output, void *base_mem, int block_n)
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


/*
 * ブロックの書き換え
 */
void psmem_mcb_write(struct Mem_ *output, void *base_mem, int block_n, const char *fname)
{
	static unsigned char m_mcb[SIZE_MEM] = {0};
	int size_mcb;
	int size_mcb_data = 0;

	if(block_n <= 0 || NUM_BLOCK <= block_n)
		error_exit("block number");

	size_mcb = mcb_read_file(fname, m_mcb);

	/* ヘッダ以外のデータ部分のサイズ */
	size_mcb_data = size_mcb - SIZE_MCB_HEAD;

	/* 管理ブロック書き換え */
	write_infoframe(base_mem, block_n, m_mcb, size_mcb_data);

	/* データブロック書き換え */
	write_datablock(base_mem, block_n, m_mcb, size_mcb_data);

	/* 出力用のメモリをbase_memとする */
	output->mem = base_mem;
	output->size = SIZE_MEM;
}


/*
 * 管理ブロックのブロック情報フレームを書き換える
 */
static void write_infoframe(void *mem, int block_n, void *mcb, int data_size)
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
static void write_datablock(void *mem, int block_n, void *mcb, int data_size)
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


/*
 * ファイルmcbfnameを元にして
 * memを初期化する
 *
 * 読み込んだサイズを返す
 */
static int mcb_read_file(const char *mcbfname, void *mem)
{
	int fd;
	struct stat st;

	if((fd = open(mcbfname, O_RDONLY)) == -1)
		error_exit("open mcbfile");

	/* サイズチェック */
	if(fstat(fd, &st) == -1)
		error_exit("fstat mcbfile");
	if((st.st_size % SIZE_BLOCK != SIZE_MCB_HEAD) || st.st_size > SIZE_MCB_MAX)
		error_exit("mcbfile size");

	if(read(fd, mem, st.st_size) == -1)
		error_exit("read mcbfile");

	close(fd);

	return st.st_size;
}


/*
 * mcbのデータ部分を取得する
 */
static void *mcb_get_data(const void *mcb)
{
	const unsigned char *m_mcb = mcb;

	return (void *)(m_mcb + SIZE_MCB_HEAD);
}


