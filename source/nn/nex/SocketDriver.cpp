#include "nn/nex/SocketDriver.hpp"

#include "nn/nex/BerkeleySocketDriver.hpp"

namespace nn
{
namespace nex
{

namespace
{

BerkeleySocketDriver g_berkeleySocketDriver;

}

SocketDriver* g_defaultSocketDriver = &g_berkeleySocketDriver;
SocketDriver* g_socketDriver = g_defaultSocketDriver;

}
}
