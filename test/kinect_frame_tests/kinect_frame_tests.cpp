/**
 * @author Alejandro Solozabal
 *
 * @file kinect_frame_test.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../inc/kinect_frame.hpp"

/*******************************************************************
 * Test class definition
 *******************************************************************/
class KinectFrameTest : public ::testing::Test
{
public:
    KinectFrameTest()
    {
        test_data_1.resize(width * height);
        test_data_2.resize(width * height);
    }

    ~KinectFrameTest()
    {
    }

    void FillWithEvenOddPattern(std::vector<uint16_t>& test_data)
    {
        test_data.assign(test_data.size(), 1);

        for(uint32_t i = 0; i < test_data.size(); i += 2)
        {
            test_data[i] = 0;
        }
    }

    void FillWithValue(std::vector<uint16_t>& test_data, uint16_t value)
    {
        test_data.assign(test_data.size(), value);
    }

    void ChangeWithValue(std::vector<uint16_t>::iterator it_begin, std::vector<uint16_t>::iterator it_end,uint16_t value)
    {
        for(auto it = it_begin; it != it_end; it++)
        {
            *it = value;
        }
    }

protected:
    uint32_t width = 1920, height = 1080;
    uint32_t timestamp = 1111;
    uint32_t tolerance = 10;
    uint32_t num_differences = 100;
    uint32_t pix_value = 12345;

    std::vector<uint16_t> test_data_1;
    std::vector<uint16_t> test_data_2;
};

/*******************************************************************
 * Test cases
 *******************************************************************/
TEST_F(KinectFrameTest, Contructor)
{
    ASSERT_NO_THROW(
        KinectDepthFrame kinect_frame(1920,1080);
    );
}

TEST_F(KinectFrameTest, Fill)
{
    KinectDepthFrame kinect_frame(width, height);

    FillWithEvenOddPattern(test_data_1);

    kinect_frame.Fill(test_data_1.data(), timestamp);

    EXPECT_EQ(kinect_frame.GetTimestamp(), timestamp);

    EXPECT_EQ(kinect_frame.GetDataPointer()[0], 0);
    EXPECT_EQ(kinect_frame.GetDataPointer()[1], 1);

    EXPECT_EQ(kinect_frame.GetDataPointer()[(width*height)-1], 1);
    EXPECT_EQ(kinect_frame.GetDataPointer()[(width*height)-2], 0);
}

TEST_F(KinectFrameTest, TimestampGetAndSet)
{
    KinectDepthFrame kinect_frame(width, height);
    kinect_frame.SetTimestamp(timestamp);
    EXPECT_EQ(kinect_frame.GetTimestamp(),timestamp);
}

TEST_F(KinectFrameTest, ComputeDifferences1)
{
    FillWithValue(test_data_1, pix_value);
    FillWithValue(test_data_2, pix_value + tolerance + 1);

    KinectDepthFrame kinect_depth_frame_1(width, height);
    KinectDepthFrame kinect_depth_frame_2(width, height);

    kinect_depth_frame_1.Fill(test_data_1.data(),0);
    kinect_depth_frame_2.Fill(test_data_2.data(),0);

    EXPECT_EQ(kinect_depth_frame_1.ComputeDifferences(kinect_depth_frame_2, tolerance), width * height);
    EXPECT_EQ(kinect_depth_frame_2.ComputeDifferences(kinect_depth_frame_1, tolerance), width * height);
}

TEST_F(KinectFrameTest, ComputeDifferences2)
{
    FillWithValue(test_data_1, pix_value);
    FillWithValue(test_data_2, pix_value + tolerance);

    KinectDepthFrame kinect_depth_frame_1(width, height);
    KinectDepthFrame kinect_depth_frame_2(width, height);

    kinect_depth_frame_1.Fill(test_data_1.data(),0);
    kinect_depth_frame_2.Fill(test_data_2.data(),0);

    EXPECT_EQ(kinect_depth_frame_1.ComputeDifferences(kinect_depth_frame_2, tolerance), 0);
    EXPECT_EQ(kinect_depth_frame_2.ComputeDifferences(kinect_depth_frame_1, tolerance), 0);
}

TEST_F(KinectFrameTest, ComputeDifferences3)
{
    FillWithValue(test_data_1, pix_value);
    FillWithValue(test_data_2, pix_value);
    ChangeWithValue(test_data_2.begin(),test_data_2.begin() + num_differences, pix_value + tolerance + 1);

    KinectDepthFrame kinect_depth_frame_1(width, height);
    KinectDepthFrame kinect_depth_frame_2(width, height);

    kinect_depth_frame_1.Fill(test_data_1.data(),0);
    kinect_depth_frame_2.Fill(test_data_2.data(),0);

    EXPECT_EQ(kinect_depth_frame_1.ComputeDifferences(kinect_depth_frame_2, tolerance), num_differences);
    EXPECT_EQ(kinect_depth_frame_2.ComputeDifferences(kinect_depth_frame_1, tolerance), num_differences);
}

TEST_F(KinectFrameTest, SaveToJpegInFileDepthFrame)
{
    KinectDepthFrame kinect_frame(width, height);
    FillWithValue(test_data_1,80);
    kinect_frame.Fill(test_data_1.data(),0);
    kinect_frame.SaveToJpegInFile("depth_frame_test.jpeg",1,1);
}

TEST_F(KinectFrameTest, SaveToJpegInFileVideoFrame)
{
    KinectVideoFrame kinect_frame(width, height);
    FillWithValue(test_data_1,240);
    kinect_frame.Fill(test_data_1.data(),0);
    kinect_frame.SaveToJpegInFile("video_frame_test.jpeg",1,1);
}
