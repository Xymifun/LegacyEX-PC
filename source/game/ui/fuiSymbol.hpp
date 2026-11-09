#ifndef GAME_UI_FUISYMBOL_HPP
#define GAME_UI_FUISYMBOL_HPP

enum eFuiObjectType
{
    eFuiObjectType_None = 0,
    eFuiObjectType_Shape = 1,
    eFuiObjectType_Timeline = 2,
    eFuiObjectType_Bitmap = 3,
    eFuiObjectType_Reference = 4,
    eFuiObjectType_EditText = 5,
    eFuiObjectType_CodeGenRect = 6
};

struct fuiSymbol
{
    char name[64];
    eFuiObjectType objectType;
    unsigned int objectIndex;

    void EndianSwap();
    static void EndianSwap(void* pData);
};

#endif
