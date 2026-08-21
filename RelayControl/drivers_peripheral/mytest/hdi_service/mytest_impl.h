#ifndef OHOS_HDI_MYTEST_V1_0_MYTESTIMPL_H
#define OHOS_HDI_MYTEST_V1_0_MYTESTIMPL_H

#include "v1_0/imy_test.h" // 必须包含自动生成的接口

namespace OHOS {
namespace HDI {
namespace Mytest { // 🚨注意：这里的 t 必须是小写！与 IDL 包名保持一致
namespace V1_0 {

class MyTestImpl : public IMyTest {
public:
    virtual ~MyTestImpl() {}
    int32_t SetGpioLevel(int32_t level) override;
};

} // V1_0
} // Mytest
} // HDI
} // OHOS

#endif