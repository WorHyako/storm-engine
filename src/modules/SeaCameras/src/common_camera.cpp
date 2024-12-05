#include "common_camera.h"

#include "string_compare.hpp"

uint64_t COMMON_CAMERA::ProcessMessage(MESSAGE &msg)
{
    const std::string &command = msg.String();

    if (storm::iEquals(command, "SetCharacter"))
    {
        ATTRIBUTES *attr = msg.AttributePointer();
        SetCharacter(attr);
        return 0;
    }

    return Camera::ProcessMessage(msg);
}
