#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <asm/io.h>
/*继电器由GPIO3_C5控制*/

#define  SYS_GRF_BASE (0xFDC60000)
#define  GRF_GPIO3C_IOMUX_H (SYS_GRF_BASE + 0x0054)
#define  GPIO3_BASE (0xFE760000)
#define  DDR_OFFSET (0x000C)
#define  DR_OFFSET (0x0004)

struct RY_data{
    dev_t dev_num;
    struct cdev cdev_m;
    struct class *class_t;
    struct device *dev;
    void __iomem *SYS_GRF_IOMUX;
    void __iomem *GPIO_SWPORT_DDR_H;
    void __iomem *GPIO_SWPORT_DR_H;
};

struct RY_data rdata;

static int chrdev_open(struct inode* inode, struct file* file)
{
    file->private_data = container_of(inode->i_cdev, struct RY_data, cdev_m);
    return 0;
}

static ssize_t chrdev_read(struct file* file, char __user* buf, size_t size, loff_t* off)
{

    printk(KERN_INFO "This is chrdev_read\n");
    return 0;
}

static ssize_t chrdev_write(struct file* file, const char __user* buf, size_t size, loff_t* off)
{
    u32 val = 0;
    val = readl(rdata.GPIO_SWPORT_DR_H);
    val &= ~(0x20 << 0); /*bit5清零*/
    val |= (0x20 << 16); /*使能*/


    struct RY_data *rdata_ptr = file->private_data;
    unsigned long ret;
    char ker_buf[16] = {0};

    if(size > sizeof(ker_buf)-1)
        size = sizeof(ker_buf)-1;
    
    ret = copy_from_user(ker_buf, buf, size);
    if (ret) {
        printk(KERN_ERR "copy_from_user failed, ret = %lu\n", ret);
        return -EFAULT;
    }
    if(!strcmp(ker_buf, "on")){
        printk(KERN_INFO "set gpio level high\n");
        val |= (0x20 << 0); /*拉高*/
        writel(val, rdata_ptr->GPIO_SWPORT_DR_H);
    }else if(!strcmp(ker_buf, "off")){
        printk(KERN_INFO "set gpio level low\n");
        val &= ~(0x20 << 0); /*拉低*/
        writel(val, rdata_ptr->GPIO_SWPORT_DR_H);
    }
    return size;
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


static int  __init dev_init(void) 
{

    int ret;
    u32 val = 0;
    
    ret = alloc_chrdev_region(&rdata.dev_num, 0, 1, "ryDev");

    if(ret < 0){
        printk(KERN_ERR "register unsuccessful..\n");
        return -EINVAL;
    }
    
    printk(KERN_INFO "alloc_register_region is ok\n");

    /* 只映射一次，避免每次 open 重复 ioremap 造成映射泄漏 */
    rdata.SYS_GRF_IOMUX = ioremap(GRF_GPIO3C_IOMUX_H, 4);
    rdata.GPIO_SWPORT_DDR_H = ioremap(GPIO3_BASE + DDR_OFFSET, 4);
    rdata.GPIO_SWPORT_DR_H = ioremap(GPIO3_BASE + DR_OFFSET, 4);
    if (!rdata.SYS_GRF_IOMUX || !rdata.GPIO_SWPORT_DDR_H || !rdata.GPIO_SWPORT_DR_H) {
        printk(KERN_ERR "ioremap failed\n");
        ret = -ENOMEM;
        goto ioremap_err;
    }

    /*GPIO3_C5复用为GPIO功能*/
    val = readl(rdata.SYS_GRF_IOMUX);
    val &= ~(0x70 << 0); /*bit4~bit6清零*/
    val |= ((0x70 << 16) | (0x0 << 0));
    writel(val, rdata.SYS_GRF_IOMUX);

    /*配置方向寄存器DDR：bit5 设为输出 */
    val = readl(rdata.GPIO_SWPORT_DDR_H);
    val &= ~(0x20 << 0); /*bit5清零*/
    val |= ((0x20 << 16) | (0x20 << 0));
    writel(val, rdata.GPIO_SWPORT_DDR_H);

    /* 将字符设备与设备文件相关操作所关联 */
    cdev_init(&rdata.cdev_m, &fops);
    rdata.cdev_m.owner = THIS_MODULE;
    
    /* 注册字符设备 */
    ret = cdev_add(&rdata.cdev_m, rdata.dev_num, 1);
     if(ret < 0){
        printk(KERN_ERR "cdev_add unsuccessful..\n");
        goto cdevadd_err;
    }
    printk(KERN_INFO "cdev_add is ok!\n");

    rdata.class_t = class_create(THIS_MODULE, "ryClass");
    if(IS_ERR(rdata.class_t)){
        ret = PTR_ERR(rdata.class_t);
        printk(KERN_ERR "class_create unsuccessful..\n");
        goto clscrea_err;
    }
    rdata.dev = device_create(rdata.class_t, NULL, rdata.dev_num, NULL, "ryControl");
    if(IS_ERR(rdata.dev)){
        ret = PTR_ERR(rdata.dev);
        printk(KERN_ERR "device_create unsuccessful..\n");
        goto devcrea_err;
    }
    return 0;

devcrea_err:
    class_destroy(rdata.class_t);
    cdev_del(&rdata.cdev_m);
    iounmap(rdata.SYS_GRF_IOMUX);
    iounmap(rdata.GPIO_SWPORT_DDR_H);
    iounmap(rdata.GPIO_SWPORT_DR_H);
    unregister_chrdev_region(rdata.dev_num, 1);
    return ret;
clscrea_err:
    cdev_del(&rdata.cdev_m);
    iounmap(rdata.SYS_GRF_IOMUX);
    iounmap(rdata.GPIO_SWPORT_DDR_H);
    iounmap(rdata.GPIO_SWPORT_DR_H);
    unregister_chrdev_region(rdata.dev_num, 1);
    return ret;
cdevadd_err:
    iounmap(rdata.SYS_GRF_IOMUX);
    iounmap(rdata.GPIO_SWPORT_DDR_H);
    iounmap(rdata.GPIO_SWPORT_DR_H);
    unregister_chrdev_region(rdata.dev_num, 1);
    return ret;
ioremap_err:
    if (rdata.SYS_GRF_IOMUX)
        iounmap(rdata.SYS_GRF_IOMUX);
    if (rdata.GPIO_SWPORT_DDR_H)
        iounmap(rdata.GPIO_SWPORT_DDR_H);
    if (rdata.GPIO_SWPORT_DR_H)
        iounmap(rdata.GPIO_SWPORT_DR_H);
    unregister_chrdev_region(rdata.dev_num, 1);
    return ret;

}

static void dev_exit(void)
{
    /* 统一释放 open 时不再重复映射的寄存器地址 */
    iounmap(rdata.SYS_GRF_IOMUX);
    iounmap(rdata.GPIO_SWPORT_DDR_H);
    iounmap(rdata.GPIO_SWPORT_DR_H);

    device_destroy(rdata.class_t, rdata.dev_num);
    class_destroy(rdata.class_t);
    cdev_del(&rdata.cdev_m);
    unregister_chrdev_region(rdata.dev_num, 1);
    printk(KERN_INFO "module exit\n");
} 



// arch_initcall(dev_init);  
module_init(dev_init);  
module_exit(dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("jipeng");