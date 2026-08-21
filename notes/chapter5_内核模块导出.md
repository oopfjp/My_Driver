### 内核模块导出:EXPORT_SYMBOL(sym)

sym表示变量或者函数，通过导出，其他模块可以使用该内容。

module_param.c:

```c

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

int number = 0;	
EXPORT_SYMBOL(number);  /*导出变量*/

int add(const int lhs, const int rhs){
    return lhs + rhs;
}
EXPORT_SYMBOL(add);		/*导出函数*/


static int __init module_param_init(void) 
{
    printk("math_moudle init\n");
    return 0;
}


static void __exit module_param_exit(void) 
{
	printk("math_module exit\n");
}

module_init(module_param_init); 
module_exit(module_param_exit); 
MODULE_LICENSE("GPL v2"); 
MODULE_AUTHOR("jipeng");

```

hello.c

```c

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

extern int number;	/*请求别的模块帮我定义变量number*/
extern int add(const int lhs, const int rhs);/*请求别的模块帮我定义函数add*/

static int __init hello_init(void) 
{
    static int num;
    printk("hello_moudle init\n");
    printk("number = %d\n", number);
    num = add(1,3);
    printk("add(1,3) = %d\n", num);
    return 0;
}

static void __exit hello_exit(void) 
{
	printk("hello_module exit\n");
}



module_init(hello_init); 
module_exit(hello_exit); 
MODULE_LICENSE("GPL v2"); 
MODULE_AUTHOR("jipeng");

```

Makefile

```makefile

KERNELDIR := /home/alientek/linux_sdk/sdk/kernel 
CURRENT_PATH := $(shell pwd)
obj-m := module_param.o
obj-m += hello.o

build: kernel_modules 

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules 
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean


```



```shell
root@ATK-DLRK3568:/lib/modules/4.19.232# ls
hello.ko  led_drv.ko  module_param.ko
root@ATK-DLRK3568:/lib/modules/4.19.232#
root@ATK-DLRK3568:/lib/modules/4.19.232# insmod hello.ko
[ 4201.359443] hello: Unknown symbol number (err -2)
[ 4201.359491] hello: Unknown symbol add (err -2)
insmod: ERROR: could not insert module hello.ko: Unknown symbol in module
root@ATK-DLRK3568:/lib/modules/4.19.232#
root@ATK-DLRK3568:/lib/modules/4.19.232#
root@ATK-DLRK3568:/lib/modules/4.19.232# insmod module_param.ko
[ 4206.840111] math_moudle init
root@ATK-DLRK3568:/lib/modules/4.19.232#
root@ATK-DLRK3568:/lib/modules/4.19.232#
root@ATK-DLRK3568:/lib/modules/4.19.232# insmod hello.ko
[ 4209.601554] hello_moudle init
[ 4209.601600] number = 0
[ 4209.601607] add(1,3) = 4
root@ATK-DLRK3568:/lib/modules/4.19.232#

```

被依赖模块要先注册，不然依赖模块会找不到前者导出的变量或函数