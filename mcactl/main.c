#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "error.h"
#include "operate.h"


static void help(void);
static void get_option(struct Options_ *opt, int argc, char *argv[]);


static char *Usage =
"usage: mcactl (-r|-w) [-h|-i|-o|-b|-l]\n"
"\n"
"MCAとの通信を行う(ps1用メモリーカードのみ)\n"
"指定がなければ、全てのブロックを対象とし(-b0 -l16)、標準入出力を使用する\n"
"*システムによっては管理者権限が必要な場合がある*\n"
"\n"
"optional arguments:\n"
"  -h       show this help message and exit\n"
"  -r       MCAから読み込んだデータを標準出力に出力する\n"
"  -w       標準入力のデータをMCAへ出力する\n"
"  -i file  入力にfileを使用する\n"
"  -o file  出力にfileを使用する\n"
"  -b num   ブロックnum番目を対象とする\n"
"  -l num   ブロックnum個分を対象とする\n"
"\n";



int main(int argc, char *argv[])
{
	struct Options_ opt;

	// オプション解析
	get_option(&opt, argc, argv);

	if(opt.action == ACT_NONE) {
		help();
		exit(0);
	}

	return operate(&opt);
}


static void help()
{
	printf("%s", Usage);
}


/*
 * オプション解析
 *
 * オプションとそれ以外が混ざっていても処理する
 */
static void get_option(struct Options_ *opt, int argc, char *argv[])
{
	// 初期化
	opt->action = ACT_NONE;
	opt->infname = NULL;
	opt->outfname = NULL;
	opt->block_index = 0;
	opt->block_num = 0;
	opt->l_f = 0;

	// 全てのオプションを処理するためのループ
	while(1) {
		int c;

		while((c = getopt(argc, argv, "hrwi:o:b:l:")) != -1) {
			int act = ACT_NONE;
			switch(c) {
			case 'h':
				help();
				exit(0);
			case 'r':
				act = ACT_READ;
				break;
			case 'w':
				act = ACT_WRITE;
				break;
			case 'i':
				opt->infname = optarg;
				break;
			case 'o':
				opt->outfname = optarg;
				break;
			case 'b':
				opt->block_index = atoi(optarg);
				break;
			case 'l':
				opt->l_f = 1;
				opt->block_num = atoi(optarg);
				break;
			default:
				help();
				exit(1);
			}

			// 動作の指定
			if(act != ACT_NONE) {
				/* Actionを複数設定した場合はエラー */
				if(opt->action != ACT_NONE)
					error_exit("to many action");
				opt->action = act;
			}
		}

		// 全ての要素を処理したら終了する
		if(optind >= argc)
			break;

		// オプション以外の処理
		// ("-"はオプション以外として処理する)
		if(!((argv[optind][0] == '-') && (argv[optind][1] != '\0')))
			error_exit("args");

		// 次のオプションをまたgetopt()で処理する
		argv = &argv[optind] - 1;
		argc = argc - optind + 1;
		optind = 1;
	}
}


