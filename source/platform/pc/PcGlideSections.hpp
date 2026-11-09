#ifndef PLATFORM_PC_PCGLIDESECTIONS_HPP
#define PLATFORM_PC_PCGLIDESECTIONS_HPP

#include <string>
#include <vector>

#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "game/ui/UIScene.hpp"

class UIScene_PcGlideSections : public UIScene
{
public:
    UIScene_PcGlideSections(int player, void* data, UILayer* layer);
    virtual ~UIScene_PcGlideSections();

    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void tick();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);

    static std::vector<int> Sections();
    static int CurrentSection(int pad);
    static void GoToCheckpoint(int pad, int checkpoint);
    static void BackTwoSections(int pad);

private:
    UIControl_MultiList m_multiList;
    UIControl_HTMLLabel m_description;
    std::vector<int> m_sections;
    int m_chosen;
    bool m_armed;
};

#endif
