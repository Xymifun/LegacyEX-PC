#include "game/ui/UIScene_EndPoem.hpp"
#include "game/core/StringIds.hpp"

#include <cstring>
#include <cwchar>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/FJ_EndPoem.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/util/Random.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/core/StringUtils.hpp"
#include "game/client/render/Font.hpp"
#include "platform/wiiu/CProfile.hpp"

extern ConsoleUIController gConsoleUIController;

namespace
{
const int cParagraphBreakLength = 12;
}

UIScene_EndPoem::UIScene_EndPoem(int type, void*, UILayer* layer)
    : UIScene(type, layer),
      m_poemText(),
      m_noisyText(),
      m_noiseLengths(),
      m_labels()
{
    initialiseMovie();
    m_finished = false;

    m_poemText += L10N::GetString(IDS_WIN_TEXT);
    m_poemText += L10N::GetString(0x69414293);
    m_poemText += L10N::GetString(0x433ec82a);
    m_poemText += L" ";
    m_poemText = gApp.FormatHTMLString(m_playerId, m_poemText, static_cast<int>(0xff000000));

    Minecraft* minecraft = Minecraft::GetInstance();
    std::wstring playerName = L"";
    if (minecraft->mLocalPlayers[gConsoleUIController.GetWinUserIndex()] != NULL)
        playerName = escapeXML(minecraft->mLocalPlayers[gConsoleUIController.GetWinUserIndex()]->getDisplayName());
    else
        playerName = escapeXML(minecraft->mLocalPlayers[gProfile->GetPrimaryPad()]->getDisplayName());
    m_poemText = replaceAll(m_poemText, L"{*PLAYER*}", playerName);

    Random random(8124371LL);
    const wchar_t* noiseTag = L"{*NOISE*}";
    for (int position = m_poemText.find(noiseTag); position != -1; position = m_poemText.find(noiseTag, position + 1))
    {
        m_noiseLengths.push_back(random.nextInt(4) + 3);
    }

    updateNoise();

    m_labels = std::vector<std::wstring>();
    int start = 0;
    int next = 0;
    do
    {
        m_labels.push_back(m_noisyText.substr(start, next - start));
        start = next;
        next = m_noisyText.find(L"<br /><br />", next + cParagraphBreakLength, cParagraphBreakLength);
    } while (next != -1);
    m_labels.push_back(m_noisyText.substr(start, m_noisyText.length() - start));
    m_currentLabel = 0;
}

void UIScene_EndPoem::updateNoise()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    m_noisyText = m_poemText;
    std::wstring noise = L"";
    Random* random = minecraft->mFont->mRandom;
    std::wstring noiseTag = L"{*NOISE*}";
    std::vector<int>::iterator length = m_noiseLengths.begin();
    int position = m_noisyText.find(noiseTag);
    while (position != -1 && length != m_noiseLengths.end())
    {
        int count = *length++;
        noise = L"";
        for (int i = 0; i < count; ++i)
        {
            wchar_t character;
            if (gConsoleUIController.UsingBitmapFont())
            {
                character =
                    SharedConstants::ALLOWED_CHARACTERS[random->nextInt(SharedConstants::ALLOWED_CHARACTERS.length())];
            }
            else
            {
                static std::wstring noiseCharacters = L"!\"#$%&'()*+,-./0123456789:;<=>?@[\\]^_'|}~";
                character = noiseCharacters[random->nextInt(noiseCharacters.length())];
            }
            std::wstring characterText = L"";
            characterText.push_back(character);

            if (character == L'<')
                characterText = L"&lt;";
            else if (character == L'>')
                characterText = L"&gt;";
            else if (character == L'"')
                characterText = L"&quot;";
            else if (character == L'&')
                characterText = L"&amp;";
            else if (character == L'\\')
                characterText = L"\\\\";
            else if (character == L'{')
                characterText = L"}";

            int colourChoice = random->nextInt(2);
            eMinecraftColour colour = eMinecraftColour_HTMLColor_8;
            if (colourChoice == 1)
                colour = eMinecraftColour_HTMLColor_9;
            else if (colourChoice == 2)
                colour = eMinecraftColour_HTMLColor_a;

            wchar_t formatted[64];
            std::memset(formatted, 0, sizeof(formatted));
            swprintf(formatted, 64, L"<font color=\"#%08x\" shadowcolor=\"#80000000\">%ls</font>",
                     gApp.GetHTMLColour(colour), characterText.c_str());
            noise += formatted;
        }
        m_noisyText.replace(position, noiseTag.length(), noise);
        position = m_noisyText.find(noiseTag, position + 1);
    }
}

void UIScene_EndPoem::updateTooltips()
{
    int tooltip = m_finished ? -1 : static_cast<int>(IDS_TOOLTIPS_CONTINUE);
    gConsoleUIController.SetTooltips(m_playerId, -1, tooltip, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     false, true);
}

std::wstring UIScene_EndPoem::getMoviePath()
{
    return L"EndPoem";
}

void UIScene_EndPoem::tick()
{
    UIScene::tick();
    if (m_currentLabel >= 0 && static_cast<unsigned int>(m_currentLabel) < m_labels.size())
    {
        std::wstring label = m_labels[m_currentLabel];
        unsigned int labelCount = m_labels.size();
        FJ_EndPoem* poem = static_cast<FJ_EndPoem*>(m_movie->getRootNode()->getFuiNode());
        poem->SetNextLabel(label, m_currentLabel, m_currentLabel == labelCount - 1);
        m_currentLabel = -1;
    }
}

void UIScene_EndPoem::handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    if (m_finished)
        return;
    if (pressed)
        gConsoleUIController.AnimateKeyPress(playerId, action, repeat, pressed, released);

    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            m_finished = true;
            Minecraft* minecraft = Minecraft::GetInstance();
            for (int i = 0; i < 4; ++i)
            {
                if (minecraft->mLocalPlayers[i] != NULL)
                    gApp.SetAction(i, eXuiAction_Respawn, NULL);
            }
            if (minecraft->mGameModes[gProfile->GetPrimaryPad()] != NULL)
            {
                Tutorial* tutorial = minecraft->mGameModes[gProfile->GetPrimaryPad()]->getTutorial();
                if (tutorial != NULL)
                    tutorial->showTutorialPopup(true);
            }
            updateTooltips();
            navigateBack();
            handled = true;
        }
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_EndPoem::handleDestroy()
{
}

void UIScene_EndPoem::handleRequestMoreData(int id, bool)
{
    m_currentLabel = id;
}

int UIScene_EndPoem::getSceneType()
{
    return EUIScene_EndPoem;
}
