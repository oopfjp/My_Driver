驱动的基本框架

```c
#include <linux/module.h>
#include <linux/kernel.h>

static int __init helloworld_init(void) //驱动入口函数
{
    printk(KERN_EMERG "helloworld_init\r\n");//注意：内核打印用 printk 而不是 printf
    return 0;
}

static void helloworld_exit(void) //驱动出口函数
{
	printk(KERN_EMERG "helloworld_exit\r\n");
}

module_init(helloworld_init); //注册入口函数
module_exit(helloworld_exit); //注册出口函数
MODULE_LICENSE("GPL v2"); //同意 GPL 开源协议
MODULE_AUTHOR("topeet"); //作者信息
```

#### 驱动传参

意义

1. 通过驱动传参，可以让驱动程序更加灵活。兼容性更强。
2. 可以通过驱动传参，设置安全校验，防止驱动被盗用。

不足

1. 使得驱动代码变得复杂化。
2. 增加了驱动的资源占比。
   

module_param(name, type, perm)

module_param_array(name, type, nump, perm)

module_param_string(name, string, len, perm)

可以传入的参数类型：

- bool **:** 布尔型
- inbool **:** 布尔反值
- charp**:** 字符指针（相当于 char *,不超过 1024 字节的字符串）
- short**:** 短整型
- ushort **:** 无符号短整型
- int **:** 整型
- uint **:** 无符号整型
- long **:** 长整型
- ulong**:** 无符号长整型。

 

examples

```c

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

static int number = 0;	
static int nums[10] = {0};
static int n_para;
static char cbuf[32] = {0};
module_param(number, int, S_IRUGO); /*S_IRUGO:所有用户是可读的*/

module_param_array(nums, int, &n_para, S_IRUGO);

module_param_string(str, cbuf ,sizeof(cbuf), S_IRUGO);

static int __init helloworld_init(void) 
{
    int i;
    printk("helloworld_init\r\n");
	printk("number = %d\n", number);
    printk("------------\n");
    for(i = 0; i != n_para; ++i)
	    printk("%d ", nums[i]);

    printk("------------\n");
    printk("cbuf = %s\n", cbuf);
    return 0;
}


static void __exit helloworld_exit(void) 
{
	printk(KERN_EMERG "helloworld_exit\r\n");
}



module_init(helloworld_init); 
module_exit(helloworld_exit); 
MODULE_LICENSE("GPL v2"); 
MODULE_AUTHOR("topeet");

```

测试：

```shell
root@ATK-DLRK3568:/lib/modules/4.19.232# lsmod
Module                  Size  Used by
8852bs               3895296  0
root@ATK-DLRK3568:/lib/modules/4.19.232#
root@ATK-DLRK3568:/lib/modules/4.19.232#
root@ATK-DLRK3568:/lib/modules/4.19.232# insmod led_drv.ko number=10 nums=1,2,3,4,5 str="nihao"
[ 2929.420078] helloworld_init
[ 2929.420129] number = 10
[ 2929.420136] ------------
[ 2929.420143] 1
[ 2929.420144] 2
[ 2929.420150] 3
[ 2929.420155] 4
[ 2929.420160] 5
[ 2929.420166] ------------
[ 2929.420178] cbuf = nihao
root@ATK-DLRK3568:/lib/modules/4.19.232#

```

