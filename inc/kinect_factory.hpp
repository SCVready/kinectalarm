/**
 * @author Alejandro Solozabal
 *
 * @file kinect_factory.hpp
 *
 */

#ifndef KINECT_FACTORY__H_
#define KINECT_FACTORY__H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>

#include "kinect_interface.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class KinectFactory
{
public:
    static std::shared_ptr<IKinect> Create(uint32_t timeout_ms);
};

#endif /* KINECT_FACTORY__H_ */