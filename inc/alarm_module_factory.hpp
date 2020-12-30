/**
 * @author Alejandro Solozabal
 *
 * @file alarm_module_factory.hpp
 *
 */

#ifndef ALARM_FACTORY__H_
#define ALARM_FACTORY__H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>

#include "alarm_module_interface.hpp"
#include "detection.hpp"
#include "liveview.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class AlarmModuleFactory
{
public:
    static std::shared_ptr<IAlarmModule> CreateDetectionModule(std::shared_ptr<IKinect> kinect,
                                                               std::shared_ptr<DetectionObserver> detection_observer,
                                                               DetectionConfig detection_config);

    static std::shared_ptr<IAlarmModule> CreateLiveviewModule(std::shared_ptr<IKinect> kinect,
                                                              std::shared_ptr<LiveviewObserver> liveview_observer,
                                                              LiveviewConfig liveview_config);
};

#endif /* ALARM_FACTORY__H_ */
