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

#include "../../../inc/kinect_factory.hpp"
#include "../../mocks/kinect_mock.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/
extern std::shared_ptr<KinectMock> kinect_mock;

std::shared_ptr<IKinect> KinectFactory::Create(uint32_t timeout_ms)
{
    return kinect_mock;
}
