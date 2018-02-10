/*
 * mcaps1m
 * MCA PS1 memory card
 *
 * TODO 刺さっているカードがPS1用か確認していない
 */
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/mutex.h>

#include "mc.h"
#include "mca.h"

#define DRIVER_NAME "mcaps1m"
#define DRIVER_DESC "device driver for MCA(PS1 memory card only)"


#define MINOR_COUNT 1

static const struct usb_device_id mca_table[] = {
	{ USB_DEVICE(MCA_VEND_ID, MCA_PROD_ID) },
	{ }
};
MODULE_DEVICE_TABLE(usb, mca_table);

struct usb_mca {
	struct usb_device *udev;
	struct usb_interface *interface;
	struct kref kref;
	struct mutex io_mutex;  /* USB転送中のクローズ防止用 */

	char mca_sbuf[MCA_LEN_BUF_MAX];  /* MCA送信用バッファ */
	char mca_rbuf[MCA_LEN_BUF_MAX];  /* MCA受信用バッファ */
};

static struct usb_driver mca_driver;

/* filp->f_pos から セクター番号取得 */
#define mca_get_sector_number(f_pos) (((f_pos) >> 7) & 0x3FF)
/* filp->f_pos から セクターのオフセット取得 */
#define mca_get_sector_offset(f_pos) ((f_pos) & 0x7F)

/* USBへ書き込むデータの格納位置を返す */
#define mca_get_sectorp_write(dev) ((dev)->mca_sbuf + MCA_LEN_HEAD_WRITE)
/* USBから読み込んだデータの格納位置を返す */
#define mca_get_sectorp_read(dev) ((dev)->mca_rbuf + MCA_LEN_HEAD_READ)


static void mca_delete(struct kref *kref)
{
	struct usb_mca *dev = container_of(kref, struct usb_mca, kref);

	usb_put_dev(dev->udev);
	kfree(dev);
}

static int mca_open(struct inode *inode, struct file *filp)
{
	struct usb_mca *dev;
	struct usb_interface *interface;
	int subminor;

	subminor = iminor(inode);

	interface = usb_find_interface(&mca_driver, subminor);
	if (!interface) {
		pr_err("%s - error, can't find device for minor %d\n",
			__func__, subminor);
		return -ENODEV;
	}

	dev = usb_get_intfdata(interface);
	if (!dev)
		return -ENODEV;

	kref_get(&dev->kref);

	filp->private_data = dev;

	return 0;
}

static int mca_release(struct inode *inode, struct file *filp)
{
	struct usb_mca *dev;

	dev = filp->private_data;
	if (!dev)
		return -ENODEV;

	kref_put(&dev->kref, mca_delete);

	return 0;
}

/*
 * MCAへlenバイトdev->mca_sbufの内容を送信する
 * MCAからlenバイトdev->mca_rbufへ受信する
 *
 * dev       デバイス情報
 * len       転送する長さ
 * waittime  送信と受信の間に待つ時間 (ms)
 *
 * return
 *  0         成功
 *  (-errno)  失敗
 */
static int mca_transfer(struct usb_mca *dev, int len, int waittime)
{
	int retval;
	int count = 0;

	/* 命令を送信 */
	retval = usb_bulk_msg(dev->udev,
			usb_sndbulkpipe(dev->udev, MCA_EP_OUTPUT),
			dev->mca_sbuf,
			len,
			&count,
			HZ * MCA_TIMEOUT);
	if (retval < 0)
		return retval;

	if (waittime > 0)
		msleep(waittime);

	/* 結果を受信 */
	retval = usb_bulk_msg(dev->udev,
			usb_rcvbulkpipe(dev->udev, MCA_EP_INPUT),
			dev->mca_rbuf,
			len,
			&count,
			HZ * MCA_TIMEOUT);
	if (retval < 0)
		return retval;

	/* 結果の確認 */
	if ((dev->mca_rbuf[0] != MCA_RCV_HEAD) ||
			(dev->mca_rbuf[1] != MCA_RCV_OK))
		return -EIO;

	return 0;
}

/*
 * read memory card (1 sector) via MCA
 *
 * dev  デバイス情報
 * sec  読み込みセクタ番号 範囲チェックをしない
 *
 * return
 *  0         成功
 *  (-errno)  失敗
 */
static int mca_read_sector(struct usb_mca *dev, size_t sec)
{
	int retval;
	const int len = MCA_LEN_READ_BUF;

	memset(dev->mca_sbuf, 0, len);
	memset(dev->mca_rbuf, 0, len);

	/* 読み込み命令作成 */
	dev->mca_sbuf[0] = 0xaa;
	dev->mca_sbuf[1] = 0x42;
	dev->mca_sbuf[2] = MC_LEN_READ_HEAD + MC_LEN_SECTOR + MC_LEN_READ_FOOT;
	dev->mca_sbuf[3] = 0x00;
	dev->mca_sbuf[4] = 0x81;
	dev->mca_sbuf[5] = 0x52;

	dev->mca_sbuf[8] = sec >> 8;
	dev->mca_sbuf[9] = sec & 0xff;

	/* USB transfer */
	retval = mca_transfer(dev, len, 0);

	return retval;
}

static ssize_t mca_read(struct file *filp, char __user *buffer,
		size_t count, loff_t *f_pos)
{
	struct usb_mca *dev;
	int retval = 0;
	size_t sec_num;
	size_t sec_off;
	size_t rest_size;
	char *datap;
	size_t read_size;

	dev = filp->private_data;

	if (mutex_lock_interruptible(&dev->io_mutex))
		return -ERESTARTSYS;

	if (*f_pos >= MC_SIZE)
		goto exit;

	if (!dev->interface) {
		/* disconnect() was called */
		retval = -ENODEV;
		goto exit;
	}

	sec_num = mca_get_sector_number(*f_pos);
	sec_off = mca_get_sector_offset(*f_pos);

	/* セクタの境界内となるようなサイズにする */
	rest_size = MC_LEN_SECTOR - sec_off;
	read_size = min(count, rest_size);

	/* USB transfer */
	retval = mca_read_sector(dev, sec_num);
	if (retval)
		goto exit;

	datap = mca_get_sectorp_read(dev);
	if (copy_to_user(buffer, datap + sec_off, read_size)) {
		retval = -EFAULT;
		goto exit;
	}

	*f_pos += read_size;
	retval = read_size;

exit:
	mutex_unlock(&dev->io_mutex);

	return retval;
}

/*
 * write memory card (1 sector) via MCA
 *
 * dev  デバイス情報
 * sec  書き込みセクタ番号 範囲チェックをしない
 *
 * return
 *  0         成功
 *  (-errno)  失敗
 */
static int mca_write_sector(struct usb_mca *dev, size_t sec)
{
	int i;
	int retval;
	const int len = MCA_LEN_WRITE_BUF;
	const int len_pre_sec = MCA_LEN_HEAD_WRITE;
	const int len_post_sec = MCA_LEN_HEAD_WRITE + MC_LEN_SECTOR;

	/*
	 * dev->mca_sbufは書き込むデータが
	 * 格納されている位置は0クリアしない
	 */
	memset(dev->mca_sbuf, 0, len_pre_sec);
	memset(dev->mca_sbuf + len_post_sec, 0, len - (len_post_sec));
	memset(dev->mca_rbuf, 0, len);

	/* 書き込み命令作成 */
	dev->mca_sbuf[0] = 0xaa;
	dev->mca_sbuf[1] = 0x42;
	dev->mca_sbuf[2] =
		MC_LEN_WRITE_HEAD + MC_LEN_SECTOR + MC_LEN_WRITE_FOOT;
	dev->mca_sbuf[3] = 0x00;
	dev->mca_sbuf[4] = 0x81;
	dev->mca_sbuf[5] = 0x57;

	dev->mca_sbuf[8] = sec >> 8;
	dev->mca_sbuf[9] = sec & 0xff;

	/* checksum */
	dev->mca_sbuf[MCA_CHECKSUM] = 0;
	for (i = MCA_START_CHECKSUM; i < MCA_END_CHECKSUM; i++)
		dev->mca_sbuf[MCA_CHECKSUM] ^= dev->mca_sbuf[i];

	/* USB transfer */
	retval = mca_transfer(dev, len, MCA_WAITTIME_WRITE);

	return retval;
}

static ssize_t mca_write(struct file *filp, const char __user *buffer,
		size_t count, loff_t *f_pos)
{
	struct usb_mca *dev;
	int retval = 0;
	size_t sec_num;
	size_t sec_off;
	size_t rest_size;
	char *datap;
	size_t write_size;

	dev = filp->private_data;

	if (mutex_lock_interruptible(&dev->io_mutex))
		return -ERESTARTSYS;

	if (*f_pos >= MC_SIZE) {
		retval = -ENOSPC;
		goto exit;
	}

	if (!dev->interface) {
		/* disconnect() was called */
		retval = -ENODEV;
		goto exit;
	}

	sec_num = mca_get_sector_number(*f_pos);
	sec_off = mca_get_sector_offset(*f_pos);

	/* セクタの境界内となるようなサイズにする */
	rest_size = MC_LEN_SECTOR - sec_off;
	write_size = min(count, rest_size);

	/* USB transfer */

	datap = mca_get_sectorp_write(dev);

	/*
	 * セクタ全体の書き込みではない場合、
	 * 一度セクタのデータを取得する
	 */
	if (write_size < MC_LEN_SECTOR) {
		char *rdatap = NULL;

		retval = mca_read_sector(dev, sec_num);
		if (retval)
			goto exit;
		rdatap = mca_get_sectorp_read(dev);
		memcpy(datap, rdatap, MC_LEN_SECTOR);
	}

	if (copy_from_user(datap + sec_off, buffer, write_size)) {
		retval =  -EFAULT;
		goto exit;
	}

	retval = mca_write_sector(dev, sec_num);
	if (retval)
		goto exit;

	*f_pos += write_size;
	retval = write_size;

exit:
	mutex_unlock(&dev->io_mutex);

	return retval;
}

static loff_t mca_llseek(struct file *filp, loff_t off, int whence)
{
	struct usb_mca *dev;
	loff_t newpos = -EINVAL;

	dev = filp->private_data;

	if (mutex_lock_interruptible(&dev->io_mutex))
		return -ERESTARTSYS;

	switch (whence) {
	case SEEK_SET:
		newpos = off;
		break;
	case SEEK_CUR:
		newpos = filp->f_pos + off;
		break;
	case SEEK_END:
		newpos = MC_SIZE + off;
		break;
	default:
		/* nop */
		break;
	}

	if (newpos < 0)
		newpos = -EINVAL;
	else
		filp->f_pos = newpos;

	mutex_unlock(&dev->io_mutex);

	return newpos;
}

static const struct file_operations mca_fops = {
	.owner = THIS_MODULE,
	.read = mca_read,
	.write = mca_write,
	.open = mca_open,
	.release = mca_release,
	.llseek = mca_llseek,
};

/*
 * デバイスファイルのパーミッションを0666にする
 */
static char *handle_devnode(struct device *dev, umode_t *mode)
{
	if (mode)
		*mode = 0666;
	return NULL;
}

static struct usb_class_driver mca_class = {
	.name = DRIVER_NAME,
	.devnode = handle_devnode,
	.fops = &mca_fops,
	.minor_base = MINOR_COUNT,
};

static int mca_probe(struct usb_interface *interface,
		const struct usb_device_id *id)
{
	struct usb_mca *dev;
	int retval = -ENOMEM;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		goto error;
	kref_init(&dev->kref);
	mutex_init(&dev->io_mutex);

	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

	usb_set_intfdata(interface, dev);

	retval = usb_register_dev(interface, &mca_class);
	if (retval) {
		dev_err(&interface->dev,
			"Not able to get a minor for this device.\n");
		usb_set_intfdata(interface, NULL);
		goto error;
	}

	dev_info(&interface->dev, "%s device now attached", DRIVER_NAME);
	return 0;

error:
	if (dev)
		kref_put(&dev->kref, mca_delete);
	return retval;
}

static void mca_disconnect(struct usb_interface *interface)
{
	struct usb_mca *dev;

	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	usb_deregister_dev(interface, &mca_class);

	mutex_lock(&dev->io_mutex);
	dev->interface = NULL;
	mutex_unlock(&dev->io_mutex);

	kref_put(&dev->kref, mca_delete);

	dev_info(&interface->dev, "%s now disconnected", DRIVER_NAME);
}

static struct usb_driver mca_driver = {
	.name = DRIVER_NAME,
	.probe = mca_probe,
	.disconnect = mca_disconnect,
	.id_table = mca_table,
};

module_usb_driver(mca_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);

