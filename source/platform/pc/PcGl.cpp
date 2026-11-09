#include "platform/pc/PcGl.hpp"

#include "platform/pc/PcOs.hpp"

#include <stdio.h>

namespace PcGl
{
#define PC_GL_DEFINE(type, name) type name = 0;
PC_GL_FUNCTIONS(PC_GL_DEFINE)
#undef PC_GL_DEFINE

bool Load()
{
    bool complete = true;
#define PC_GL_LOAD(type, name)                                                                                         \
    name = reinterpret_cast<type>(PcOsGlGetProcAddress("gl" #name));                                                  \
    if (name == 0)                                                                                                     \
    {                                                                                                                  \
        printf("[pc] gl: missing gl" #name "\n");                                                                      \
        complete = false;                                                                                              \
    }
    PC_GL_FUNCTIONS(PC_GL_LOAD)
#undef PC_GL_LOAD
    return complete;
}
}
