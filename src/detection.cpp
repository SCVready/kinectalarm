/**
 * @author Alejandro Solozabal
 *
 * @file detection.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include "detection.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/

Detection::Detection(std::shared_ptr<Kinect> kinect) : m_kinect(kinect)
{
}

Detection::~Detection()
{
}

int Detection::Start()
{
    return 0;
}

int Detection::Stop()
{
    return 0;
}