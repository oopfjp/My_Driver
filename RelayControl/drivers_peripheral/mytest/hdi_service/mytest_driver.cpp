#include <hdf_base.h>
#include <hdf_device_desc.h>
#include <hdf_log.h>
#include <hdf_sbuf_ipc.h>

// 🚨 必须同时包含这三个头文件，缺一不可！
#include "v1_0/imy_test.h"      // 系统生成的接口定义 (提供 IMyTest)
#include "v1_0/my_test_stub.h"  // 系统生成的服务端存根
#include "mytest_impl.h"        // 你自己写的实现类 (提供 MyTestImpl)

// 不用 using namespace，彻底杜绝大小写找不到的问题！

struct HdfMyTestHost {
    struct IDeviceIoService ioService;
    OHOS::sptr<OHOS::IRemoteObject> stub;
};

static int32_t MyTestDriverDispatch(struct HdfDeviceIoClient *client, int cmdId, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    auto *hdfMyTestHost = CONTAINER_OF(client->device->service, struct HdfMyTestHost, ioService);
    
    OHOS::MessageParcel *dataParcel = nullptr;
    OHOS::MessageParcel *replyParcel = nullptr;
    OHOS::MessageOption option;

    if (SbufToParcel(data, &dataParcel) != HDF_SUCCESS) {
        return HDF_ERR_INVALID_PARAM;
    }
    if (SbufToParcel(reply, &replyParcel) != HDF_SUCCESS) {
        return HDF_ERR_INVALID_PARAM;
    }

    return hdfMyTestHost->stub->SendRequest(cmdId, *dataParcel, *replyParcel, option);
}

static int HdfMyTestDriverBind(struct HdfDeviceObject *deviceObject)
{
    auto *hdfMyTestHost = new (std::nothrow) HdfMyTestHost;
    if (hdfMyTestHost == nullptr) {
        return HDF_FAILURE;
    }

    hdfMyTestHost->ioService.Dispatch = MyTestDriverDispatch;
    hdfMyTestHost->ioService.Open = NULL;
    hdfMyTestHost->ioService.Release = NULL;

    // 🚨 终极大法：全部使用完整的绝对命名空间路径，彻底消灭歧义！
    OHOS::sptr<OHOS::HDI::Mytest::V1_0::IMyTest> serviceImpl = 
        new (std::nothrow) OHOS::HDI::Mytest::V1_0::MyTestImpl();
        
    if (serviceImpl == nullptr) {
        delete hdfMyTestHost;
        return HDF_FAILURE;
    }

    // 🚨 同上：全路径调用 GetDescriptor()
    hdfMyTestHost->stub = OHOS::HDI::ObjectCollector::GetInstance().GetOrNewObject(
        serviceImpl, 
        OHOS::HDI::Mytest::V1_0::IMyTest::GetDescriptor()
    );
    
    if (hdfMyTestHost->stub == nullptr) {
        delete hdfMyTestHost;
        return HDF_FAILURE;
    }

    deviceObject->service = &hdfMyTestHost->ioService;
    return HDF_SUCCESS;
}

static int HdfMyTestDriverInit(struct HdfDeviceObject *deviceObject)
{
    return HDF_SUCCESS;
}

static void HdfMyTestDriverRelease(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject->service == nullptr) {
        return;
    }
    auto *hdfMyTestHost = CONTAINER_OF(deviceObject->service, struct HdfMyTestHost, ioService);
    delete hdfMyTestHost;
}

struct HdfDriverEntry g_mytestDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "mytest_driver",
    .Bind = HdfMyTestDriverBind,
    .Init = HdfMyTestDriverInit,
    .Release = HdfMyTestDriverRelease,
};

HDF_INIT(g_mytestDriverEntry);