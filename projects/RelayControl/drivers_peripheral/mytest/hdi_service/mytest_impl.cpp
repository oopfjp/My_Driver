#define HDF_LOG_TAG mytest_driver
#include "mytest_impl.h"
#include <fcntl.h>
#include <unistd.h>
#include <hdf_log.h>

#include <sys/reboot.h>
#ifndef RB_AUTOBOOT
#define RB_AUTOBOOT 0x01234567 
#endif

namespace OHOS {
namespace HDI {
namespace Mytest {
namespace V1_0 {

int32_t MyTestImpl::RebootSystem() {

    sync();
    reboot(RB_AUTOBOOT); 
    return HDF_FAILURE; 
}

int32_t MyTestImpl::SetGpioLevel(int32_t level) {

    
    int fd = open("/dev/ryControl", O_RDWR);
    if (fd < 0) {
        HDF_LOGE("Failed to open /dev/ryControl");
        return HDF_FAILURE;
    }

    // 高电平写入 '1'，低电平写入 '0'
    const char *cmd;
    if(level == 1){
        cmd = "on";
    }else if(level == 0){
        cmd = "off";
    }
    // char buf = (level == 1) ? '1' : '0';
    ssize_t ret = write(fd, cmd, strlen(cmd));
    
    if (ret < 0) {
        HDF_LOGE("Failed to write to /dev/ryControl");
        close(fd);
        return HDF_FAILURE;
    } 

    close(fd);
    return HDF_SUCCESS;
}

} // V1_0
} // Mytest
} // HDI
} // OHOS