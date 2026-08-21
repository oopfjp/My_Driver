insmod是一个由buildroot编译生成的可执行程序，用来加载内核模块。



insmod程序对应的源文件insmod.c位于：
./rk3568_linux_sdk/buildroot/output/rockchip_rk3568/build/busybox-1.34.1/modutils/insmod.c
前提：全量编译sdk才会生成该文件。

#### insmod.c

```c
/* vi: set sw=4 ts=4: */
/*
 * Mini insmod implementation for busybox
 *
 * Copyright (C) 2008 Timo Teras <timo.teras@iki.fi>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config INSMOD
//config:	bool "insmod (22 kb)"
//config:	default y
//config:	help
//config:	insmod is used to load specified modules in the running kernel.

//applet:IF_INSMOD(IF_NOT_MODPROBE_SMALL(APPLET_NOEXEC(insmod, insmod, BB_DIR_SBIN, BB_SUID_DROP, insmod)))

//kbuild:ifneq ($(CONFIG_MODPROBE_SMALL),y)
//kbuild:lib-$(CONFIG_INSMOD) += insmod.o modutils.o
//kbuild:endif

#include "libbb.h"
#include "modutils.h"

/* 2.6 style insmod has no options and required filename
 * (not module name - .ko can't be omitted) */

//usage:#if !ENABLE_MODPROBE_SMALL
//usage:#define insmod_trivial_usage
//usage:	IF_FEATURE_2_4_MODULES("[-fkvqLx] MODULE")
//usage:	IF_NOT_FEATURE_2_4_MODULES("FILE")
//usage:	IF_FEATURE_CMDLINE_MODULE_OPTIONS(" [SYMBOL=VALUE]...")
//usage:#define insmod_full_usage "\n\n"
//usage:       "Load kernel module"
//usage:	IF_FEATURE_2_4_MODULES( "\n"
//usage:     "\n	-f	Force module to load into the wrong kernel version"
//usage:     "\n	-k	Make module autoclean-able"
//usage:     "\n	-v	Verbose"
//usage:     "\n	-q	Quiet"
//usage:     "\n	-L	Lock: prevent simultaneous loads"
//usage:	IF_FEATURE_INSMOD_LOAD_MAP(
//usage:     "\n	-m	Output load map to stdout"
//usage:	)
//usage:     "\n	-x	Don't export externs"
//usage:	)
//usage:#endif

int insmod_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int insmod_main(int argc UNUSED_PARAM, char **argv)
{
	char *filename;
	int rc;

	/* Compat note:
	 * 2.6 style insmod has no options and required filename
	 * (not module name - .ko can't be omitted).
	 * 2.4 style insmod can take module name without .o
	 * and performs module search in default directories
	 * or in $MODPATH.
	 */

	IF_FEATURE_2_4_MODULES(
		getopt32(argv, INSMOD_OPTS INSMOD_ARGS);
		argv += optind - 1;
	);

	filename = *++argv;
	if (!filename)
		bb_show_usage();

	rc = bb_init_module(filename, parse_cmdline_module_options(argv, /*quote_spaces:*/ 0));
	if (rc)
		bb_error_msg("can't insert '%s': %s", filename, moderror(rc));

	return rc;
}

```

当insmod xxx.ko文件时，会调用关键的函数：bb_init_module(filename, parse_cmdline_module_options(argv, /*quote_spaces:*/ 0));

该函数的定义如下：

```c
int FAST_FUNC bb_init_module(const char *filename, const char *options)
{
	size_t image_size;
	char *image;
	int rc;
	bool mmaped;

	if (!options)
		options = "";

//TODO: audit bb_init_module_24 to match error code convention
#if ENABLE_FEATURE_2_4_MODULES
	if (get_linux_version_code() < KERNEL_VERSION(2,6,0))
		return bb_init_module_24(filename, options);
#endif

	/*
	 * First we try finit_module if available.  Some kernels are configured
	 * to only allow loading of modules off of secure storage (like a read-
	 * only rootfs) which needs the finit_module call.  If it fails, we fall
	 * back to normal module loading to support compressed modules.
	 */
# ifdef __NR_finit_module
	{
		int fd = open(filename, O_RDONLY | O_CLOEXEC);
		if (fd >= 0) {
			rc = finit_module(fd, options, 0) != 0;
			close(fd);
			if (rc == 0)
				return rc;
		}
	}
# endif

	image_size = INT_MAX - 4095;
	mmaped = 0;
	image = try_to_mmap_module(filename, &image_size);
	if (image) {
		mmaped = 1;
	} else {
		errno = ENOMEM; /* may be changed by e.g. open errors below */
		image = xmalloc_open_zipped_read_close(filename, &image_size);
		if (!image)
			return -errno;
	}

	errno = 0;
	init_module(image, image_size, options);
	rc = errno;
	if (mmaped)
		munmap(image, image_size);
	else
		free(image);
	return rc;
}
```

可以看出有两种系统调用用来初始化模块：①finit_module、②init_module。我们想知道我们板子上使用insmod命令加载驱动模块是使用上面哪个系统调用可以在模块初始化函数里调用dump_stack()函数来打印出调用关系：

```c
static int  __init define_init(void) /*驱动入口函数*/ 
{

    dump_stack();
    ......
    return 0;
}
```

编译模块后导入到开发板上进行测试：

```shell
root@ATK-DLRK3568:/lib/modules/4.19.232# insmod my_insmod.ko
[326579.017600] CPU: 0 PID: 4495 Comm: insmod Tainted: G           O      4.19.232 #1
[326579.017640] Hardware name: Rockchip RK3568 ATK EVB1 DDR4 V10 Board (DT)
[326579.017650] Call trace:
[326579.017666root@ATK-DLRK3568:/lib/modules/4.19.232# ]  dump_backtrace+0x0/0x188
[326579.017675]  show_stack+0x24/0x30
[326579.017686]  dump_stack+0x8c/0xb4
[326579.017697]  define_init+0x14/0x1000 [my_insmod]
[326579.017705]  do_one_initcall+0xa0/0x1c0
[326579.017713]  do_init_module+0x54/0x1d8
[326579.017723]  load_module+0x1ac8/0x1c14
[326579.017755]  __se_sys_finit_module+0xd8/0xf4
[326579.017771]  __arm64_sys_finit_module+0x24/0x30
[326579.017786]  el0_svc_common.constprop.0+0xe8/0x168
[326579.017800]  el0_svc_handler+0x70/0x8c
[326579.017812]  el0_svc+0x8/0xc
[326579.017822] define_init

```

关键信息如下：[326579.017755]  __se_sys_finit_module+0xd8/0xf4
可以看出使用insmod加载模块时，系统默认使用finit_module()系统调用。

#### 分析finit_module

```c
int FAST_FUNC bb_init_module(const char *filename, const char *options)
{
    ...
# ifdef __NR_finit_module
	{
		int fd = open(filename, O_RDONLY | O_CLOEXEC);
		if (fd >= 0) {
			rc = finit_module(fd, options, 0) != 0;
			close(fd);
			if (rc == 0)
				return rc;
		}
	}
# endif
    ...
}
```

finit_module()是一个宏定义：
             \# define finit_module(fd, uargs, flags) syscall(__NR_finit_module, fd, uargs, flags)
可知其本质是一个系统调用。

#### 分析init_module

```c
int FAST_FUNC bb_init_module(const char *filename, const char *options)
{
    ...
	size_t image_size;
	char *image;
	int rc;
	bool mmaped;

	if (!options)
		options = "";
	image_size = INT_MAX - 4095;
	mmaped = 0;
	image = try_to_mmap_module(filename, &image_size);
	if (image) {
		mmaped = 1;
	} else {
		errno = ENOMEM; /* may be changed by e.g. open errors below */
		image = xmalloc_open_zipped_read_close(filename, &image_size);
		if (!image)
			return -errno;
	}

	errno = 0;
	init_module(image, image_size, options);
	rc = errno;
	if (mmaped)
		munmap(image, image_size);
	else
		free(image);
	return rc;
    ...
}
```

关键处：image = xmalloc_open_zipped_read_close(filename, &image_size); 它也是一个宏定义：\# define xmalloc_open_zipped_read_close(fname, maxsz_p) xmalloc_open_read_close((fname), (maxsz_p))，其功能为读取模块文件的内容并创建一片堆内存，其大小等于文件大小。然后将文件的内容read到该片内存中，并用char *image指向这片内存首地址。最后调用init_module(image, image_size, options);



#### 自己实现insmod的两种方法：finit_module()、init_module()

```c
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>


#define finit_module(fd, uargs, flags) syscall(__NR_finit_module, fd, uargs, flags)
#define init_module(mod, len, opts) syscall(__NR_init_module, mod, len, opts)

// #define FINIT_MODULE //finit_module()
#define INIT_MODULE //init_module()

int main(int argc, char const *argv[])
{
    if(argc < 2){
        printf("<Usage> ./mian argv[1]\n");
        return -1;
    }
#ifdef FINIT_MODULE
    int fd, ret;
    fd = open(argv[1], O_RDONLY | O_CLOEXEC);
    if(fd < 0){
        printf("open failed!\n");
        return -1;
    }

    ret = finit_module(fd, "", 0) != 0;
    close(fd);
    if(ret == 0)
        return ret;
#endif

#ifdef INIT_MODULE

    struct stat statbuf;
    size_t image_size;
    char *image;
    int ret;

    int fd;
    fd = open(argv[1], O_RDONLY);
    if(fd < 0){
        printf("open failed!\n");
        return -1;
    }
    ret = fstat(fd, &statbuf);
    if(-1 == ret){
        printf("fstat failed!\n");
        return -1;
    }
	
    image_size = statbuf.st_size;
    image = (char*)malloc(image_size);
    read(fd, image, image_size);

    ret = init_module(image, image_size, "");
    if(ret < 0){
        printf("init_module failed!\n");
    }else{
        printf("init_module success!\n");
    }
	free(image);
    return ret;
#endif
}

```

