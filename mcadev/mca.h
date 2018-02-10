/*
 * Momory Card Adaptor (for PS3) data
 */
#ifndef MCA_PLAYSTATION_H
#define MCA_PLAYSTATION_H

#include "mc.h"


/* USB ID */
#define MCA_VEND_ID 0x054c
#define MCA_PROD_ID 0x02ea

/* Endpoint */
#define MCA_EP_OUTPUT 0x02
#define MCA_EP_INPUT  0x81

/* USB転送時のタイムアウト時間 (s) */
#define MCA_TIMEOUT 3

/* write完了までの待ち時間 (ms) */
#define MCA_WAITTIME_WRITE 50


/* Buffer Size */
/* メモリーカードの種類確認時のバッファサイズ */
#define MCA_LEN_CHECK_MC_BUF 2
/* MCA読み書き時のヘッダサイズ */
#define MCA_LEN_HEAD 4
#define MCA_LEN_HEAD_READ (MCA_LEN_HEAD + MC_LEN_READ_HEAD)
#define MCA_LEN_HEAD_WRITE (MCA_LEN_HEAD + MC_LEN_WRITE_HEAD)
/* MCAでのメモリーカード読み込み時に必要なバッファサイズ */
#define MCA_LEN_READ_BUF (MCA_LEN_HEAD_READ + MC_LEN_SECTOR + MC_LEN_READ_FOOT)
/* MCAでのメモリーカード書き込み時に必要なバッファサイズ */
#define MCA_LEN_WRITE_BUF (MCA_LEN_HEAD_WRITE + MC_LEN_SECTOR + MC_LEN_WRITE_FOOT)
/* 読み書きでの最大バッファサイズ */
#define MCA_LEN_BUF_MAX MCA_LEN_READ_BUF

/* チェックサム格納位置 */
#define MCA_CHECKSUM (MCA_LEN_HEAD_WRITE + MC_LEN_SECTOR)
/* チェックサム計算開始位置 */
#define MCA_START_CHECKSUM 8
/* チェックサム計算終了位置 (ここは含まない) */
#define MCA_END_CHECKSUM (MCA_LEN_HEAD_WRITE + MC_LEN_SECTOR)


/* Check Memory Card  */
/* MCAの応答の先頭バイトの値 */
#define MCA_RCV_HEAD 0x55
/* メモリーカードの種類確認応答 場所 */
#define MCA_RCV_CHECK_POS 1
/* メモリーカードの種類確認応答2byte目 空 */
#define MCA_RCV_CHECK_MC_NONE 0x00
/* メモリーカードの種類確認応答2byte目 PS1 */
#define MCA_RCV_CHECK_MC_PS1 0x01
/* メモリーカードの種類確認応答2byte目 PS2 */
#define MCA_RCV_CHECK_MC_PS2 0x02
/* 読み書き応答2byte目 成功 */
#define MCA_RCV_OK 0x5a

#endif

