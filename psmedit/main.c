#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "operate.h"
#include "error.h"


static void get_option(struct Options_ *opt, int argc, char *argv[]);
static void help();


/*
 * @のオプションは未実装
 */
static char *Usage =
"Usage psmedit [<options>] [<file>]\n"
"\n"
" PS1用メモリーカードのデータの編集を行う\n"
" データブロックの変更をした場合は\n"
" 管理ブロック(ヘッダ)をできる限り適切に変更する\n"
"\n"
" 大文字のオプションでコマンドの動作を指定する\n"
" 動作は複数指定するとエラー\n"
"\n"
" file      データ生成時の元となるファイル(128kBであること)\n"
"           指定無しの場合、標準入力を使用する\n"
"\n"
" Options:\n"
"  -h       Display this informaion\n"
"  -o outf  出力ファイルを指定する\n"
"           指定がない場合は標準出力を使用する\n"
"  -b num   ブロック1以外を操作する際のブロック番号を指定する\n"
"  -f       初期化したデータを操作する\n"
"           fileは指定していても無視する\n"
"\n"
"  -I       メモリの情報を表示する\n"
"  -N num   ブロック1からnum個のブロックを0埋めし、使用済みにする\n"
"           num==0の場合は何も変更しない\n"
"  -W mcb   ブロック1にmcbファイルの内容を書き込む\n"
"           続きがあれば次のブロックへも書き込む\n"
"  -S num   ブロック1とブロックnumを入れ替える\n"
"  -T title ブロック1のデータ名を変更する\n"
"  -D       ブロック1を削除する(ヘッダのみ)\n"
"           他ブロックに続きがあれば削除する\n"
"  -E       ブロック1の内容をmcb形式で出力する\n"
"           他ブロックに続きがあれば取り出す\n"
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

	operate(&opt);

	return 0;
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
	opt->format_f = 0;
	opt->basefname = NULL;
	opt->infname = NULL;
	opt->outfname = NULL;
	opt->title = NULL;
	opt->block_index = 1;
	opt->bi_enable_f = 0;
	opt->dest_index = 1;

	// 全てのオプションを処理するためのループ
	while(1) {
		int c;

		while((c = getopt(argc, argv, "ho:b:fIN:W:S:T:DE")) != -1) {
			int act = ACT_NONE;
			switch(c) {
			case 'h':
				help();
				exit(0);
			case 'o':
				opt->outfname = optarg;
				break;
			case 'b':
				opt->block_index = atoi(optarg);
				opt->bi_enable_f = 1;
				break;
			case 'f':
				opt->format_f = 1;
				break;
			case 'I':
				act = ACT_PRINT;
				break;
			case 'N':
				act = ACT_NEWDATA;
				opt->dest_index = atoi(optarg);
				break;
			case 'W':
				act = ACT_WRITE;
				opt->infname = optarg;
				break;
			case 'T':
				act = ACT_TITLE;
				opt->title = optarg;
				break;
			case 'S':
				act = ACT_SWAP;
				opt->dest_index = atoi(optarg);
				break;
			case 'D':
				act = ACT_DELETE;
				break;
			case 'E':
				act = ACT_EXTRACT;
				break;
			default:
				help();
				exit(1);
			}

			// 動作の指定
			if(act != ACT_NONE) {
				/* Actionを複数設定した場合はエラー */
				if(opt->action != ACT_NONE)
					error_exit("too many action");
				opt->action = act;
			}
		}

		// 全ての要素を処理したら終了する
		if(optind >= argc)
			break;

		// オプション以外の処理
		// ("-"はオプション以外として処理する)
		if(!((argv[optind][0] == '-') && (argv[optind][1] != '\0'))) {
			if(opt->basefname != NULL)
				error_exit("too many base file");

			opt->basefname = argv[optind];
			optind++;
		}

		// 次のオプションをまたgetopt()で処理する
		argv = &argv[optind] - 1;
		argc = argc - optind + 1;
		optind = 1;
	}
}


/*
 * ヘルプ出力
 */
static void help()
{
	printf("%s", Usage);
}


