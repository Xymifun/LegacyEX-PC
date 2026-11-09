#include "game/ui/UIScene_AchievementsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <stdint.h>
#include <cwchar>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/FJ_AchievementsMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/GameSettings.hpp"

const int UIScene_AchievementsMenu::sDisplayList[UIScene_AchievementsMenu::DISPLAY_COUNT] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 26,
    27, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75,
    76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
};

static const AchievementInfo s_achievementInfo[97] = {
    {0, L"MCTrophy_00", 1, (int)0xf28ac13d, (int)0x2d7b8efc},
    {1, L"MCTrophy_01", 2, (int)0x2ba9a566, (int)0x251faf0f},
    {2, L"MCTrophy_02", 3, (int)0xd5d0ceef, (int)0xaad1343e},
    {3, L"MCTrophy_10", 4, (int)0x77e6dd7c, (int)0xef6aa635},
    {4, L"MCTrophy_12", 5, (int)0x1f3264ae, (int)0x8fd1a9f7},
    {5, L"MCTrophy_14", 6, (int)0x36f66718, (int)0xacb5a871},
    {6, L"MCTrophy_03", 7, (int)0xcbf9c690, (int)0xb744b349},
    {7, L"MCTrophy_04", 8, (int)0xaaf2c159, (int)0xec03c5b8},
    {8, L"MCTrophy_05", 9, (int)0xd40e6602, (int)0xd051fccb},
    {9, L"MCTrophy_11", 10, (int)0x177692f7, (int)0x265eaf26},
    {10, L"MCTrophy_13", 11, (int)0x6c76f061, (int)0xfac6d4a0},
    {11, L"MCTrophy_15", 12, (int)0xcdfb2a93, (int)0x74ce7062},
    {12, L"MCTrophy_06", 13, (int)0xaaffe10b, (int)0x69596afa},
    {13, L"MCTrophy_07", 14, (int)0x75094a9c, (int)0xd7c1b555},
    {14, L"MCTrophy_08", 15, (int)0x088994e5, (int)0x89ac6b84},
    {15, L"MCTrophy_09", 16, (int)0x9a5947c5, (int)0x25d5bee4},
    {16, L"MCTrophy_16", 17, (int)0x9bef14ca, (int)0x20e37b33},
    {17, L"MCTrophy_17", 18, (int)0x1dc25b1d, (int)0x52a09fdc},
    {18, L"MCTrophy_18", 19, (int)0x83e5d954, (int)0xa2520ced},
    {19, L"MCTrophy_19", 20, (int)0x7a0bf856, (int)0x82303fdf},
    {20, L"", 0, (int)0x00000000, (int)0x00000000},
    {21, L"", 0, (int)0x00000000, (int)0x00000000},
    {22, L"MCTrophy_33", 34, (int)0x4d436e43, (int)0x50af2032},
    {23, L"MCTrophy_34", 35, (int)0x1f0da0ba, (int)0x709d6743},
    {24, L"MCTrophy_35", 36, (int)0xf4a12ad1, (int)0x35276db0},
    {25, L"", 0, (int)0x00000000, (int)0x00000000},
    {26, L"MCTrophy_20", 21, (int)0xdbdf2f5f, (int)0x28ac430e},
    {27, L"MCTrophy_21", 22, (int)0x491119a4, (int)0x6987c51d},
    {28, L"", 0, (int)0x00000000, (int)0x00000000},
    {29, L"MCTrophy_22", 23, (int)0xb0c864ed, (int)0x3eeb208c},
    {30, L"MCTrophy_23", 24, (int)0xab890ff2, (int)0x15bc7a1b},
    {31, L"MCTrophy_24", 25, (int)0xea3f287b, (int)0xb9be144a},
    {32, L"MCTrophy_25", 26, (int)0xbce10ac0, (int)0xd21802d9},
    {33, L"MCTrophy_26", 27, (int)0x22b4ed89, (int)0xb856de48},
    {34, L"MCTrophy_27", 28, (int)0xe61c41be, (int)0x13281da7},
    {35, L"MCTrophy_28", 29, (int)0xa43999c7, (int)0x2c6abd96},
    {36, L"MCTrophy_29", 30, (int)0x4c6c4ba7, (int)0xa43f9df6},
    {37, L"MCTrophy_30", 31, (int)0xb870791e, (int)0xde4e6187},
    {38, L"MCTrophy_31", 32, (int)0xe8bb9ab5, (int)0xa4159c74},
    {39, L"MCTrophy_32", 33, (int)0x80ab4f2c, (int)0xa53ccf85},
    {40, L"MCTrophy_36", 37, (int)0x9c9fdec8, (int)0xf3ac90c1},
    {41, L"MCTrophy_37", 38, (int)0xe60a7fbf, (int)0x81ba0f6e},
    {42, L"MCTrophy_38", 39, (int)0xddb1abb6, (int)0xc798ecbf},
    {43, L"MCTrophy_39", 40, (int)0xea1b0670, (int)0x6ce0f229},
    {44, L"MCTrophy_40", 41, (int)0x34a718b9, (int)0x1d2f8518},
    {45, L"MCTrophy_41", 42, (int)0xf30a1ce2, (int)0x49f3b52b},
    {46, L"MCTrophy_42", 43, (int)0x5a8f366b, (int)0x1ef5a9da},
    {47, L"MCTrophy_43", 44, (int)0x8bd39d54, (int)0x1e7130ed},
    {48, L"MCTrophy_44", 45, (int)0x25b01f1d, (int)0xcebfc3dc},
    {49, L"MCTrophy_45", 46, (int)0x205573c6, (int)0xb6677aef},
    {50, L"MCTrophy_46", 47, (int)0xde6986cf, (int)0x48c75f9e},
    {51, L"MCTrophy_47", 48, (int)0x3ee42b18, (int)0x28d4cc71},
    {52, L"MCTrophy_48", 49, (int)0x7464b461, (int)0x76e5f8a0},
    {53, L"MCTrophy_49", 50, (int)0xe42b6e41, (int)0xe7673b80},
    {54, L"MCTrophy_50", 51, (int)0xedf234f8, (int)0xb0fc22d1},
    {55, L"MCTrophy_51", 52, (int)0x1ea6a973, (int)0x39bb0442},
    {56, L"MCTrophy_52", 53, (int)0x81e7d32a, (int)0x0212b713},
    {57, L"MCTrophy_53", 54, (int)0x676ebe25, (int)0x2a1c3944},
    {58, L"MCTrophy_54", 55, (int)0x922534dc, (int)0x63c99615},
    {59, L"MCTrophy_55", 59, (int)0x3f0da9d0, (int)0x14931a09},
    {60, L"MCTrophy_56", 60, (int)0x416c6fd2, (int)0xbd23ce7b},
    {61, L"MCTrophy_57", 61, (int)0xa47cdb5b, (int)0x7744f12a},
    {62, L"MCTrophy_58", 62, (int)0xcdcf8120, (int)0xf3cbc339},
    {63, L"MCTrophy_59", 63, (int)0x344997e9, (int)0xdf4ea728},
    {64, L"MCTrophy_60", 64, (int)0xe59f6fb6, (int)0x43b810bf},
    {65, L"MCTrophy_61", 65, (int)0xedf843bf, (int)0xfdd9336e},
    {66, L"MCTrophy_62", 66, (int)0x51a9d184, (int)0x1d9d78fd},
    {67, L"MCTrophy_63", 67, (int)0x18ec08cd, (int)0x4bc26cec},
    {68, L"MCTrophy_64", 68, (int)0x26fb64ba, (int)0xecbc8b43},
    {69, L"MCTrophy_65", 69, (int)0x55313243, (int)0xccce4432},
    {70, L"MCTrophy_66", 70, (int)0xdb5aeb23, (int)0x18fb1412},
    {71, L"MCTrophy_67", 71, (int)0xd746a31a, (int)0x89c055a3},
    {72, L"MCTrophy_68", 72, (int)0x328dc131, (int)0x65a71910},
    {73, L"MCTrophy_69", 73, (int)0x5cdf6928, (int)0xa2799c21},
    {74, L"MCTrophy_70", 74, (int)0xbffd5c07, (int)0x86ac0e56},
    {75, L"MCTrophy_71", 75, (int)0xd1719bfe, (int)0x1191cc67},
    {76, L"MCTrophy_72", 76, (int)0x7c092995, (int)0xec3b5b54},
    {77, L"MCTrophy_73", 77, (int)0x3f07710c, (int)0x39e95e65},
    {78, L"MCTrophy_74", 78, (int)0x04e0a9bb, (int)0x45c5f50a},
    {79, L"MCTrophy_75", 79, (int)0x53bd0332, (int)0x9f49f1db},
    {80, L"MCTrophy_76", 80, (int)0x373fcdac, (int)0x778c8a05},
    {81, L"MCTrophy_77", 81, (int)0x9f501935, (int)0x766556f4},
    {82, L"MCTrophy_78", 82, (int)0x6f04f79e, (int)0xb09e1c07},
    {83, L"MCTrophy_79", 83, (int)0x0300ca27, (int)0x768f5876},
    {84, L"MCTrophy_80", 84, (int)0x53345d48, (int)0xc5fc4b41},
    {85, L"MCTrophy_81", 85, (int)0xab35a951, (int)0x07772830},
    {86, L"MCTrophy_82", 86, (int)0xd5a21f3a, (int)0x42ed21c3},
    {87, L"MCTrophy_83", 87, (int)0x03d7ecc3, (int)0x22fedab2},
    {88, L"MCTrophy_84", 88, (int)0x00508c04, (int)0x73ce0f7d},
    {89, L"MCTrophy_85", 89, (int)0xc792c34d, (int)0xa1f3036c},
    {90, L"MCTrophy_86", 90, (int)0x4173e15d, (int)0xfa479a9c},
    {91, L"MCTrophy_87", 91, (int)0xa7975f94, (int)0x763818ad},
    {92, L"MCTrophy_88", 92, (int)0xa66c8f0f, (int)0xc6edaf5e},
    {93, L"MCTrophy_89", 93, (int)0x6a8f8606, (int)0xe1103aaf},
    {94, L"MCTrophy_90", 94, (int)0x230988f9, (int)0xd075e4d8},
    {95, L"MCTrophy_91", 95, (int)0x39dc97b0, (int)0x4c6662e9},
    {96, L"MCTrophy_92", 97, (int)0x1ad380ab, (int)0x155d709a},
};

const AchievementInfo* UIScene_AchievementsMenu::GetInfo(int achievement)
{
    return &s_achievementInfo[achievement];
}

void UIScene_AchievementsMenu::registerTexture(int achievement)
{
    wchar_t path[64];
    swprintf(path, 64, L"Graphics\\Achievements\\%ls.png", GetInfo(achievement)->icon);
    if (gApp.hasArchiveFile(path, false))
    {
        arrayWithLength<unsigned char> file = gApp.getArchiveFile(path, false);
        if (file.length != 0 && file.data != NULL)
            registerSubstitutionTexture(GetInfo(achievement)->icon, file.data, file.length, true);
    }
}

UIScene_AchievementsMenu::UIScene_AchievementsMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer),
      m_mainPanel(),
      m_achievementsList(),
      m_titleLabel(),
      m_achievementName()
{
    initialiseMovie();
    m_achievementIndex = 0;
    m_showDescription = false;
    m_updateDetails = false;
    m_achievementsList.init(0);
    int selectedAchievement = (int)(uintptr_t)data;
    int selectedIndex = 0;
    for (int i = 0; i < DISPLAY_COUNT; ++i)
    {
        if (selectedAchievement == sDisplayList[i])
        {
            selectedIndex = i;
        }
        registerTexture(sDisplayList[i]);
    }
    m_titleLabel.init(UIString(IDS_ACHIEVEMENTS));
    m_achievementName.init(UIString(L""));
    for (int i = 0; i < DISPLAY_COUNT; ++i)
    {
        const AchievementInfo* info = GetInfo(sDisplayList[i]);
        m_achievementsList.AddNewItem(i, info->icon);
        m_achievementsList.EnableItem(i, gProfile->IsAwardsFlagSet(m_playerId, info->id), true);
    }
    m_achievementsList.setCurrentSelection(selectedIndex, true);
    updateTooltips();
    UpdateAchievementDetails();
}

UIScene_AchievementsMenu::~UIScene_AchievementsMenu()
{
}

void UIScene_AchievementsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(static_cast<unsigned int>(m_playerId), -1, IDS_TOOLTIPS_CANCEL,
                                     IDS_TOOLTIPS_SHOW_DESCRIPTION, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     false, false);
}

void UIScene_AchievementsMenu::SetAchievementDescription(std::vector<HtmlString>* lines)
{
    std::wstring description = HtmlString::Compose(lines);
    static_cast<FJ_AchievementsMenu*>(m_movie->getRootNode()->getFuiNode())->SetAchievementDescription(description);
}

void UIScene_AchievementsMenu::UpdateAchievementDetails()
{
    int achievementId = sDisplayList[m_achievementIndex];
    std::wstring title(L10N::GetString(GetInfo(achievementId)->title));
    m_achievementName.setLabel(UIString(title), false, false);
    std::vector<HtmlString>* lines = new std::vector<HtmlString>();
    if (m_showDescription)
    {
        std::wstring description(L10N::GetString(GetInfo(achievementId)->description));
        HtmlString line(description, eMinecraftColour_HTMLColor_White, false, false, 0);
        lines->push_back(line);
    }
    SetAchievementDescription(lines);
    delete lines;
}

unsigned int UIScene_AchievementsMenu::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_titleLabel.setupControl(this, currentNode, std::string("AcheivementsLabel"));
        m_controls.push_back(&m_titleLabel);
    }
    {
        m_achievementName.setupControl(this, currentNode, std::string("AchievementName"));
        m_controls.push_back(&m_achievementName);
    }
    {
        m_mainPanel.setupControl(this, currentNode, std::string("AchievementsListContainer"));
        m_controls.push_back(&m_mainPanel);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_mainPanel.getFuiRenderNode();
        {
            m_achievementsList.setupControl(this, currentNode, std::string("AchievementsList"));
            m_controls.push_back(&m_achievementsList);
        }
        currentNode = parentNode;
    }
    return 1;
}

int UIScene_AchievementsMenu::getSceneType()
{
    return EUIScene_AchievementsMenu;
}

void UIScene_AchievementsMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

std::wstring UIScene_AchievementsMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return L"AchievementsMenuSplit";
    return L"AchievementsMenu";
}

void UIScene_AchievementsMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                           bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    if (pressed)
    {
        switch (action)
        {
        case EControllerActions_MenuCancel:
            navigateBack();
            handled = true;
            break;
        case EControllerActions_MenuX:
            m_showDescription = !m_showDescription;
            m_updateDetails = true;
            break;
        case EControllerActions_MenuUp:
        case EControllerActions_MenuDown:
        case EControllerActions_MenuRight:
        case EControllerActions_MenuLeft:
        case EControllerActions_MenuPageUp:
        case EControllerActions_MenuPageDown:
            sendInputToMovie(action, repeat, pressed, released);
            handled = true;
            break;
        }
    }
    handled = true;
}

void UIScene_AchievementsMenu::handleGainFocus(bool hadFocus)
{
    m_updateDetails = true;
}

void UIScene_AchievementsMenu::handleFocusChange(int focus, int childFocus)
{
    m_achievementIndex = childFocus;
    m_updateDetails = true;
}

UIControl* UIScene_AchievementsMenu::GetMainPanel()
{
    return &m_mainPanel;
}

void UIScene_AchievementsMenu::handleReload()
{
    m_achievementsList.init(0);
    for (int i = 0; i < DISPLAY_COUNT; ++i)
    {
        registerTexture(sDisplayList[i]);
    }
    m_titleLabel.init(UIString(IDS_ACHIEVEMENTS));
    m_achievementName.init(UIString(L""));
    for (int i = 0; i < DISPLAY_COUNT; ++i)
    {
        const AchievementInfo* info = GetInfo(sDisplayList[i]);
        m_achievementsList.AddNewItem(i, info->icon);
        m_achievementsList.EnableItem(i, gProfile->IsAwardsFlagSet(m_playerId, info->id), true);
    }
    updateTooltips();
    UpdateAchievementDetails();
}

void UIScene_AchievementsMenu::tick()
{
    UIScene::tick();
    if (m_updateDetails)
    {
        UpdateAchievementDetails();
        m_updateDetails = false;
    }
}
