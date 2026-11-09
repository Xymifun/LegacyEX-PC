#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATCHECKREPLY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATCHECKREPLY_HPP

namespace nn
{
namespace pia
{
namespace nat
{

class NatCheckReply
{
public:
    struct Body
    {
        Body() : type(0), portWord(0), address(0), payload(0) {}

        int type;
        int portWord;
        int address;
        int payload;
    };

    NatCheckReply();

    int count;
    Body body;
};

}
}
}

#endif
