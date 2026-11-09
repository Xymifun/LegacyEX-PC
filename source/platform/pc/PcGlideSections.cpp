#include "game/core/CConsoleMinecraftApp.hpp"
#include "platform/pc/PcGlideSections.hpp"
#include "game/core/StringIds.hpp"

#include <cstdio>
#include <set>

#include "game/client/audio/SoundEvents.hpp"
#include "game/command/CheckpointCommand.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GhostController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/ui/FJ_MultilistMenu.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/pc/PcGlide.hpp"
#include "platform/wiiu/CInput.hpp"

namespace
{
const int kFirstSectionId = 100;

std::vector<CheckpointRuleDefinition*>* Checkpoints()
{
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options == NULL || options->getRequiredGameRules() == NULL)
        return NULL;
    return options->getRequiredGameRules()->getCheckpoints();
}
}

UIScene_PcGlideSections::UIScene_PcGlideSections(int player, void*, UILayer* layer)
    : UIScene(player, layer), m_chosen(-1), m_armed(false)
{
    initialiseMovie();
    m_sections = Sections();
    const int current = CurrentSection(m_playerId);
    m_multiList.AddNewLabel(L"Go to a section (the run stops counting)", 1);
    for (unsigned int index = 0; index < m_sections.size(); ++index)
    {
        wchar_t label[64];
        if (index == 0)
            std::swprintf(label, 64, L"Section 1 (Start)");
        else
            std::swprintf(label, 64, L"Section %u", index + 1);
        std::wstring text(label);
        if (static_cast<int>(index) == current)
            text += L" - current";
        m_multiList.AddNewButton(text, kFirstSectionId + static_cast<int>(index), false);
    }
    if (current >= 0)
        m_multiList.HighlightItem(static_cast<unsigned int>(kFirstSectionId + current), true);
    FJ_MultilistMenu* menu = static_cast<FJ_MultilistMenu*>(m_movie->getRootNode()->getFuiNode());
    menu->DoVerticalResizeCheck();
    doHorizontalResizeCheck();
    menu->HideDescription();
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_PcGlideSections::~UIScene_PcGlideSections()
{
}

std::wstring UIScene_PcGlideSections::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"MultilistMenuSplit") : std::wstring(L"MultilistMenu");
}

unsigned int UIScene_PcGlideSections::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_multiList.setupControl(this, root, std::string("MultiList"));
    m_controls.push_back(&m_multiList);
    m_description.setupControl(this, root, std::string("Description"));
    m_controls.push_back(&m_description);
    return 1;
}

int UIScene_PcGlideSections::getSceneType()
{
    return EUIScene_PcGlideSections;
}

void UIScene_PcGlideSections::tick()
{
    UIScene::tick();
    if (!m_armed && !g_pInput->ButtonDown(m_playerId, EControllerActions_RestartRace))
        m_armed = true;
    if (m_chosen < 0)
        return;
    const int section = m_chosen;
    m_chosen = -1;
    if (static_cast<unsigned int>(section) < m_sections.size())
    {
        PcGlideSetPractice(m_playerId, true);
        GhostController* ghosts = Minecraft::GetInstance()->GetGhostController();
        if (ghosts != NULL)
            ghosts->PcCancelRecording(m_playerId);
        GoToCheckpoint(m_playerId, m_sections[section]);
        std::printf("[pc] glide: practice from section %d (checkpoint %d)\n", section + 1, m_sections[section]);
    }
    navigateBack();
}

void UIScene_PcGlideSections::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_PcGlideSections::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

void UIScene_PcGlideSections::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                          bool& handled)
{
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed && m_armed)
        {
            navigateBack();
            handled = true;
        }
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_PcGlideSections::handlePress(int, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (childId >= kFirstSectionId)
        m_chosen = childId - kFirstSectionId;
}

std::vector<int> UIScene_PcGlideSections::Sections()
{
    std::vector<int> sections;
    std::vector<CheckpointRuleDefinition*>* checkpoints = Checkpoints();
    if (checkpoints == NULL)
        return sections;
    std::set<unsigned int> sequences;
    for (unsigned int index = 0; index < checkpoints->size(); ++index)
    {
        CheckpointRuleDefinition* checkpoint = (*checkpoints)[index];
        if (checkpoint->hasSpawnPoint() && sequences.insert(checkpoint->getSequenceID()).second)
            sections.push_back(static_cast<int>(index));
    }
    return sections;
}

int UIScene_PcGlideSections::CurrentSection(int pad)
{
    std::vector<CheckpointRuleDefinition*>* checkpoints = Checkpoints();
    Minecraft* minecraft = Minecraft::GetInstance();
    if (checkpoints == NULL || pad < 0 || pad >= 4 || minecraft->mLocalPlayers[pad] == NULL)
        return -1;
    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[pad];
    MultiPlayerLevel* level = player->GetLevel();
    if (level == NULL || level->GetClientScoreboard() == NULL)
        return -1;
    int last = level->GetClientScoreboard()->GetLastCheckpointID(player->getSUID(), -1);
    if (last >= static_cast<int>(checkpoints->size()))
        last = static_cast<int>(checkpoints->size()) - 1;
    while (last > 0 && !(*checkpoints)[last]->hasSpawnPoint())
        --last;
    if (last < 0)
        return -1;
    const unsigned int sequence = (*checkpoints)[last]->getSequenceID();
    std::vector<int> sections = Sections();
    for (unsigned int index = 0; index < sections.size(); ++index)
    {
        if ((*checkpoints)[sections[index]]->getSequenceID() == sequence)
            return static_cast<int>(index);
    }
    return 0;
}

void UIScene_PcGlideSections::GoToCheckpoint(int pad, int checkpoint)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[pad];
    ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(pad);
    if (player == NULL || connection == NULL)
        return;
    connection->send(
        CheckpointCommand::preparePacket(player->getSUID(), checkpoint > 0 ? checkpoint : kPcCheckpointStartKeepTime));
    player->ResetPercentageTraversed();
    player->ResetThermalVariables();
}

void UIScene_PcGlideSections::BackTwoSections(int pad)
{
    const int current = CurrentSection(pad);
    std::vector<int> sections = Sections();
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[pad];
    ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(pad);
    if (player == NULL || connection == NULL)
        return;
    if (current > 0 && static_cast<unsigned int>(current) <= sections.size())
    {
        GoToCheckpoint(pad, sections[current - 1]);
        std::printf("[pc] glide: back from section %d to section %d\n", current + 1, current);
    }
    else
    {
        connection->send(CheckpointCommand::preparePacket(player->getSUID(), -1));
        player->ResetPercentageTraversed();
        player->ResetThermalVariables();
    }
}
