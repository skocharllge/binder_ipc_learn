#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/circ_buf.h>
#include <linux/sched.h>
#include <linux/dma-mapping.h>

#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fcntl.h>

/* CONSTANTS */
#define BUF_LENGTH  1024
#define NAME        "binder_test_ipc"
#define ERROR       -1
#define CIR_BUF_CNT  5

/******************************************************************************
 						Declaration of functions
******************************************************************************/
static int device_open(struct inode *, struct file *);
static int device_close(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *); 
static int device_mmap(struct file *file, struct vm_area_struct *vma);
static void free_kalloc(void);
static int dev_count = 0;

struct mmap_info {
    char *data; /* the data */
    int reference;       /* how many times it is mmapped */
};

struct mmap_info *info = NULL;

/* Global structures */
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close,
	.write = device_write,
	.read = device_read,
	.mmap = device_mmap
};


struct data {
	int  src;
	int  dest;
	int  bufLength;
	char buf[BUF_LENGTH];
};

struct buffer_device {
	struct cdev *cdev;
	struct class *class;
	struct semaphore sem;
	dev_t num;
	struct data* cirBuf;
} bdev;



/******************************************************************************
 Initializes the driver when command "insmod chardriver.ko" is executed.
******************************************************************************/
static int __init mod_init(void) 
{ 	
	bdev.cdev = NULL;
	bdev.class = NULL;	  

	// allocates a range of character device numbers 	
	if (alloc_chrdev_region(&bdev.num, 0, 1, NAME) < 0) {
		printk(KERN_ALERT "\n%s: failed to allocate a major number", NAME);
		return ERROR;
	}
	// allocate a device class
	if ((bdev.class = class_create(THIS_MODULE, NAME)) == NULL) {
		printk(KERN_ALERT "\n%s: failed to allocate class", NAME);
		free_kalloc();
		return ERROR;
	}
	// allocate a device file
	if (device_create(bdev.class, NULL, bdev.num, NULL, NAME) == NULL) {
		printk(KERN_ALERT "\n%s: failed to allocate device file", NAME);
		free_kalloc();
		return ERROR;
	}	
	// allocates a character device structure 
	bdev.cdev = cdev_alloc();
	bdev.cdev->ops = &fops;
	bdev.cdev->owner = THIS_MODULE;

	 if ((bdev.cirBuf = kcalloc(1, sizeof(struct data), GFP_KERNEL)) == NULL) {
                        printk(KERN_ALERT "\n%s: failed to allocate circular buffer", NAME);
                        free_kalloc();
                        return ERROR;
        }

	
	// add device to the kernel 
	if (cdev_add(bdev.cdev, bdev.num, 1) == ERROR) {
		printk(KERN_ALERT "\n%s: unable to add char device", NAME);
		free_kalloc();
		return ERROR;
	}
	// initialize semaphore
	sema_init(&bdev.sem, 1);
	printk(KERN_ALERT "\n%s: loaded module", NAME);
	return 0;
}

/******************************************************************************
 Exits the driver when command "rmmod chardriver" is executed.
******************************************************************************/
static void __exit mod_exit(void) 
{
  	free_kalloc();
	printk(KERN_ALERT "\n%s: unloaded module", NAME); 
}

int init_mmap_info(struct file *file)
{
	unsigned long order;
	printk(KERN_ALERT "\n%s: opened device", NAME);	
	info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL);
	/* obtain new memory */
	order = get_order(1024*1024);
	pr_info("order %ld\n", order);
	//info->data = (char *)get_zeroed_page(GFP_KERNEL);
	info->data = (char *)__get_free_pages(GFP_KERNEL, order);
	memset(info->data, 0, (1024*1024));
	/* writing something to it */
	//memcpy(info->data, "hello from kernel this is file: ", 32);
	//memcpy(info->data + 32, file->f_dentry->d_name.name,
	//		strlen(file->f_dentry->d_name.name));
	return 0;
}
/******************************************************************************
 Opens the character device.
******************************************************************************/
static int device_open(struct inode *inode, struct file *file)
{ 
	if(!dev_count)
	{
		init_mmap_info(file);
	}
	dev_count++;
	pr_info("dev_count %d\n", dev_count);

	file->private_data = info;
	return 0;
}

/******************************************************************************
 Closes the character device.
******************************************************************************/
static int device_close(struct inode *inode, struct file *file)
{
	struct mmap_info *info = file->private_data;
	printk(KERN_INFO "\n%s: device close", NAME);	
	if(dev_count == 1)
	{
		free_pages((unsigned long)info->data, get_order(1024*1024));
		//free_page((unsigned long)info->data);
		kfree(info);
		file->private_data = NULL;
	}
	dev_count--;
	return 0;
}

/* keep track of how many times it is mmapped */
void mmap_open(struct vm_area_struct *vma)
{
    struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
    info->reference++;
}

/* decrement reference cout */
void mmap_close(struct vm_area_struct *vma)
{
    struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
    info->reference--;
}

int mmap_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	static struct page *page;
	struct mmap_info *info;
	unsigned long address = (unsigned long)vmf->address;
	/* is the address valid? */
	if (address > vma->vm_end) {
		pr_info("invalid address");
		return VM_FAULT_SIGBUS;
	}
	/* the data is in vma->vm_private_data */
	info = (struct mmap_info *)vma->vm_private_data;
	if (!info->data) {
		pr_info("no data");
		return VM_FAULT_SIGBUS;
	}

	page = virt_to_page(info->data);
	/* increment the reference count of this page */
	get_page(page);
	/* type is the page fault type */
	vmf->page = page;
	return 0;
}

struct vm_operations_struct mmap_vm_ops = {
    .open =     mmap_open,
    .close =    mmap_close,
    .fault =    mmap_fault,
};

static int device_mmap(struct file *file, struct vm_area_struct *vma)
{
	pr_info("my mmap called\n");
	vma->vm_ops = &mmap_vm_ops;
	vma->vm_flags |= VM_DONTCOPY | VM_MIXEDMAP;
	vma->vm_flags &= ~VM_MAYWRITE;
	/* assign the file private data to the vm private data */
	vma->vm_private_data = file->private_data;
	mmap_open(vma);
	return 0;

}

/******************************************************************************
 Called when user wants to send information to the device and copies data 
 from kernel space to user space.
******************************************************************************/
static ssize_t device_read(struct file *file, char *dst, size_t count, 
							loff_t *f_offset) {
	
	printk(KERN_INFO "\n%s: reading from device", NAME);
	printk(KERN_INFO "\n%s: The process is \"%s\" (pid %i)\n",NAME, current->comm, current->pid);

	return copy_to_user(dst, bdev.cirBuf, count);
}

/******************************************************************************
 Called when user wants to send information to the device and copies data 
 from user space to kernel space.
******************************************************************************/
static ssize_t device_write(struct file *file, const char *src, size_t count, 
							loff_t *f_offset) {
	// return an error code when device is already open for writting	
	if (down_interruptible(&bdev.sem))
		return -ERESTARTSYS;

	printk(KERN_INFO "\n%s %s: writing to device %zu", NAME, current->comm,count);
	
		
	if (copy_from_user(bdev.cirBuf, (struct data*)src, count) != 0)
               return -EFAULT;

	// free semaphore
	up(&bdev.sem);
	return 0;
}

/******************************************************************************
 Frees the dynamic memory allocations.
******************************************************************************/
void free_kalloc()
{
	// if pointers have valid addresses, the allocations are freed	
	if (bdev.cirBuf)
		kfree(bdev.cirBuf);	
	if (bdev.cdev)
		cdev_del(bdev.cdev);
	if (bdev.class && bdev.num) {
		device_destroy(bdev.class, bdev.num);
		class_destroy(bdev.class);
		unregister_chrdev_region(bdev.num, 1);
	}
}

/******************************************************************************
 Assign module: information, entry point and exit point.
******************************************************************************/
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Srinivas");
MODULE_DESCRIPTION("IPC demo Module");
module_init(mod_init);
module_exit(mod_exit);

/*****************************************************************************/
