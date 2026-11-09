#include "nn/pia/nat/NatCheckReply.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

NatCheckReply::NatCheckReply() : count(0), body()
{
    body.type = 0;
    body.portWord = 0;
    body.address = 0;
    body.payload = 0;
}

}
}
}
