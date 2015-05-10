#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#define ROWS 15
#define COLS 20
#define BYTES 38

//variables
static unsigned char framebuffer[BYTES] = {0}; //data up to date

//character device implemention
static ssize_t bfb_read(struct file *filp,char __user *buffer,size_t count,loff_t *offp){
	if(*offp>=BYTES)
		return 0;
	if(*offp+count>BYTES)
		count = BYTES - *offp;
	if(copy_to_user(buffer,&framebuffer[*offp],count))
		return -EFAULT;
	*offp += count;
	return count;
}
static ssize_t bfb_write(struct file *filp,const char __user *buffer,size_t count,loff_t *offp){
	if(*offp>BYTES)
		return 0;
	if(*offp+count>=BYTES)
		count = BYTES - *offp;
	if(copy_from_user(&framebuffer[*offp],buffer,count))
		return -EFAULT;
	*offp += count;
	return count;
}
static struct file_operations bfb_fops = {
	.owner = THIS_MODULE,
	.read = bfb_read,
	.write = bfb_write,
};
static struct cdev *bfb_cdev;
static dev_t devno;

//module init and clean up
static int __init bfb_init(void){
	//initialization of character device
	int result = alloc_chrdev_region(&devno,0,1,"blindfb");
	if(result<0){
		printk(KERN_WARNING "blindfb: Error allocating device number");
		return result;
	}
	bfb_cdev = cdev_alloc();
	bfb_cdev->ops = &bfb_fops;
	bfb_cdev->owner = THIS_MODULE;
	result = cdev_add(bfb_cdev,devno,1);
	if(result){
		printk(KERN_WARNING "blindfb: Error adding character device");
		return result;
	}
	return 0;
}
static void __exit bfb_exit(void){
	//destroying character device
	cdev_del(bfb_cdev);
	unregister_chrdev_region(devno,1);
}
module_init(bfb_init);
module_exit(bfb_exit);
