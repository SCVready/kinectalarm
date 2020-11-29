#include <memory>

#include <gtest/gtest.h>

#include "../../inc/kinect.hpp"
#include "mocks/libfreenect_mock.hpp"

using ::testing::_;
using ::testing::Return; 
using ::testing::NiceMock;

MockLibFreenect *libfreenect_mock;

TEST(kinecttests, Contructor)
{
    ASSERT_NO_THROW(
        Kinect kinect;
    );
}

TEST(kinecttests, Initialization)
{
    libfreenect_mock = new NiceMock<MockLibFreenect>();

    Kinect kinect;

    EXPECT_CALL(*libfreenect_mock, freenect_init(_,_)).WillOnce(Return(0));
    EXPECT_CALL(*libfreenect_mock, freenect_num_devices(_)).WillOnce(Return(1));

    ASSERT_EQ(kinect.Init(), 0);

    delete static_cast<NiceMock<MockLibFreenect>*>(libfreenect_mock);
}