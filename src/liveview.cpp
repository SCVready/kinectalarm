/**
 * @author Alejandro Solozabal
 *
 * @file liveview.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include "liveview.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/

Liveview::Liveview(std::shared_ptr<Kinect> kinect) : m_kinect(kinect)
{
}

Liveview::~Liveview()
{
}

int Liveview::Start()
{
    return 0;
}

int Liveview::Stop()
{
    return 0;
}