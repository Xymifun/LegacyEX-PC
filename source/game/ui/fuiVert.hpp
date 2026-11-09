#ifndef GAME_UI_FUIVERT_HPP
#define GAME_UI_FUIVERT_HPP

struct fuiVert
{
    float x;
    float y;

    void EndianSwap();
    static void EndianSwap(void* pData);
};

#endif
