#include <stdio.h>
#include <string.h>

#include "mcaio.h"

#include "error.h"
#include "operate.h"


static int read_mca(McaDev *md, unsigned char *rbuf, int block_index, int block_num);
static int write_mca(McaDev *md, unsigned char *wbuf, int block_index, int block_num);



int operate(struct Options_ *opt)
{
	int ps1m_f = 0;
	unsigned char buf[MC_SECTOR_SIZE * MC_SECTORS_PER_MC] = {0};
	McaDev *md = NULL;
	FILE *fp = NULL;

	// -lオプションを使用していない場合、全ブロックを対象とする
	if(!opt->l_f)
		opt->block_num = MC_BLOCKS_PER_MC;

	if((opt->block_index < 0) || (MC_BLOCKS_PER_MC <= opt->block_index)) {
		error_only("block index over");
		return 1;
	}

	if((opt->block_num < 1) || (MC_BLOCKS_PER_MC < opt->block_index + opt->block_num)) {
		error_only("block num over");
		return 1;
	}

	if(mca_open(&md) != 0) {
		error_only("mca open");
		return 1;
	}

	mca_ps1_is(md, &ps1m_f);
	if(!ps1m_f) {
		error_only("not ps1 memory");
		mca_close(md);
		return 1;
	}

	switch(opt->action) {
	case ACT_READ:
		if(opt->infname != NULL)
			break;   // 間違ったファイル指定

		if(opt->outfname == NULL)
			fp = stdout;
		else
			fp = fopen(opt->outfname, "wb");

		if(fp != NULL) {
			if(read_mca(md, buf, opt->block_index, opt->block_num) == 0)
				fwrite(buf, MC_SECTOR_SIZE , MC_SECTORS_PER_BLOCK * opt->block_num, fp);
		}

		break;
	case ACT_WRITE:
		if(opt->outfname != NULL)
			break;  // 間違ったファイル指定

		if(opt->infname == NULL)
			fp = stdin;
		else
			fp = fopen(opt->infname, "rb");

		if(fp != NULL) {
			int num = MC_SECTORS_PER_BLOCK * opt->block_num;
			int rnum = 0;
			rnum = fread(buf, MC_SECTOR_SIZE, num, fp);
			if(rnum == num)
				write_mca(md, buf, opt->block_index, opt->block_num);
		}

		break;
	default:
		error_only("action");
		break;
	}

	mca_close(md);

	if(fp == NULL) {
		error_only("file open");
		return 1;
	}

	if((fp != stdin) && (fp != stdout))
		fclose(fp);

	return 0;
}


/*
 * block_index番目のブロックからblock_num個のブロックを読み込む
 *
 * 成功したら0を返す
 */
static int read_mca(McaDev *md, unsigned char *rbuf, int block_index, int block_num)
{
	int i;
	int read_start;

	read_start = MC_SECTORS_PER_BLOCK * block_index;
	for(i = 0; i < MC_SECTORS_PER_BLOCK * block_num; i++) {
		int r;
		r = mca_ps1_read(md, &rbuf[i * MC_SECTOR_SIZE], read_start + i);
		if(r != 0) {
			error_only("read");
			return 1;
		}
		//fprintf(stderr, "%3d%%\r", ((i + 1) * 100) / (MC_SECTORS_PER_BLOCK * block_num));
	}

	return 0;
}


/*
 * block_index番目のブロックからblock_num個のブロックに書き込む
 *
 * 成功したら0を返す
 */
static int write_mca(McaDev *md, unsigned char *wbuf, int block_index, int block_num)
{
	int i;
	int write_start;

	write_start = MC_SECTORS_PER_BLOCK * block_index;
	for(i = 0; i < MC_SECTORS_PER_BLOCK * block_num; i++) {
		int r;
		r = mca_ps1_write(md, &wbuf[i * MC_SECTOR_SIZE], write_start + i);
		if(r != 0) {
			error_only("write");
			return 1;
		}
		//fprintf(stderr, "%3d%%\r", ((i + 1) * 100) / (MC_SECTORS_PER_BLOCK * block_num));
	}

	return 0;
}



