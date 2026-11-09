#ifndef GAME_UI_FJ_QUADRANTSIGNIN_HPP
#define GAME_UI_FJ_QUADRANTSIGNIN_HPP
#include "game/ui/FJ_Document.hpp"
class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;
class FJ_QuadrantSignin : public FJ_Document
{
public:
    explicit FJ_QuadrantSignin(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    static void _init(FJ_FuiNode* node, FJ_Event* event);
    void ShowControllerTimer(int controller, bool visible);
    void SetControllerStatus(int controller, int status, int frame);
    void ShowTimerAnimation(FJ_Event* event);
    static void _ShowTimerAnimation(FJ_FuiNode* node, FJ_Event* event);
    virtual void init(FJ_Event* event);
    void SetABSwap(bool swap);
    void JoinButtonPressed(int controller);

private:
    bool mABSwap;
    FJ_FuiNode* mControllers[4];
};
#endif
