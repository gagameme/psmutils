#ifndef MEM_H
#define MEM_H


#define NUM_FRAME 64
#define NUM_BLOCK 16

/* データブロック数 (先頭1ブロックは管理用) */
#define NUM_DATABLOCK (NUM_BLOCK - 1)

/* 1 frame = 128 byte */
#define SIZE_FRAME 128
/* 1 block = 64 frame = 8192 byte */
#define SIZE_BLOCK (SIZE_FRAME * NUM_FRAME)
/* 1 memory = 16 block = 131072 byte */
#define SIZE_MEM (SIZE_BLOCK * NUM_BLOCK)

/* name */
#define NAME_DEFAULT "BISSSSPDEFAULT"
/* memory name length */
#define SIZE_MEM_NAME 20
/* default name */
#define NAME_COUNTRY_CODE_JP "BI"

/* SCxS=Made by Sony, SLxS=Licensed by Sony */
#define NAME_PRODUCT_CODE_JP1 "SCPS"
#define NAME_PRODUCT_CODE_JP2 "SLPS"

#define NAME_SEP_MEM "-"
#define NAME_SEP_PKS "P"

/* country code(2) + product number  (10) + idendifier(8) */
/*                   (code + sep + 6byte)                 */



/* DATA FLAG HIGH */
#define DFH_UNUSED  0xA0
#define DFH_USED    0x50
#define DFH_RESERVE 0xF0
/* DATA FLAG LOW */
#define DFL_UNUSED  0x00
#define DFL_NORMAL  0x01
#define DFL_LINK    0x02
#define DFL_END     0x03
#define DFL_RESERVE 0x0F
/* DATA FLAG */
#define DF_UNUSED      (DFH_UNUSED | DFL_UNUSED)
#define DF_USED_HEAD   (DFH_USED | DFL_NORMAL)
#define DF_USED_MIDDLE (DFH_USED | DFL_LINK)
#define DF_USED_TAIL   (DFH_USED | DFL_END)
#define DF_RESERVE     (DFH_RESERVE | DFL_RESERVE)

/* next value */
#define NEXT_NOT_EXIST 0xff
#define NEXT_HIGH 0x00
#define NEXT_HIGH_NOT_EXIST 0xff


/* macro */
/* block番目のブロックの先頭オフセットを計算する */
#define OFFSET_BLOCK(block) ((block) * SIZE_BLOCK)
/* あるブロックのframe番目のフレームの先頭オフセットを計算する */
#define OFFSET_FRAME(frame) ((frame) * SIZE_FRAME)
/* sizeバイトのデータを格納するために必要なブロック数を計算する */
#define REQUIRED_BLOCK(size) ((size) / SIZE_BLOCK + (((size) % SIZE_BLOCK)?1:0))


/* block header info */
struct Blkinfo {
	char *name;
	int data_flag;
	int used_size;
	int next;
	int used_num;
};



void mc_format(void *mem);
void mc_format_block0(void *mem);
void mc_format_datablock(void *mem, int block_n);

int mc_read_basefile(const char *basefname, void *mem);

void mc_read_blockinfo(const void *mem, int block_n, struct Blkinfo *bi);
void mc_write_blockinfo(void *mem, int block_n, const struct Blkinfo *bi);
void mc_init_blockinfo(struct Blkinfo *bi);

void mc_write_infoframe(void *mem, int block_n, int df, int size, int next, const char *name);

void mc_read_data(const void *mem, int block_n, void *data, int size);
void mc_write_data(void *mem, int block_n, const void *data, int size);


/* debug */
void mc_print_frame(void *m_frame);



#endif


