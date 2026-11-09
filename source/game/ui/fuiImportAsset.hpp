#ifndef GAME_UI_FUIIMPORTASSET_HPP
#define GAME_UI_FUIIMPORTASSET_HPP

struct fuiImportAsset
{
    char name[64];

    void EndianSwap();
    static void EndianSwap(void* pData);
};

#endif
