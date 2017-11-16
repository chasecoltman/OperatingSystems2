
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>

#include <linux/crypto.h> /* Encryption library */

MODULE_LICENSE("Dual BSD/GPL");


/* GLOBALS */

/* Crypto struct with general naming scheme */
static struct crypto_cipher *tfm;

static int major_num = 0;
module_param(major_num, int, 0);
static int logical_block_size = 512;
module_param(logical_block_size, int, 0);
static int nsectors = 1024; /* How big the drive is */
module_param(nsectors, int, 0);

/* AES encryption key */
static char* key = "SuperSecretPassword";
module_param(key, charp, 0);
/* END GLOBALS */

/*
 * We can tweak our hardware sector size, but the kernel talks to us
 * in terms of small sectors, always.
 */
#define KERNEL_SECTOR_SIZE 512

/*
 * Our request queue.
 */
static struct request_queue *Queue;

static struct ebd_device {
        unsigned long size;
        spinlock_t lock; 
        u8 *data; 
        struct gendisk *gd;
} Device;


static void ebd_transfer(struct ebd_device *dev, sector_t sector,
	unsigned long nsect, char *buffer, int write) {

	unsigned long offset = sector * logical_block_size;
	unsigned long nbytes = nsect * logical_block_size;

	u8 *hex_buf, *hex_disk;
	unsigned int i;
 
	hex_disk = dev->data + offset;
	hex_buf = buffer;

	if ((offset + nbytes) > dev->size) {
		printk (KERN_NOTICE "ebd: Beyond-end write (%ld %ld)\n", offset, nbytes);
		return;
	} if (write) {
		//printk("ebd: Begin write/encryption\n");
		for(i = 0; i < nbytes; i += crypto_cipher_blocksize(tfm)){
			crypto_cipher_encrypt_one(tfm, hex_disk + i, hex_buf + i);
		}

	} else {
		//printk("ebd: Begin read/decryption\n");
		for(i = 0; i < nbytes; i += crypto_cipher_blocksize(tfm)){
			crypto_cipher_decrypt_one(tfm, hex_buf + i,hex_disk + i);
		}
	}
}

static void ebd_request(struct request_queue *q) {
	struct request *req;

	req = blk_fetch_request(q);
	while (req != NULL) {
		//if (!blk_fs_request(req)) {
		if (req == NULL || (req->cmd_type != REQ_TYPE_FS)) {
			printk (KERN_NOTICE "Skip non-CMD request\n");
			__blk_end_request_all(req, -EIO);
			continue;
		}

		ebd_transfer(&Device, blk_rq_pos(req), blk_rq_cur_sectors(req),
			bio_data(req->bio), rq_data_dir(req));

		if ( ! __blk_end_request_cur(req, 0) ) {
			req = blk_fetch_request(q);
		}
	}
}


int ebd_getgeo(struct block_device * block_device, struct hd_geometry * geo) {
	long size;
	size = Device.size * (logical_block_size / KERNEL_SECTOR_SIZE);
	geo->cylinders = (size & ~0x3f) >> 6;
	geo->heads = 4;
	geo->sectors = 16;
	geo->start = 0;
	return 0;
}

static struct block_device_operations ebd_ops = {
		.owner  = THIS_MODULE,
		.getgeo = ebd_getgeo
};

static int __init ebd_init(void) {

	printk("ebd: block device initializing.\n");

	Device.size = nsectors * logical_block_size;
	spin_lock_init(&Device.lock);
	Device.data = vmalloc(Device.size);
	if (Device.data == NULL)
		return -ENOMEM;
	
	Queue = blk_init_queue(ebd_request, &Device.lock);
	if (Queue == NULL)
		goto out;
	blk_queue_logical_block_size(Queue, logical_block_size);
	major_num = register_blkdev(major_num, "ebd");
	if (major_num < 0) {
		printk(KERN_WARNING "ebd: unable to get major number\n");
		goto out;
	}
	tfm = crypto_alloc_cipher("aes",0,0);
	if(!tfm){
		printk(KERN_WARNING "ebd: unable to alloc crypto.\n");
		goto out;
	}

//	printk("ebd: enc_key > %s\n",key);
	crypto_cipher_setkey(tfm,key,strlen(key));
	Device.gd = alloc_disk(16);
	if (!Device.gd)
		goto out_unregister;
	Device.gd->major = major_num;
	Device.gd->first_minor = 0;
	Device.gd->fops = &ebd_ops;
	Device.gd->private_data = &Device;
	strcpy(Device.gd->disk_name, "ebd0");
	set_capacity(Device.gd, nsectors);
	Device.gd->queue = Queue;
	add_disk(Device.gd);
	printk("ebd: block device initialized.\n");
	return 0;

out_unregister:
	unregister_blkdev(major_num, "ebd");
out:
	vfree(Device.data);
	return -ENOMEM;
}

static void __exit ebd_exit(void)
{
	printk("ebd: freeing block device.\n");
	crypto_free_cipher(tfm);

	del_gendisk(Device.gd);
	put_disk(Device.gd);
	unregister_blkdev(major_num, "ebd");
	blk_cleanup_queue(Queue);

	printk("ebd: freed block device.\n");
}

module_init(ebd_init);
module_exit(ebd_exit);
