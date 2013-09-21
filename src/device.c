/*
 * device.c
 *
 *  Created on: 21 sept. 2013
 *      Author: pierre
 */

#include "kernelincludes.h"
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/spinlock.h>

#include <api.h>

/*
 * Static vars
 */
spinlock_t io_lock;
char *InputBuffer;
char *OutputBuffer;
int OutputBufferlenght;

typedef enum _COMP_DECOMP
{
	COMP_DECOMP_COMPRESS = 0,
	COMP_DECOMP_DECOMPRESS
} COMP_DECOMP;

COMP_DECOMP Compress;

const char * AlgoList[]=
{
		"sharc",
		"lzo",
		"lz4",
};

unsigned int Algo;
unsigned int SharcCompression;
unsigned int SharcoutputType;
unsigned int SharcblockType;

struct crypto_comp *tfm;
/*
 * Parameters
 */
unsigned int AlgoParameter = 3;
module_param(AlgoParameter, int, 0644);
MODULE_PARM_DESC(AlgoParameter,"\n"
		"sharc = 0\n"
		"lzo = 1\n"
		"lz4 = 2\n"
		"copy = 3\n");

unsigned int SharcCompressionParameter = 1;
module_param(SharcCompressionParameter, int, 0644);
MODULE_PARM_DESC(SharcCompressionParameter,"\n"
		"SHARC_COMPRESSION_MODE_COPY = 0\n"
	    "SHARC_COMPRESSION_MODE_FASTEST = 1\n"
	    "SHARC_COMPRESSION_MODE_DUAL_PASS = 2\n");

unsigned int SharcoutputTypeParameter = 0;
module_param(SharcoutputTypeParameter, int, 0644);
MODULE_PARM_DESC(SharcoutputTypeParameter,"\n"
	"SHARC_ENCODE_OUTPUT_TYPE_DEFAULT = 0\n"
    "SHARC_ENCODE_OUTPUT_TYPE_WITHOUT_HEADER = 1\n"
    "SHARC_ENCODE_OUTPUT_TYPE_WITHOUT_FOOTER = 2\n"
    "SHARC_ENCODE_OUTPUT_TYPE_WITHOUT_HEADER_NOR_FOOTER = 3\n");

unsigned int SharcblockTypeParameter = 0;
module_param(SharcblockTypeParameter, int, 0644);
MODULE_PARM_DESC(SharcblockType,"\n"
	"SHARC_BLOCK_TYPE_DEFAULT = 0\n"
    "SHARC_BLOCK_TYPE_NO_HASHSUM_INTEGRITY_CHECK = 1\n");

/*
 * helper functions
 */

void reset(void)
{
	if (InputBuffer)
		vfree(InputBuffer);
	if (OutputBuffer)
		vfree(OutputBuffer);
	Compress = COMP_DECOMP_COMPRESS;
	OutputBufferlenght = 0;


	Algo             = AlgoParameter;
	SharcCompression = SharcCompressionParameter;
	SharcoutputType  = SharcoutputTypeParameter;
	SharcblockType   = SharcblockTypeParameter;


}

/*
 * Device operations
 */
static int device_open(struct inode *node, struct file *fichier);
static int device_release(struct inode *node, struct file *fichier);
static ssize_t device_read(struct file *filep, char *buffer, size_t length, loff_t *offset);
static ssize_t device_write(struct file *filep, const char *buffer, size_t length, loff_t *offset);
static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg_);

struct file_operations Comp_fops	 = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = device_ioctl
};

int device_open(struct inode *node, struct file *fichier)
{
	printk("Open\n");
	spin_lock(&io_lock);
	spin_unlock(&io_lock);
	return 0;
}
int device_release(struct inode *node, struct file *fichier)
{
	printk("close\n");
	spin_lock(&io_lock);
	spin_unlock(&io_lock);
	return 0;
}
ssize_t device_read(struct file *filep, char *buffer, size_t length, loff_t *offset)
{
	spin_lock(&io_lock);

	/* Copy kernel->user */
	copy_to_user(buffer,OutputBuffer,length < OutputBufferlenght ? length: OutputBufferlenght);

	spin_unlock(&io_lock);
	return 0;
}

ssize_t device_write(struct file *filep, const char *buffer, size_t length, loff_t *offset)
{
	int Retval = 0;

	spin_lock(&io_lock);

	if (InputBuffer)
		vfree(InputBuffer);
	if (OutputBuffer)
		vfree(OutputBuffer);

	OutputBufferlenght = 0;

	/* Alloc input */
	InputBuffer = (char *)vmalloc(length);

	if (InputBuffer)
	{
		/* Alloc output*/
		OutputBuffer = (char *)vmalloc(length);

		if (OutputBuffer) {
			/* Copy user->kernel */
			copy_from_user(InputBuffer,buffer,length);
			/* Compression/décompression */
			if (Algo == 3)
			{
				memcpy(OutputBuffer,InputBuffer,length);
			}
			else
			{
				switch (Compress) {
				case COMP_DECOMP_COMPRESS:
					Retval = crypto_comp_compress(tfm, InputBuffer, length, OutputBuffer, &OutputBufferlenght);
					break;
				case COMP_DECOMP_DECOMPRESS:
					Retval = crypto_comp_decompress(tfm, InputBuffer, length, OutputBuffer, &OutputBufferlenght);
					break;
				default:
					Retval = -EINVAL;
				}

			}
		}
	}
	spin_unlock(&io_lock);
	return Retval;
}
typedef enum _ENUM_SHARC_IOCTL
{
	IOCTL_RESET = 0,
	IOCTL_COMP_DECOMP,
	IOCTL_ALGO,
	IOCTL_SHARC_COMP_PARAM,
	IOCTL_SHARC_OUTPUT_TYPE,
	IOCTL_SHARC_BLOCK_TYPE
} ENUM_SHARC_IOCTL;

static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg_)
{
	int retval = 0;

	spin_lock(&io_lock);
	switch (cmd)
	{
	case IOCTL_RESET :
		reset();
		break;
	case IOCTL_COMP_DECOMP:
		if (arg_ < 2)
			Compress = arg_;
		else
			retval = -EINVAL;
		break;
	case IOCTL_ALGO :
		if ( arg_ < 4)
		{
			Algo = arg_;
			reset();

			crypto_free_tfm(crypto_comp_tfm(tfm));
			/* Activate new algo */
			tfm = crypto_alloc_comp(AlgoList[Algo], 0, CRYPTO_ALG_ASYNC);
		}
		else
			retval = -EINVAL;
		break;
/* Sharc related ioctl */
	case IOCTL_SHARC_COMP_PARAM  :
		if ( arg_ < 3)
			SharcCompression = arg_;
		else
			retval = -EINVAL;
		break;
	case IOCTL_SHARC_OUTPUT_TYPE :
		if ( arg_ < 4)
			SharcoutputType = arg_;
		else
			retval = -EINVAL;
		break;
	case IOCTL_SHARC_BLOCK_TYPE  :
		if ( arg_ < 2)
			SharcblockType = arg_;
		else
			retval = -EINVAL;
		break;
	default:
		retval = -EINVAL;
	}
	spin_unlock(&io_lock);
	return retval;
}
/*
 * Module operations
 */

#define DEVICE_MAJOR 42
#define DRIVER_NAME "compress1"

static dev_t CompDev = MKDEV(DEVICE_MAJOR,0);

static int __init monmodule_init(void)
{
	int RetVal=-1;


	pr_info("DeviceName_pc : %s\n",DRIVER_NAME);

	RetVal = register_chrdev(CompDev,DRIVER_NAME, &Comp_fops);
	if (RetVal >=0)
	{
		reset();

		if (Algo <3)
		{
			if (!crypto_has_comp(AlgoList[Algo], 0, 0)) {
				pr_info("%s compressor not available\n", AlgoList[Algo]);
				RetVal =-ENODEV;
			}

			tfm = crypto_alloc_comp(AlgoList[Algo], 0, CRYPTO_ALG_ASYNC);
		}

		spin_lock_init(&io_lock);
		RetVal = 0;
	}

	if (RetVal <0)
		pr_info("Erreur au chargement\n");

	return RetVal;
}

static void __exit monmodule_exit(void)
{
	spin_lock(&io_lock);

	crypto_free_tfm(crypto_comp_tfm(tfm));

	if (InputBuffer)
		vfree(InputBuffer);
	if (OutputBuffer)
		vfree(OutputBuffer);

	unregister_chrdev(DEVICE_MAJOR, DRIVER_NAME);
	pr_info("Unregistered\n");

	spin_unlock(&io_lock);
}

module_init(monmodule_init);
module_exit(monmodule_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pierre Mazein");	/* Who wrote this module? */
MODULE_DESCRIPTION("Device to test kernel compression from user space");
