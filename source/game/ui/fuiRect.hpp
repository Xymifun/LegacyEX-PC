#ifndef GAME_UI_FUIRECT_HPP
#define GAME_UI_FUIRECT_HPP

struct fuiRect
{
    float xMin;
    float xMax;
    float yMin;
    float yMax;

    void EndianSwap();
};

#endif
