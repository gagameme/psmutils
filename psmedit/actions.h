#ifndef ACTIONS_H
#define ACTIONS_H


struct Mem_ {
	void *mem;
	int size;
};



void psmem_delete(struct Mem_ *output, void *base_mem, int block_n);

void psmem_mcb_write(struct Mem_ *output, void *base_mem, int block_n, const char *fname);

void psmem_mcb_extract(struct Mem_ *output, void *base_mem, int block_n);

void psmem_newdata(struct Mem_ *output, void *base_mem, int block_n, int len);

void psmem_print(void *base_mem);

void psmem_swap(struct Mem_ *output, void *base_mem, int src_block, int dst_block);

void psmem_title(struct Mem_ *output, void *base_mem, int block_n, char *name);

void psmem_name(void *base_mem, int block_n);

void psmem_raw_read(struct Mem_ *output, void *base_mem, int block_n);

void psmem_raw_write(struct Mem_ *output, void *base_mem, int block_n, const char *fname);


#endif


