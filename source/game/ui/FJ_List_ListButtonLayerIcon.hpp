#ifndef GAME_UI_FJ_LIST_LISTBUTTONLAYERICON_HPP
#define GAME_UI_FJ_LIST_LISTBUTTONLAYERICON_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_List.hpp"

class fuiRenderNode;

class FJ_List_ListButtonLayerIcon : public FJ_List
{
public:
    explicit FJ_List_ListButtonLayerIcon(fuiRenderNode* renderNode);

    static FJ_List* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void UseIggyIcons();
    void addNewItem(const std::wstring& label, const std::wstring& secondaryText, int id,
                    const std::wstring& textureName);
    void SetLayerLabel(int index, const std::wstring& label);
    void SetTextureName(int index, const std::wstring& textureName);

private:
    bool m_useIggyIcons;
};

#endif
