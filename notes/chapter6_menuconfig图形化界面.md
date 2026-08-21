menuconfig 图形化的配置工具可以手动的配置内核编译时加载的驱动、它需要 ncurses 库支持，其提供了一系列的 API 函数供调用者生成基于文本的图形界面，因此在使用 menuconfig 图形化配置界面之前需要先在 虚拟机 Ubuntu 中安装 ncurses 库：

```shell
sudo apt-get install build-essential
sudo apt-get install libncurses5-dev
```

#### 打开menuconfig图形化配置界面：

在内核源码目录下

```shell
make menuconfig
```

会出现一个图形化界面，涵盖了多种在Kconfig配置的驱动选项，使用方向键+Y、M、空格可以选择将驱动编译成内核中、模块或不进行编译。

选择完驱动编译选项后，保存退出会在kernel原目录下生成一个本次编译的驱动选项列表文件：.config

Kconfig 如同饭店的菜单，.config 就是客人点好的菜。根据.config 中的配置编译内核可以类比成厨师根据客人点好的菜品（即.config）来烹饪。

