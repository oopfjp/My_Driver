#### 正点原子rk3568底板led灯(ds1)点亮gpio引脚配置：引脚复用寄存器、方向寄存器、数据寄存器

##### 1、引脚复用寄存器

base: PMU_GRF_GPIO0C_IOMUX_L(0xFDC20000)

offset: 0x0010

base+offset = 0xFDC20010

![image-20260713153958803](C:\Users\fjp\AppData\Roaming\Typora\typora-user-images\image-20260713153958803.png)

可以看出GPIO0_C0这个引脚复用为gpio功能时需要在bit2:bit0三位都设置为0

先用命令查看该寄存器 0xFDC20010的默认值

![image-20260713154134621](C:\Users\fjp\AppData\Roaming\Typora\typora-user-images\image-20260713154134621.png)

发现bit2:bit0值已经设置为0，那么我们就不用设置这个寄存器了

##### 2、方向寄存器

gpio0的基地址：0xFDD60000

方向寄存器的地址：0xFDD60000+0x000C = 0xFDD6000C

![image-20260713160013326](C:\Users\fjp\AppData\Roaming\Typora\typora-user-images\image-20260713160013326.png)

一个gpio0里面包含4组引脚（gpio0_A，gpio0_B，gpio0_C，gpio0_D）,每组都有8个引脚，那么可以得知C0是bit16，所以使用高16位的方向寄存器地址：GPIO_SWPORT_DDR_H。寄存器是32位，高16位是使能位，哪个引脚要设置就先要使能，低16位是具体引脚的方向配置，1表示输出，0表示输入。所以C0要配置bit0为1(输出)，相对应的bit16要配置为1(使能)。



先用命令查看该寄存器0xFDD6000C的默认值

![image-20260713155957243](C:\Users\fjp\AppData\Roaming\Typora\typora-user-images\image-20260713155957243.png)

0111   0000  0000  0001

可以看出bit0已经被设置为1(输出方向)

那么这个方向寄存器也无需设置

##### 3、数据寄存器

gpio0的基地址：0xFDD60000

数据寄存器的地址：0xFDD60000+0x0004= 0xFDD60004

![image-20260713160744728](C:\Users\fjp\AppData\Roaming\Typora\typora-user-images\image-20260713160744728.png)

先用命令查看该寄存器0xFDD60004的默认值

![image-20260713161104148](C:\Users\fjp\AppData\Roaming\Typora\typora-user-images\image-20260713161104148.png)

**0x00006000**

io -w -4 0xFDD60004 **0x00016000**（手动灭灯）

io -w -4 0xFDD60004 **0x00016001**  (手动亮灯)



#define  GPIO0_C0_DR_H 0xFDD60004

unsigned 









#### 2、led驱动框架的编写

![image-20260713201224049](C:\Users\fjp\AppData\Roaming\Typora\typora-user-images\image-20260713201224049.png)

```c
/*
*	led_drv.c
*/

#define GPIO0_BASE 0xFDD60000
#define GPIO0_C0_DR_H (GPIO0_BASE + 0x0004)

static int major = 0;
static struct class *led_class;
static struct device *dev;
static __u16 __iomem *GPIO0_C0_DR_H_REMAP;


static int led_open(struct inode *node, struct file *fd)
{
    return 0;
}
static int led_release(struct inode *node, struct file *fd)
{
    iounmap(GPIO0_C0_DR_H);
    return 0;
}
static ssize_t led_read(struct file *fd, char __user *buf, size_t sz, loff_t *loft)
{
    return 0;
}
static ssize_t led_write(struct file *fd, const char __user *buf, size_t sz, loff_t *loft)
{
    int err;
    unsigned value;
    u32 val;
    GPIO0_C0_DR_H_REMAP = ioremap(GPIO0_C0_DR_H, 4);
    if (!ledctl) {
		pr_err("Unable to remap memory\n");
		return -1;
	}
    err = copy_from_user(&value, buf, sizeof(unsigned));
    if(value == 1){
        /* led on */
        val = readl(GPIO0_C0_DR_H_REMAP);
        val &= ~(0x1 << 0);
       	val |= ((0X1 << 16) | (0X1 << 0));	/* bit16 置1，允许写bit0， bit0，高电平 */
		writel(val, GPIO0_C0_DR_H_REMAP);
    }else{
        /* led off */
        val = readl(GPIO0_C0_DR_H_REMAP);
        val &= ~(0x1 << 0);
        val |= ((0X1 << 16) | (0X0 << 0));	/* bit16 置1，允许写bit0， bit0，高电平 */
        writel(val, GPIO0_C0_DR_H_REMAP);
    }
    return sizeof(unsigned);
}

static struct file_operations led_ops = {
	.owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
    .read = led_read,
    .realse = led_realse,
};

static int __init led_init(void){
    
    int err;
    major = register_chrdev(0, "chipLed", &led_ops);
    led_class = class_create(THIS_MODULE, "chipLed_class");
    if(IS_ERR(led_class))
        goto fail_clscrte;
	
    dev = device_create(led_class, NULL, MKDEV(major, 0), NULL, "chipLed0");
    if(IS_ERR(dev))
        goto fail_devcrte;
    return 0;
    
 fail_clscrte:
    unregister_chrdev(major);
 
 fail_devcrte:
    class_destory(led_class);
    unregister_chrdev(major);
    
}
static void led_exit(void){
    
    device_destroy(led_class, MKDEV(major, 0));
    class_destroy(led_class);
    unregister_chrdev(major, "chipLed");
}
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
```



```c
/*
* led_test.c
*/


int main(int argc, const char *argv[])
{
    
    return 0;
}
```







































