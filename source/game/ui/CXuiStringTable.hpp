#ifndef GAME_UI_CXUISTRINGTABLE_HPP
#define GAME_UI_CXUISTRINGTABLE_HPP

class CXuiStringTable
{
public:
    const wchar_t* Lookup(const wchar_t* stringId);
    const wchar_t* Lookup(unsigned int index);
    void Clear();
    int Load(const wchar_t* stringId);
};

#endif
