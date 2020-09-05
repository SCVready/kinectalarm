/**
 * @author Alejandro Solozabal
 *
 * @file detection.hpp
 *
 */

#ifndef DETECTION_H_
#define DETECTION_H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>

#include "kinect.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class Detection
{
public:
    /**
     * @brief Construct a new Liveview object
     * 
     */
    Detection(std::shared_ptr<Kinect> kinect);

    /**
     * @brief Destroy the Liveview object
     * 
     */
    ~Detection();

    /**
     * @brief 
     * 
     * @return int 
     */
    int Start();

    /**
     * @brief 
     * 
     * @return int 
     */
    int Stop();

    /**
     * @brief 
     * 
     * @return int 
     */
    int IsRunning();

private:
    std::shared_ptr<Kinect> m_kinect;
};

#endif /* DETECTION_H_ */