#include <stdio.h>
#include <string.h>

#include "mc.h"
#include "mcb.h"
#include "error.h"

#include "operate.h"
#include "actions.h"


static void write_mem(void *mem, int outsize, const char *outfname);



/*
 * 動作選択
 */
int operate(struct Options_ *opt)
{
	unsigned char base_mem[SIZE_MEM] = {0};
	unsigned char out_mem[SIZE_MEM] = {0};
	struct Mem_ output;

	output.mem = out_mem;
	output.size = SIZE_MEM;

	/*
	 * format_fが真の場合、
	 * ファイル読み込みをせずフォーマットしたデータを使う
	 */
	if(opt->format_f)
		mc_format(base_mem);
	else
		mc_read_basefile(opt->basefname, base_mem);

	switch(opt->action) {
	case ACT_NEWDATA:
		psmem_newdata(&output, base_mem, opt->block_index, opt->dest_index);
		break;
	case ACT_WRITE:
		psmem_write(&output, base_mem, opt->block_index, opt->infname);
		break;
	case ACT_SWAP:
		psmem_swap(&output, base_mem, opt->block_index, opt->dest_index);
		break;
	case ACT_TITLE:
		psmem_title(&output, base_mem, opt->block_index, opt->title);
		break;
	case ACT_DELETE:
		psmem_delete(&output, base_mem, opt->block_index);
		break;
	case ACT_PRINT:
		psmem_print(base_mem);
		output.mem = NULL;
		break;
	case ACT_EXTRACT:
		psmem_extract(&output, base_mem, opt->block_index);
		break;
	default:
		output.mem = NULL;
		break;
	}

	if(output.mem != NULL)
		write_mem(output.mem, output.size, opt->outfname);

	return 0;
}


/*
 * データ出力
 *
 * ファイル名指定時    ファイル
 * ファイル名未指定時  標準出力
 */
static void write_mem(void *mem, int outsize, const char *outfname)
{
	FILE *outfile = NULL;

	/* open */
	if(outfname != NULL) {
		if((outfile = fopen(outfname, "wb")) == NULL)
			error_exit("open outfile");
	}else
		outfile = stdout;

	/* write */
	fwrite(mem, 1, outsize, outfile);

	/* close */
	if(outfile != stdout)
		fclose(outfile);
}



