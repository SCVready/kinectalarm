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

#include "kinect_factory.hpp"
#include "kinect.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/

std::shared_ptr<IKinect> KinectFactory::Create(uint32_t timeout_ms)
{
    return std::make_shared<Kinect>(timeout_ms);
}
