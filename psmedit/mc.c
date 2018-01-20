/*
 * メモリ操作の共通関数
 */
#include <stdio.h>
#include <string.h>
#include <limits.h>

/*
 * open() close() fstat()
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "mc.h"
#include "error.h"

#define INT_BYTE 4


static void mc_write_xor_frame(void *m_frame);



/*
 * memのフォーマット
 * 128kBであること(関数内でサイズチェックはしていない)
 */
void mc_format(void *mem)
{
	int i;

	/* set zero */
	memset(mem, 0, SIZE_MEM);

	/* block0の初期化 */
	mc_format_block0(mem);

	/* block1 ~ 15 の初期化 */
	for(i = 1; i < NUM_BLOCK; i++)
		mc_format_datablock(mem, i);
}


/*
 * block0の初期化
 */
void mc_format_block0(void *mem)
{
	int i;
	unsigned char *block0 = mem + OFFSET_BLOCK(0);
	unsigned char *fr0 = block0 + OFFSET_FRAME(0);
	unsigned char *fr_wt = block0 + OFFSET_FRAME(63);

	/* frame0 */
	fr0[0] = 'M';
	fr0[1] = 'C';
	mc_write_xor_frame(fr0);

	/* frame1 ~ 15 (ブロック情報フレーム)の初期化 */
	for(i = 1; i <  NUM_BLOCK; i++)
		mc_write_infoframe(block0, i, DF_UNUSED, 0, NEXT_NOT_EXIST, "");

	/* frame16 ~ 35 (Broken Secter List) の初期化 */
	for(i = 16; i <= 35; i++) {
		unsigned char *fr = block0 + OFFSET_FRAME(i);
		fr[0x00] = 0xff;
		fr[0x01] = 0xff;
		fr[0x02] = 0xff;
		fr[0x03] = 0xff;
		mc_write_xor_frame(fr);
	}

	/* frame36 ~ 55 (Broken Sector Replacement Data)の初期化 */
	for(i = 36; i <= 55; i++) {
		unsigned char *fr = block0 + OFFSET_FRAME(i);
		memset(fr, 0xff, SIZE_FRAME);
	}

	/* frame56 ~ 62 (Unused Frames)の初期化 */
	for(i = 56; i <= 62; i++) {
		unsigned char *fr = block0 + OFFSET_FRAME(i);
		memset(fr, 0xff, SIZE_FRAME);
	}

	/* frame63 (Write Test Frame)の初期化 */
	fr_wt[0] = 'M';
	fr_wt[1] = 'C';
	mc_write_xor_frame(fr_wt);
}


/*
 * データブロックの初期化
 * 0埋め
 */
void mc_format_datablock(void *mem, int block_n)
{
	unsigned char *block = mem + OFFSET_BLOCK(block_n);

	memset(block, 0, SIZE_BLOCK);
}


/*
 * ファイルbasefnameを元にして
 * memを初期化する
 *
 * basefnameがNULLの場合、標準入力を使用する
 *
 * 読み込んだサイズを返す(常にSIZE_MEM)
 */
int mc_read_basefile(const char *basefname, void *mem)
{

	if(basefname == NULL) {
		/*
		 * 標準入力で初期化
		 */
		fread(mem, 1, SIZE_MEM, stdin);
	}else {
		/*
		 * ファイルの内容で初期化
		 */
		int fd;
		struct stat st;

		if((fd = open(basefname, O_RDONLY)) == -1)
			error_exit("open basefile");

		/* サイズチェック */
		if(fstat(fd, &st) == -1)
			error_exit("fstat basefile");
		if(st.st_size != SIZE_MEM)
			error_exit("basefile size");

		if(read(fd, mem, SIZE_MEM) == -1)
			error_exit("read basefile");

		close(fd);
	}

	return SIZE_MEM;
}



/*
 * ブロックのヘッダ情報を読み込み
 * biにセットする
 */
void mc_read_blockinfo(const void *mem, int block_n, struct Blkinfo *bi)
{
	int i;
	const unsigned char *infoframe = mem + OFFSET_BLOCK(0) + OFFSET_FRAME(block_n);
	const unsigned char *block = mem + OFFSET_BLOCK(block_n);

	if(bi == NULL)
		return;

	/* データフラグ取得 */
	bi->data_flag = infoframe[0x00];

	/* 使用バイト数取得 */
	/* infoframe[0x04]からリトルエンディアンで格納されている */
	bi->used_size = 0;
	for(i = INT_BYTE - 1; i >= 0; i--)
		bi->used_size = (bi->used_size << CHAR_BIT) | infoframe[0x04 + i];

	/*
	 * リンク取得
	 *
	 * 実際は2バイトだが、
	 * 0x00~0x0e、0xff の値しかとらないので
	 * 0x08番地のみで十分
	 */
	bi->next = infoframe[0x08];

	/* 使用ブロック数取得 */
	bi->used_num = block[0x03];

	/* データの名前 */
	bi->name = (char *)(&infoframe[0x0A]);
}


/*
 * biの値をブロックのヘッダ情報に書き込む
 *
 * bi->name       NULL以外の場合のみ
 * bi->data_flag  0以上の場合のみ
 * bi->used_size  0以上の場合のみ
 * bi->next       0以上の場合のみ
 * bi->used_num   0以上の場合のみ
 */
void mc_write_blockinfo(void *mem, int block_n, const struct Blkinfo *bi)
{
	unsigned char *infoframe = mem + OFFSET_BLOCK(0) + OFFSET_FRAME(block_n);
	unsigned char *block = mem + OFFSET_BLOCK(block_n);

	if(bi == NULL)
		return;

	/*
	 * データ名
	 * SIZE_MEM_NAMEより短い場合は0埋めする
	 */
	if(bi->name != NULL) {
		int size_name = strlen(bi->name);
		if(size_name > SIZE_MEM_NAME)
			size_name = SIZE_MEM_NAME;
		memset(&infoframe[0x0A], 0, SIZE_MEM_NAME);
		memcpy(&infoframe[0x0A], bi->name, size_name);
		/*
		 * 最後が'\0'である必要はない
		 * (名前の領域の後は常に0)
		 */
	}

	/* データフラグ */
	if(bi->data_flag >= 0)
		infoframe[0x00] = bi->data_flag;

	/*
	 * 使用サイズ
	 * infoframe[0x04]からリトルエンディアンで格納
	 */
	if(bi->used_size >= 0) {
		int i;
		for(i = 0; i < INT_BYTE; i++)
			infoframe[0x04 + i] = (unsigned char)((bi->used_size >> (i * CHAR_BIT)) & UCHAR_MAX);
	}

	/* 次のブロックの番号 */
	if(bi->next >= 0) {
		infoframe[0x08] = bi->next;
		/* 上位バイト */
		if(bi->next == NEXT_NOT_EXIST)
			infoframe[0x09] = NEXT_HIGH_NOT_EXIST;
		else
			infoframe[0x09] = NEXT_HIGH;
	}

	/*
	 * 使用ブロック数
	 * この要素は管理ブロックではなく
	 * データブロックに書き込まれる
	 */
	if(bi->used_num >= 0)
		block[0x03] = bi->used_num;

	mc_write_xor_frame(infoframe);
}


/*
 * biをmc_write_blockinfo()で
 * 何も書き込まない値となるように初期化する
 */
void mc_init_blockinfo(struct Blkinfo *bi)
{
	if(bi == NULL)
		return;

	bi->name = NULL;
	bi->data_flag = -1;
	bi->used_size = -1;
	bi->next = -1;
	bi->used_num = -1;
}


/*
 * ブロック情報フレームの書き換え
 *
 * ブロック情報フレームに以下の値を書き込む
 * df    データフラグ
 *         0以上の場合のみ
 * size  使用サイズ
 *         0以上の場合のみ
 * next  次のブロック番号
 *         0以上の場合のみ
 * name  データ名
 *         NULL以外の場合のみ
 */
void mc_write_infoframe(void *mem, int block_n, int df, int size, int next, const char *name)
{
	struct Blkinfo bi;

	mc_init_blockinfo(&bi);
	bi.name = (char *)name;
	bi.data_flag = df;
	bi.used_size = size;
	bi.next = next;

	mc_write_blockinfo(mem, block_n, &bi);
}


/*
 * 1フレーム分のxor計算をして
 * フレームの最後に書き込む
 */
static void mc_write_xor_frame(void *m_frame)
{
	unsigned char x = 0;
	unsigned char *current = m_frame;
	unsigned char *end = current + SIZE_FRAME -1;

	/* 最後のバイト以外のxorを計算 */
	while(current < end)
		x ^= *current++;

	*current = x;
}


/*
 * block_nブロックからsizeバイトの内容を読み込み、dataに書き込む
 */
void mc_read_data(const void *mem, int block_n, void *data, int size)
{
	const unsigned char *block;

	if(block_n <= 0 || NUM_BLOCK <= block_n)
		error_exit("bad block number");

	if(REQUIRED_BLOCK(size) > NUM_BLOCK - block_n)
		error_exit("bad data size");

	block = mem + OFFSET_BLOCK(block_n);

	memcpy(data, block, size);
}


/*
 * sizeバイトのdataをblock_nブロック以降に書き込む
 *
 * サイズチェックを行う
 * 管理ブロックの書き換えは行わない
 */
void mc_write_data(void *mem, int block_n, const void *data, int size)
{
	unsigned char *block;

	if(block_n <= 0 || NUM_BLOCK <= block_n)
		error_exit("bad block number");

	if(REQUIRED_BLOCK(size) > NUM_BLOCK - block_n)
		error_exit("bad data size");

	block = mem + OFFSET_BLOCK(block_n);

	memcpy(block, data, size);
}




/********** debug **********/

/*
 * print 1 frame (128 byte)
 */
void mc_print_frame(void *m_frame)
{
	int i;
	unsigned char *c = m_frame;

	for(i = 0; i < SIZE_FRAME; i++) {
		if(i && (i % 16 == 0))
			fputc('\n', stderr);

		fprintf(stderr, "%02x ", *(c + i));
	}

	fputc('\n', stderr);
}



