#include <gmock/gmock.h>

#include <libfreenect/libfreenect.h>

class MockLibFreenect
{
    public:
    MOCK_METHOD(int, freenect_init, (freenect_context**, freenect_usb_context*));
    MOCK_METHOD(int, freenect_num_devices, (freenect_usb_context*));
};