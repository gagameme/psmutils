/*
 * Momory Card (PS1) data
 */
#ifndef MC_PS1_H
#define MC_PS1_H

#define MC_NUM_SECTORS_OF_BLOCK 64  /* 1block中のセクタ数 */
#define MC_NUM_BLOCKS_OF_MC 16  /* 1メモリーカード中のブロック数 */

/* セクターの最大数 */
#define MC_MAX_NUM_SECTORS (MC_NUM_SECTORS_OF_BLOCK * MC_NUM_BLOCKS_OF_MC)

/* セクタサイズ */
#define MC_LEN_SECTOR 128

/* ps1メモリーカードのバイト数 */
#define MC_SIZE (MC_LEN_SECTOR * MC_MAX_NUM_SECTORS)

/* BUFFER SIZE */
/* メモリーカード読み込み時のヘッダ、フッタサイズ */
#define MC_LEN_READ_HEAD 10
#define MC_LEN_READ_FOOT 2
/* メモリーカード書き込み時のヘッダ、フッタサイズ */
#define MC_LEN_WRITE_HEAD 6
#define MC_LEN_WRITE_FOOT 4

#endif

