#ifndef GAME_UI_FJ_SKINSELECT_HPP
#define GAME_UI_FJ_SKINSELECT_HPP

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_SkinSelect : public FJ_Document
{
public:
    FJ_SkinSelect(fuiRenderNode* renderNode);
    virtual ~FJ_SkinSelect() {}
    void SetPlayerCharacterSelected(bool selected, bool visible);
    void SetCharacterLocked(bool locked);
    void SetCharacterFavourite(bool favourite);
    static FJ_SkinSelect* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void handleAnimationEnd(FJ_Event* event);
    static void _handleAnimationEnd(FJ_FuiNode* node, FJ_Event* event);
    virtual void init(FJ_Event* event);
    virtual void SetSafeZone(int top, int bottom, int left, int right);
    void CycleLeft();
    void CycleRight();
    void SetCharacterBlocked(bool blocked);
    virtual void keyDownHandler(FJ_Event* event);

private:
    bool m_animationPlaying;
};

#endif
