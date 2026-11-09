#ifndef GAME_UI_FJ_TUTORIALPOPUP_HPP
#define GAME_UI_FJ_TUTORIALPOPUP_HPP

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_TutorialPopup : public FJ_FuiNode
{
public:
    FJ_TutorialPopup(fuiRenderNode* pRenderNode);

    static FJ_TutorialPopup* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    void SetSafeZone(int top, int bottom, int left, int right);
    void ShowTitle(bool visible);
    void ShowExitScreenShot(bool visible);
    void ShowDescription(bool visible);
    void ShowIconHolder(bool visible);
    void AdjustLayout(bool reposition);
    fuiRenderNode* GetBitmapData(int type);
    void SetupIconHolder(int type);

private:
    int m_titleY;
    int m_exitScreenShotY;
    int m_descriptionY;
    int m_iconHolderY;
    FJ_FuiNode* m_backgroundPanel;
    int m_titleHeight;
    FJ_FuiNode* m_iconHolder;
    fuiRenderNode* m_bitmap;
};

#endif
