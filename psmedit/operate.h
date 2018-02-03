#ifndef OPERATE_H
#define OPERATE_H


enum Action_type {
	ACT_NONE,
	ACT_FORMAT,
	ACT_CREATE,
	ACT_UPLOAD,
	ACT_DOWNLOAD,
	ACT_TITLE,
	ACT_NAME,
	ACT_ERASE,
	ACT_PRINT,
	ACT_SWAP,
	ACT_READ,
	ACT_WRITE,
	ACT_ERROR
};


struct Options_ {
	int action;
	int format_f;  // フォーマットしたデータを使用する場合、真
	char *basefname;
	char *infname;
	char *outfname;
	char *title;
	int block_index;
	int bi_enable_f;  // -bでブロック番号を指定した場合、真
	int dest_index;
};


int operate(struct Options_ *opt);


#endif


