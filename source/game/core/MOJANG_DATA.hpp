#ifndef GAME_CORE_MOJANG_DATA_HPP
#define GAME_CORE_MOJANG_DATA_HPP

struct MOJANG_DATA
{
    int field0;
    wchar_t capePath[30];
    char pad64[4];
    wchar_t skinPath[30];
    char reserved[124];
};

#endif
