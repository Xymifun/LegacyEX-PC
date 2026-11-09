#ifndef GAME_UI_FUIFONTNAME_HPP
#define GAME_UI_FUIFONTNAME_HPP

struct fuiFontName
{
    int fontId;
    char name[256];

    void EndianSwap();
    static void EndianSwap(void* pData);
};

#endif
