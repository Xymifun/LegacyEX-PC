#ifndef GAME_UI_FUICOLORTRANSFORM_HPP
#define GAME_UI_FUICOLORTRANSFORM_HPP

struct fuiColorTransform
{
    float redMultiply;
    float greenMultiply;
    float blueMultiply;
    float alphaMultiply;
    float redAdd;
    float greenAdd;
    float blueAdd;
    float alphaAdd;

    void EndianSwap();
};

#endif
