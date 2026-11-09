#ifndef SOURCE_NN_PIA_COMMON_MATH_HPP
#define SOURCE_NN_PIA_COMMON_MATH_HPP

namespace nn
{
namespace pia
{
namespace common
{

template <typename T> const T minimum(T a, T b)
{
    return (a <= b) ? a : b;
}

template <typename T> const T maximum(T a, T b)
{
    return (a >= b) ? a : b;
}

}
}
}

#endif
