#ifndef GAME_UI_FJ_DOCUMENT_HPP
#define GAME_UI_FJ_DOCUMENT_HPP

#include <string>
#include <vector>

#include "game/ui/FJ_FuiNode.hpp"

class FJ_Event;
class FJ_FuiNodeStage;
class fuiRenderNode;

class FJ_Document : public FJ_FuiNode
{
public:
    FJ_Document(fuiRenderNode* renderNode);
    virtual ~FJ_Document() {}

    int getISafeZoneTop();
    int getISafeZoneBottom();
    int getISafeZoneLeft();
    int getISafeZoneRight();
    static void _init(FJ_FuiNode* node, FJ_Event* event);
    void HandleOnFocusChangeInit(FJ_Event* event);
    static void _HandleOnFocusChangeInit(FJ_FuiNode* node, FJ_Event* event);
    void HandleOnFocusChangeUser(FJ_Event* event);
    static void _HandleOnFocusChangeUser(FJ_FuiNode* node, FJ_Event* event);
    static void _handlePress(FJ_FuiNode* node, FJ_Event* event);
    static void _keyDownHandler(FJ_FuiNode* node, FJ_Event* event);
    virtual void init(FJ_Event* event);
    void SetFocus(int id);
    virtual void SetSafeZone(int top, int bottom, int left, int right);
    virtual void handlePress(FJ_Event* event);
    virtual void keyDownHandler(FJ_Event* event);
    virtual void OnFocusChange(bool focused);
    FJ_FuiNode* GetTopObject(FJ_FuiNode* node);
    void RepositionItems(FJ_FuiNode* first, FJ_FuiNode* second, bool horizontal);
    void FixNavigation(FJ_FuiNode* node);
    void RemoveObject(const std::string& name, bool destroy);
    virtual void DoHorizontalResizeCheck();
    void ResetHorizontalResizeCheck();
    void SlideLeft();
    void SlideRight();
    int getFrameByLabel(const std::string& label);
    void SliderClosed();
    void SliderOpen();

protected:
    int m_safeZoneTop;
    int m_safeZoneBottom;
    int m_safeZoneLeft;
    int m_safeZoneRight;
    FJ_FuiNode* m_root;
    FJ_FuiNode* m_focus;
    int m_focusParentId;
    int m_focusChildId;
    bool m_focusChange;
};

#endif
