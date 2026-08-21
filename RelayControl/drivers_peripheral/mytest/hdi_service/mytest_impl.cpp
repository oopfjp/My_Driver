#define HDF_LOG_TAG mytest_driver
#include "mytest_impl.h"
#include <fcntl.h>
#include <unistd.h>
#include <hdf_log.h>

namespace OHOS {
namespace HDI {
namespace Mytest { // 🚨 注意这里：t 必须是小写！与头文件严格保持一致
namespace V1_0 {

int32_t MyTestImpl::SetGpioLevel(int32_t level) {

    // HDF_LOGI("MyTestImpl: SetGpioLevel called with level %{public}d", level);
    FILE *fp = fopen("/data/mytest_proof.txt", "w+");
    
    // 打开 /dev/null 用于打桩测试
    int fd = open("/dev/null", O_RDWR);
    if (fd < 0) {
        HDF_LOGE("Failed to open /dev/null");
        return HDF_FAILURE;
    }

    // 假设高电平写入 '1'，低电平写入 '0'
    char buf = (level == 1) ? '1' : '0';
    ssize_t ret = write(fd, &buf, 1);
    
    if (ret < 0) {
        HDF_LOGE("Failed to write to /dev/null");
        close(fd);
        return HDF_FAILURE;
    } 

    // HDF_LOGI("Successfully wrote %c to /dev/null via HDI!", buf);
    if (fp != nullptr) {
        fprintf(fp, "Successfully wrote %c to /dev/null via HDI!\n", buf);
        fclose(fp);
    }
    close(fd);
    return HDF_SUCCESS;
}

} // V1_0
} // Mytest
} // HDI
} // OHOS