#ifndef GAME_UI_FJ_INTRO_HPP
#define GAME_UI_FJ_INTRO_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_Intro : public FJ_Document
{
public:
    FJ_Intro(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void handleAnimationStep(FJ_Event* event);
    static void _handleAnimationStep(FJ_FuiNode* node, FJ_Event* event);
    virtual void init(FJ_Event* event);
    void SetIntroPlatform(int platform, bool alternate, bool mappedPlatform);

private:
    int m_platform;
    int m_animationStep;
    bool m_alternate;
};

#endif
