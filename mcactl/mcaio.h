#ifndef MCARW_H
#define MCARW_H

// USB (MCA)
#define MCA_VEND_ID 0x054c
#define MCA_PROD_ID 0x02ea

#define MCA_EP_OUTPUT 0x02
#define MCA_EP_INPUT  0x81

#define MCA_TIMEOUT 3000  // ms

#define MCA_SUCCESS 0  // 正常終了
#define MCA_ERR_INIT_USB (128)  // usb初期化エラー
#define MCA_ERR_OPEN_DEV (129)  // MCAデバイスのオープンエラー
#define MCA_ERR_CLAIM_IF (130)  // インターフェースの設定エラー
#define MCA_ERR_EP_O (131)  // 出力用のエンドポイントのエラー
#define MCA_ERR_EP_I (132)  // 入力用のエンドポイントのエラー
#define MCA_ERR_ARGS (133)  // 引数エラー
#define MCA_ERR_MALLOC (134)  // malloc()エラー


// Memory Card
#define MC_SECTOR_SIZE 128  // Byte size of 1 sector
#define MC_SECTORS_PER_BLOCK 64  // sector number of 1 block
#define MC_BLOCKS_PER_MC 16  // block number of 1 MC
#define MC_SECTORS_PER_MC (MC_SECTORS_PER_BLOCK * MC_BLOCKS_PER_MC)  // sector number of 1 MC

// READ WRITE BUFFER
#define MC_READ_HEAD_LEN 12  // メモリーカード読み込み時のヘッダサイズ
#define MC_WRITE_HEAD_LEN 10  // メモリーカード書き込み時のヘッダサイズ

#define MCA_HEAD_LEN 4  // MCA使用時のヘッダサイズ
// MCAでのメモリーカード読み込み時に必要なバッファサイズ
#define MCA_READ_BUF_LEN (MCA_HEAD_LEN + MC_READ_HEAD_LEN + MC_SECTOR_SIZE)
// MCAでのメモリーカード書き込み時に必要なバッファサイズ
#define MCA_WRITE_BUF_LEN (MCA_HEAD_LEN + MC_WRITE_HEAD_LEN + MC_SECTOR_SIZE)



typedef struct McaDev McaDev;

int mca_open(McaDev **md);
int mca_close(McaDev *md);
int mca_ps1_is(McaDev *md, int *ps1_f);
int mca_ps1_read(McaDev *md, unsigned char *buf, int pos);
int mca_ps1_write(McaDev *md, unsigned char *buf, int pos);


#endif

