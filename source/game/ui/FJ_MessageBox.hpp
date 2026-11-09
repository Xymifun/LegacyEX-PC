#ifndef GAME_UI_FJ_MESSAGEBOX_HPP
#define GAME_UI_FJ_MESSAGEBOX_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_MessageBox : public FJ_Document
{
private:
    int m_buttonCount;
    int m_titleHeight;
    int m_contentY;

public:
    FJ_MessageBox(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void Init(int buttonCount, int focusIndex);
    void AutoResize();
#ifdef MINECRAFT_PC
    void Widen(float fraction);
#endif
};

#endif
