#include "game/ui/UIComponent_DebugUIMarketingGuide.hpp"
#include "game/ui/UILayer.hpp"

UIComponent_DebugUIMarketingGuide::UIComponent_DebugUIMarketingGuide(int type, void*, UILayer* layer)
    : UIScene(type, layer)
{
    initialiseMovie();
}

std::wstring UIComponent_DebugUIMarketingGuide::getMoviePath()
{
    return std::wstring(L"DebugUIMarketingGuide");
}

int UIComponent_DebugUIMarketingGuide::getSceneType()
{
    return EUIScene_DebugUIMarketingGuide;
}

bool UIComponent_DebugUIMarketingGuide::stealsFocus()
{
    return false;
}

bool UIComponent_DebugUIMarketingGuide::hasFocus(int)
{
    return false;
}

bool UIComponent_DebugUIMarketingGuide::hidesLowerScenes()
{
    return false;
}
