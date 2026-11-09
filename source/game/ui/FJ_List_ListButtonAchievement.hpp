#ifndef GAME_UI_FJ_LIST_LISTBUTTONACHIEVEMENT_HPP
#define GAME_UI_FJ_LIST_LISTBUTTONACHIEVEMENT_HPP

#include <stdint.h>

#include "game/ui/FJ_List2D.hpp"

class fuiRenderNode;

class FJ_List_ListButtonAchievement : public FJ_List2D
{
public:
    explicit FJ_List_ListButtonAchievement(fuiRenderNode* renderNode);

    void addNewItem(int id, const std::wstring& textureName);
    static FJ_List2D* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();

private:
    int m_unknown184;
    int m_unknown188;
};

#endif
