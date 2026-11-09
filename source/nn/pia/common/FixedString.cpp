#include "nn/pia/common/FixedString.hpp"

#include <cstring>

namespace nn
{
namespace pia
{
namespace common
{

template class FixedString<8>;
template class FixedString<10>;
template class FixedString<16>;
template class FixedString<20>;
template class FixedString<32>;
template class FixedString<64>;
template class FixedString<256>;

}
}
}
