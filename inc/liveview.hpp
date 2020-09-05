/**
 * @author Alejandro Solozabal
 *
 * @file liveview.hpp
 *
 */

#ifndef LIVEVIEW_H_
#define LIVEVIEW_H_
/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>

#include "kinect.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class Liveview
{
public:
    /**
     * @brief Construct a new Liveview object
     * 
     */
    Liveview(std::shared_ptr<Kinect> kinect);

    /**
     * @brief Destroy the Liveview object
     * 
     */
    ~Liveview();

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
    uint16_t* video_frame;
};

#endif /* LIVEVIEW_H_ */