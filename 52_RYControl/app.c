#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* 用法:
 *   ./app on   —— 继电器吸合 (拉高 GPIO3_C5)
 *   ./app off  —— 继电器断开 (拉低 GPIO3_C5)
 */
int main(int argc, char const *argv[])
{
    int fd;
    int ret;
    const char *cmd;

    if (argc >= 2)
        cmd = argv[1];
    else
        cmd = "on";  /* 不传参数时默认吸合 */

    if (strcmp(cmd, "on") != 0 && strcmp(cmd, "off") != 0) {
        printf("usage: %s [on|off]\n", argv[0]);
        return -1;
    }

    fd = open("/dev/ryControl", O_WRONLY);
    if (fd < 0) {
        printf("open /dev/ryControl is error\n");
        return -1;
    }

    ret = write(fd, cmd, strlen(cmd));
    if (ret < 0) {
        printf("write \"%s\" is error\n", cmd);
        close(fd);
        return -1;
    }
    printf("send \"%s\" to relay success\n", cmd);

    close(fd);
    return 0;
}
