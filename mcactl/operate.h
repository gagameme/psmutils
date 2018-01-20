#ifndef OPERATE_H
#define OPERATE_H


enum Action_type {
	ACT_NONE,
	ACT_READ,
	ACT_WRITE,
};


struct Options_ {
	int action;
	char *infname;
	char *outfname;
	int block_index;
	int block_num;
	int l_f;  // -lを指定した場合、真
};


int operate(struct Options_ *opt);


#endif


