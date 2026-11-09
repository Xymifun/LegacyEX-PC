#ifndef GAME_UI_FUIRGBA_HPP
#define GAME_UI_FUIRGBA_HPP

struct fuiRGBA
{
    union
    {
        unsigned int rgba;
        struct
        {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;
#else
            unsigned char a;
            unsigned char b;
            unsigned char g;
            unsigned char r;
#endif
        };
    };

    void EndianSwap();
};

#endif
