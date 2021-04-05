#include <gmock/gmock.h>

#include "../../../inc/alarm_module_interface.hpp"

using ::testing::_;
using ::testing::SaveArg;
using ::testing::SaveArgPointee;
using ::testing::SetArgPointee;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::IsNull;
using ::testing::NotNull;

class AlarmModuleMock : public IAlarmModule
{
public:

    AlarmModuleMock();
    virtual ~AlarmModuleMock();

    MOCK_METHOD(int, Init, ());
    MOCK_METHOD(int, Start, ());
    MOCK_METHOD(int, Stop, ());
    MOCK_METHOD(bool, IsRunning, ());
    MOCK_METHOD(void, UpdateConfig, (AlarmModuleConfig& config));
};
