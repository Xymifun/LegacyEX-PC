#include "game/ui/FJ_Intro.hpp"
#include "game/ui/UIScene.hpp"

#include <string>
#include <vector>

namespace
{
std::vector<std::string> g_0x104F357C[11];
bool g_0x104C6614 = true;
}

FJ_Intro::FJ_Intro(fuiRenderNode* renderNode)
    : FJ_Document(renderNode),
      m_platform(0),
      m_animationStep(0),
      m_alternate(false)
{
    if (g_0x104C6614)
    {
        g_0x104F357C[0].push_back("ESRB");
        g_0x104F357C[0].push_back("Mojang");
        g_0x104F357C[0].push_back("4J");

        g_0x104F357C[1].push_back("ESRB");
        g_0x104F357C[1].push_back("XboxLive");
        g_0x104F357C[1].push_back("Microsoft");
        g_0x104F357C[1].push_back("Mojang");
        g_0x104F357C[1].push_back("4J");

        g_0x104F357C[2].push_back("ESRB");
        g_0x104F357C[2].push_back("Microsoft");
        g_0x104F357C[2].push_back("Mojang");
        g_0x104F357C[2].push_back("4J");

        g_0x104F357C[3].push_back("ESRB");
        g_0x104F357C[3].push_back("Mojang");
        g_0x104F357C[3].push_back("4J");

        g_0x104F357C[4].push_back("ESRB");
        g_0x104F357C[4].push_back("Mojang");
        g_0x104F357C[4].push_back("4J");

        g_0x104F357C[5].push_back("ESRB");
        g_0x104F357C[5].push_back("Mojang");
        g_0x104F357C[5].push_back("4J");

        g_0x104F357C[6].push_back("ESRB");
        g_0x104F357C[6].push_back("Mojang");
        g_0x104F357C[6].push_back("4J");

        g_0x104F357C[7].push_back("ESRB");
        g_0x104F357C[7].push_back("Mojang");
        g_0x104F357C[7].push_back("4J");

        g_0x104F357C[8].push_back("ESRB");
        g_0x104F357C[8].push_back("Chinese_Health_Warning");
        g_0x104F357C[8].push_back("Mojang");
        g_0x104F357C[8].push_back("4J");
        g_0x104F357C[8].push_back("Chinese_Logo_XboxOne");

        g_0x104F357C[9].push_back("ESRB");
        g_0x104F357C[9].push_back("Chinese_Health_Warning");
        g_0x104F357C[9].push_back("Mojang");
        g_0x104F357C[9].push_back("4J");
        g_0x104F357C[9].push_back("Chinese_Logo_PS4");

        g_0x104F357C[10].push_back("ESRB");
        g_0x104F357C[10].push_back("Chinese_Health_Warning");
        g_0x104F357C[10].push_back("Mojang");
        g_0x104F357C[10].push_back("4J");
        g_0x104F357C[10].push_back("Chinese_Logo_PSVita");

        g_0x104C6614 = false;
    }
}

FJ_Document* FJ_Intro::Create(fuiRenderNode* renderNode)
{
    return new FJ_Intro(renderNode);
}

unsigned int FJ_Intro::getType()
{
    return 7;
}

void FJ_Intro::handleAnimationStep(FJ_Event*)
{
    ++m_animationStep;
    if (static_cast<unsigned int>(m_animationStep) >= g_0x104F357C[m_platform].size())
    {
        m_pCallbackScene->handleAnimationEnd();
    }
    else
    {
        gotoAndPlay(g_0x104F357C[m_platform][m_animationStep]);
    }
}

void FJ_Intro::_handleAnimationStep(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Intro*>(node)->handleAnimationStep(event);
}

void FJ_Intro::init(FJ_Event* event)
{
    FJ_Document::init(event);
    addEventListener(eFJEventType_AnimationStep, this, &_handleAnimationStep, false);
}

void FJ_Intro::SetIntroPlatform(int platform, bool alternate, bool mappedPlatform)
{
    m_animationStep = alternate;
    m_platform = platform;
    if (mappedPlatform)
    {
        if (m_platform == 2)
        {
            m_platform = 8;
        }
        else if (m_platform == 4)
        {
            m_platform = 9;
        }
        else if (m_platform == 5)
        {
            m_platform = 10;
        }
    }
    gotoAndPlay(g_0x104F357C[m_platform][m_animationStep]);
}
