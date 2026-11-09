#include "game/ui/UIComponent_DebugUIConsole.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/UILayer.hpp"

UIComponent_DebugUIConsole::UIComponent_DebugUIConsole(int sceneType, void*, UILayer* layer) : UIScene(sceneType, layer)
{
    initialiseMovie();
    m_updateLabels = false;
}

unsigned int UIComponent_DebugUIConsole::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();

    {
        m_consoleLines[0].setupControl(this, root, std::string("consoleLine1"));
        m_controls.push_back(&m_consoleLines[0]);
    }
    {
        m_consoleLines[1].setupControl(this, root, std::string("consoleLine2"));
        m_controls.push_back(&m_consoleLines[1]);
    }
    {
        m_consoleLines[2].setupControl(this, root, std::string("consoleLine3"));
        m_controls.push_back(&m_consoleLines[2]);
    }
    {
        m_consoleLines[3].setupControl(this, root, std::string("consoleLine4"));
        m_controls.push_back(&m_consoleLines[3]);
    }
    {
        m_consoleLines[4].setupControl(this, root, std::string("consoleLine5"));
        m_controls.push_back(&m_consoleLines[4]);
    }
    {
        m_consoleLines[5].setupControl(this, root, std::string("consoleLine6"));
        m_controls.push_back(&m_consoleLines[5]);
    }
    {
        m_consoleLines[6].setupControl(this, root, std::string("consoleLine7"));
        m_controls.push_back(&m_consoleLines[6]);
    }
    {
        m_consoleLines[7].setupControl(this, root, std::string("consoleLine8"));
        m_controls.push_back(&m_consoleLines[7]);
    }
    {
        m_consoleLines[8].setupControl(this, root, std::string("consoleLine9"));
        m_controls.push_back(&m_consoleLines[8]);
    }
    {
        m_consoleLines[9].setupControl(this, root, std::string("consoleLine10"));
        m_controls.push_back(&m_consoleLines[9]);
    }
    return 1;
}

void UIComponent_DebugUIConsole::tick()
{
    UIScene::tick();
    if (!m_updateLabels)
        return;

    m_updateLabels = false;
    for (unsigned int index = 0; index < 10 && index < m_text.size(); ++index)
        m_consoleLines[index].setLabel(UIString(*(m_text.begin() + index)), false, false);
}

std::wstring UIComponent_DebugUIConsole::getMoviePath()
{
    return std::wstring(L"DebugUIConsoleComponent");
}

int UIComponent_DebugUIConsole::getSceneType()
{
    return EUIScene_DebugUIConsole;
}

bool UIComponent_DebugUIConsole::stealsFocus()
{
    return false;
}

bool UIComponent_DebugUIConsole::hasFocus(int)
{
    return false;
}

bool UIComponent_DebugUIConsole::hidesLowerScenes()
{
    return false;
}
