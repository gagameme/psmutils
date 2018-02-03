/*
 * 生データの操作
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
#include "actions.h"
#include "error.h"


static int read_file(const char *fname, void *mem);
static void write_infoframe(void *mem, int block_n, int data_size);



/*
 * ブロックの取り出し
 */
void psmem_raw_read(struct Mem_ *output, void *base_mem, int block_n)
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

	block_n_write = block_n;
	for(i = 0; i < block_num; i++) {
		unsigned char *infoframe_write = base_mem + OFFSET_BLOCK(0) + OFFSET_FRAME(block_n_write);
		unsigned char *block_write = base_mem + OFFSET_BLOCK(block_n_write);
		if(block_n_write > NUM_DATABLOCK)
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
void psmem_raw_write(struct Mem_ *output, void *base_mem, int block_n, const char *fname)
{
	static unsigned char m_raw[SIZE_MEM];
	int size_raw = 0;

	if(block_n <= 0 || NUM_DATABLOCK < block_n)
		error_exit("block number");

	size_raw = read_file(fname, m_raw);

	/* 管理ブロック書き換え */
	write_infoframe(base_mem, block_n, size_raw);

	/* データブロック書き換え */
	mc_write_data(base_mem, block_n, m_raw, size_raw);

	/* 出力用のメモリをbase_memとする */
	output->mem = base_mem;
	output->size = SIZE_MEM;
}


/*
 * 管理ブロックのブロック情報フレームを書き換える
 */
static void write_infoframe(void *mem, int block_n, int data_size)
{
	int i;
	int used_num = REQUIRED_BLOCK(data_size);
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
 * ファイルfnameを元にして
 * memを初期化する
 *
 * 読み込んだサイズを返す
 */
static int read_file(const char *fname, void *mem)
{
	int fd;
	struct stat st;

	if((fd = open(fname, O_RDONLY)) == -1)
		error_exit("open rawfile");

	/* サイズチェック */
	if(fstat(fd, &st) == -1)
		error_exit("fstat rawfile");
	if(st.st_size > SIZE_BLOCK * NUM_DATABLOCK)
		error_exit("rawfile size");

	if(read(fd, mem, st.st_size) == -1)
		error_exit("read rawfile");

	close(fd);

	return st.st_size;
}



