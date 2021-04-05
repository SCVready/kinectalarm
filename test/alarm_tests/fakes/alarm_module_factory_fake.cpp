/**
 * @author Alejandro Solozabal
 *
 * @file alarm_modules_factory_fake.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>

#include "../../../inc/alarm_module_factory.hpp"
#include "../mocks/alarm_module_mock.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/
extern std::shared_ptr<AlarmModuleMock> g_detection_mock;
extern std::shared_ptr<AlarmModuleMock> g_liveview_mock;

std::shared_ptr<IAlarmModule> AlarmModuleFactory::CreateDetectionModule(std::shared_ptr<IKinect> kinect,
                                                                        std::shared_ptr<DetectionObserver> detection_observer,
                                                                        DetectionConfig detection_config)
{
    return g_detection_mock;
}

std::shared_ptr<IAlarmModule> AlarmModuleFactory::CreateLiveviewModule(std::shared_ptr<IKinect> kinect,
                                                                       std::shared_ptr<LiveviewObserver> liveview_observer,
                                                                       LiveviewConfig liveview_config)
{
    return g_liveview_mock;
}
