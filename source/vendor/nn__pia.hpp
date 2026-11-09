#ifndef NN_PIA_VENDOR_HPP
#define NN_PIA_VENDOR_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{

void Assert(const char* expression, const char* file, unsigned int line, const char* message, ...);
void EmbedMiddlewareInfo();

}
}

#endif
