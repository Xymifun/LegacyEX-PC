#include "game/ui/UIScene_Timer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/UILayer.hpp"

UIScene_Timer::UIScene_Timer(int type, void* data, UILayer* layer) : UIScene(type, layer)
{
    initialiseMovie();
    if (data == 0)
        m_control.setVisible(false);
}

void UIScene_Timer::reloadMovie(bool)
{
}

bool UIScene_Timer::needsReloaded()
{
    return false;
}

std::wstring UIScene_Timer::getMoviePath()
{
    return std::wstring(L"Timer");
}

unsigned int UIScene_Timer::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_control.setupControl(this, currentNode, std::string("Background"));
        m_controls.push_back(&m_control);
    }
    return 1;
}

int UIScene_Timer::getSceneType()
{
    return EUIScene_Timer;
}

bool UIScene_Timer::hidesLowerScenes()
{
    return true;
}
