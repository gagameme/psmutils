#include <stdio.h>
#include <string.h>

#include "mc.h"
#include "actions.h"
#include "error.h"

static void print_info_1block(void *mem, int block_n);



/*
 * メモリ情報の表示
 */
void psmem_print(void *base_mem)
{
	int i;

	printf("BLK  DATA_FLAG SIZE   NEXT NUM NAME\n");
	printf("-----------------------------------------------------\n");
	for(i = 1; i < NUM_BLOCK; i++)
		print_info_1block(base_mem, i);
}


/* データフラグフィールドの値 */
#define DATA_FLAG_UNUSED  "UNUSED"
#define DATA_FLAG_HEAD    "HEAD"
#define DATA_FLAG_MIDDLE  "MIDDLE"
#define DATA_FLAG_TAIL    "TAIL"
#define DATA_FLAG_RESERVE "RESERVE"
/* 最大の文字列長 */
#define DATA_FLAG_MAX_LEN 9
#define SIZE_MAX_LEN 6
static void print_info_1block(void *mem, int block_n)
{
	struct Blkinfo bi;
	char *data_flag_str = NULL;
	int flag_other = 0;
	int flag_used = 1;
	int flag_linked = 0; /* 他からリンクされていれば真 */

	mc_read_blockinfo(mem, block_n, &bi);
	switch(bi.data_flag) {
	case DF_UNUSED:
		data_flag_str = DATA_FLAG_UNUSED;
		flag_used = 0;
		break;
	case DF_USED_HEAD:
		data_flag_str = DATA_FLAG_HEAD;
		break;
	case DF_USED_MIDDLE:
		data_flag_str = DATA_FLAG_MIDDLE;
		flag_linked = 1;
		break;
	case DF_USED_TAIL:
		data_flag_str = DATA_FLAG_TAIL;
		flag_linked = 1;
		break;
	case DF_RESERVE:
		data_flag_str = DATA_FLAG_RESERVE;
		break;
	default:
		flag_other = 1;
	}

	/* ブロック番号 */
	printf("%02d   ", block_n);
	/* データフラグ */
	if(flag_other) {
		/*
		 * パディングのサイズ
		 * 最初に表示する文字数文を引く(4)
		 * ただし、空白1つは表示するのでその分パディングは減る(1)
		 */
		printf("0x%02x%-*c", bi.data_flag, DATA_FLAG_MAX_LEN - (4 - 1), ' ');
	}else
		printf("%-*s ", DATA_FLAG_MAX_LEN, data_flag_str);
	/* データサイズ */
	if(flag_linked)
		printf("*%-*c",SIZE_MAX_LEN - (1 - 1), ' ');
	else
		printf("%-*d ", SIZE_MAX_LEN, bi.used_size);
	/* リンク(次のブロック) */
	if(!flag_used)
		printf("-    ");
	else if(bi.next == NEXT_NOT_EXIST)
		printf("*    ");
	else
		printf("%02d   ", bi.next + 1);
	/* 使用ブロック数 */
	if(!flag_used)
		printf("-  ");
	else if(flag_linked)
		printf("*  ");
	else
		printf("%02d ", bi.used_num);
	printf(" ");

	/* データの名前 */
	if(flag_used && !flag_linked)
		printf("\"%.*s\" ", SIZE_MEM_NAME, bi.name);
	else
		printf("-");

	putchar('\n');
}



