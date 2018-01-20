#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <libusb-1.0/libusb.h>

#include "mcaio.h"


struct McaDev {
	libusb_context *ctx;
	libusb_device_handle *dh;
};


/*
 * MCA通信のための初期化
 */
int mca_open(McaDev **mdp)
{
	int r;
	McaDev *md;

	if(mdp == NULL)
		return MCA_ERR_ARGS;

	if((*mdp = malloc(sizeof(McaDev))) == NULL)
		return MCA_ERR_MALLOC;

	md = *mdp;

	r = libusb_init(&md->ctx);
	if(r < 0) {
		// Error: init usb
		free(md);
		return MCA_ERR_INIT_USB;
	}

	//libusb_set_debug(ctx, 4);

	md->dh = libusb_open_device_with_vid_pid(md->ctx, MCA_VEND_ID, MCA_PROD_ID);
	if(md->dh == NULL) {
		// Error: open device
		libusb_exit(md->ctx);
		free(md);
		return MCA_ERR_OPEN_DEV;
	}

	if(libusb_kernel_driver_active(md->dh, 0) == 1) {
		// kernel driver active
		libusb_detach_kernel_driver(md->dh, 0);
	}

	r = libusb_claim_interface(md->dh, 0);
	if(r < 0) {
		// Error: claim interface
		mca_close(md);
		return MCA_ERR_CLAIM_IF;
	}

	return MCA_SUCCESS;
}


/*
 * MCA通信の終了処理
 */
int mca_close(McaDev *md)
{

	if(md == NULL)
		return MCA_SUCCESS;

	libusb_release_interface(md->dh, 0);
	libusb_close(md->dh);
	libusb_exit(md->ctx);

	free(md);

	return MCA_SUCCESS;
}


/*
 * MCAにPS1のメモリーカードがある場合、ps1_fに非0をセットする
 */
int mca_ps1_is(McaDev *md, int *ps1_f)
{
	int r;
	int t;
	unsigned char sbuf[2] = {0xaa, 0x40};
	unsigned char rbuf[2] = {0};

	if(ps1_f == NULL)
		return MCA_ERR_ARGS;

	// 命令を送信
	r = libusb_bulk_transfer(md->dh, MCA_EP_OUTPUT, sbuf, sizeof(sbuf), &t, MCA_TIMEOUT);
	if(r < 0)
		return MCA_ERR_EP_O;

	// 受信
	r = libusb_bulk_transfer(md->dh, MCA_EP_INPUT, rbuf, sizeof(rbuf), &t, MCA_TIMEOUT);
	if(r < 0)
		return MCA_ERR_EP_I;

	if((rbuf[0] == 0x55) && (rbuf[1] == 0x01))
		*ps1_f = 1;
	else
		*ps1_f = 0;

	return MCA_SUCCESS;
}


/*
 * 1セクタ(128Byte)分読み込む
 * bufはMC_SECTOR_SIZE以上あるとみなす
 *
 * 成功時0を返す
 * bufの値、posの範囲を検査しない
 */
int mca_ps1_read(McaDev *md, unsigned char *buf, int pos)
{
	int r;
	int t;
	unsigned char sbuf[MCA_READ_BUF_LEN] =
		{0xaa, 0x42, MC_READ_HEAD_LEN + MC_SECTOR_SIZE, 0x00, 0x81, 0x52};
	unsigned char rbuf[MCA_READ_BUF_LEN] = {0};

	// 読み込み位置の指定
	sbuf[8] = pos >> 8;
	sbuf[9] = pos & 0xff;

	// 読み込み命令を送信
	r = libusb_bulk_transfer(md->dh, MCA_EP_OUTPUT, sbuf, sizeof(sbuf), &t, MCA_TIMEOUT);
	if(r < 0)
		return MCA_ERR_EP_O;

	// 受信
	r = libusb_bulk_transfer(md->dh, MCA_EP_INPUT, rbuf, sizeof(rbuf), &t, MCA_TIMEOUT);
	if((r < 0) || (rbuf[0] != 0x55) || (rbuf[1] != 0x5a))
		return MCA_ERR_EP_I;

	memcpy(buf, &rbuf[14], MC_SECTOR_SIZE);

	return MCA_SUCCESS;
}


/*
 * 1セクタ(128Byte)分書き込む
 * bufはMC_SECTOR_SIZE以上あるとみなす
 *
 * 成功時0を返す
 * bufの値、posの範囲を検査しない
 */
int mca_ps1_write(McaDev *md, unsigned char *buf, int pos)
{
	int i;
	int r;
	int t = 0;
	unsigned char sbuf[MCA_WRITE_BUF_LEN] =
		{0xaa, 0x42, MC_WRITE_HEAD_LEN + MC_SECTOR_SIZE, 0x00, 0x81, 0x57};
	unsigned char rbuf[MCA_WRITE_BUF_LEN] = {0};
	// 書き込み待ち時間 50ms
	struct timespec wait50ms = {
		.tv_sec = 0,
		.tv_nsec = 50 * 1000000,
	};

	// 書き込み位置の指定
	sbuf[8] = pos >> 8;
	sbuf[9] = pos & 0xff;

	// 書き込むデータを設定
	memcpy(&sbuf[10], buf, MC_SECTOR_SIZE);

	// チェックサム計算
	sbuf[10 + MC_SECTOR_SIZE] = 0;
	for(i = 8; i < 10 + MC_SECTOR_SIZE; i++)
		sbuf[10 + MC_SECTOR_SIZE] ^= sbuf[i];

	// 書き込み命令を送信
	r = libusb_bulk_transfer(md->dh, MCA_EP_OUTPUT, sbuf, sizeof(sbuf), &t, MCA_TIMEOUT);
	if(r < 0)
		return MCA_ERR_EP_O;

	// 書き込み待ち
	nanosleep(&wait50ms, NULL);

	// 受信
	r = libusb_bulk_transfer(md->dh, MCA_EP_INPUT, rbuf, sizeof(rbuf), &t, MCA_TIMEOUT);
	if((r < 0) || (rbuf[0] != 0x55) || (rbuf[1] != 0x5a))
		return MCA_ERR_EP_I;

	return MCA_SUCCESS;
}



