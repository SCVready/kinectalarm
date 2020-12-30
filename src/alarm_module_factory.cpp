/**
 * @author Alejandro Solozabal
 *
 * @file alarm_module_factory.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>

#include "alarm_module_factory.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/

std::shared_ptr<IAlarmModule> AlarmModuleFactory::CreateDetectionModule(std::shared_ptr<IKinect> kinect,
                                                                        std::shared_ptr<DetectionObserver> detection_observer,
                                                                        DetectionConfig detection_config)
{
    return std::make_shared<Detection>(kinect, detection_observer, detection_config);
}

std::shared_ptr<IAlarmModule> AlarmModuleFactory::CreateLiveviewModule(std::shared_ptr<IKinect> kinect,
                                                                       std::shared_ptr<LiveviewObserver> liveview_observer,
                                                                       LiveviewConfig liveview_config)
{
    return std::make_shared<Liveview>(kinect, liveview_observer, liveview_config);
}
