#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/err.h>


struct device_test{
    dev_t dev_num; /* 设备号,32位,前12位是主设备号,后20位是次设备号 */
    struct cdev cdev_m; /* 字符设备结构体 */
    struct class *class;
    int minor;
    struct device *dev;
    char buf[64];
};
struct device_test devs[2];

static int chrdev_open(struct inode* inode, struct file* file)
{
    
    devs[0].minor = 0;
    devs[1].minor = 1;
    file->private_data = container_of(inode->i_cdev, struct device_test, cdev_m);
    printk(KERN_INFO "This is chrdev_open \n");
    return 0;
}

static ssize_t chrdev_read(struct file* file, char __user* buf, size_t size, loff_t* off)
{

    printk(KERN_INFO "This is chrdev_read\n");
    return 0;
}

static ssize_t chrdev_write(struct file* file, const char __user* buf, size_t size, loff_t* off)
{

    struct device_test *dev_ptr = (struct device_test *)file->private_data;
    
    unsigned long ret;
    switch (dev_ptr->minor)
    {
    case 0:
        ret = copy_from_user(devs[0].buf, buf, 64);
        printk(KERN_INFO "This is chrdev_write, minor = 0, buf = %s\n", devs[0].buf);
        break;
    case 1:
        ret = copy_from_user(devs[1].buf, buf, 64);
        printk(KERN_INFO "This is chrdev_write, minor = 1, buf = %s\n", devs[1].buf);
        break;
    default:
        break;
    }

    return 0;
}


static int chrdev_release(struct inode* inode, struct file* file)
{

    return 0;
}

static struct file_operations fops = {

    .owner = THIS_MODULE,
    .open = chrdev_open,
    .read = chrdev_read,
    .write = chrdev_write,
    .release = chrdev_release,
};


static int  __init dev_init(void) /*驱动入口函数*/ 
{

    unsigned ret;
    
    /*动态申请主设备号,其包含2个次设备号,次设备号从0开始*/
    ret = alloc_chrdev_region(&devs[0].dev_num, 0, 2, "reg_dynamic_dev");

    if(ret < 0){
        printk(KERN_ERR "register unsuccessful..\n");
        return -EINVAL;
    }
    
    printk(KERN_INFO "alloc_register_region is ok\n");

    /* 将字符设备与设备文件相关操作所关联 */
    /* 设备1 */
    devs[0].cdev_m.owner = THIS_MODULE;
    cdev_init(&devs[0].cdev_m, &fops);
    
    /* 注册字符设备 */
    ret = cdev_add(&devs[0].cdev_m, devs[0].dev_num, 1);
    devs[0].class = class_create(THIS_MODULE, "private_class0");
    devs[0].dev = device_create(devs[0].class, NULL, devs[0].dev_num, NULL, "private_device0");
    


    /* 设备2 */
    devs[1].cdev_m.owner = THIS_MODULE;
    cdev_init(&devs[1].cdev_m, &fops);
    /* 注册字符设备 */
    ret = cdev_add(&devs[1].cdev_m, devs[1].dev_num+1, 1);
    devs[1].class = class_create(THIS_MODULE, "private_class1");
    devs[1].dev = device_create(devs[1].class, NULL, devs[1].dev_num+1, NULL, "private_device1");

    return 0;
}

static void dev_exit(void)
{
    unregister_chrdev_region(devs[0].dev_num, 1);
    unregister_chrdev_region(devs[1].dev_num, 1);
    cdev_del(&devs[0].cdev_m);
    cdev_del(&devs[1].cdev_m);
    device_destroy(devs[0].class, devs[0].dev_num+1);
    device_destroy(devs[1].class, devs[1].dev_num);
    class_destroy(devs[0].class);
    class_destroy(devs[1].class);
    printk(KERN_INFO "module exit\n");
} 



// arch_initcall(dev_init);  
module_init(dev_init);  
module_exit(dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("jipeng");