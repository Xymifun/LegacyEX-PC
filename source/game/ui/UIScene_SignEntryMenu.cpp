#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/ui/UIScene_SignEntryMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cstring>
#include <string>

#include "game/core/CMinecraftApp.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/UIControl_Base.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UIString.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerboundSignUpdatePacket.hpp"
#include "game/world/block/SignBlockEntity.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"
#include "game/core/GameSettings.hpp"

UIScene_SignEntryMenu::UIScene_SignEntryMenu(int type, void* data, UILayer* layer) : UIScene(type, layer), m_sign()
{
    initialiseMovie();
    m_sign = *static_cast<boost::shared_ptr<SignBlockEntity>*>(data);
    m_confirmRequested = false;
    m_keyboardActive = false;

    m_confirmButton.init(UIString(L10N::GetString(IDS_DONE)), 4);
    m_messageLabel.init(UIString(L10N::GetString(IDS_EDIT_SIGN_MESSAGE)));

    for (int i = 0; i < 4; ++i)
    {
        std::wstring message(m_sign.get()->mMessages[i]);
        m_lineInputs[i].init(UIString(message.c_str()), i);
    }

    layer->addComponent(type, EUIScene_MenuBackground, NULL);
}

UIScene_SignEntryMenu::~UIScene_SignEntryMenu()
{
    m_layer->removeComponent(EUIScene_MenuBackground);
}

unsigned int UIScene_SignEntryMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_confirmButton.setupControl(this, currentNode, std::string("Confirm"));
        m_controls.push_back(&m_confirmButton);
    }
    {
        m_messageLabel.setupControl(this, currentNode, std::string("Message"));
        m_controls.push_back(&m_messageLabel);
    }
    {
        m_lineInputs[0].setupControl(this, currentNode, std::string("Line1"));
        m_controls.push_back(&m_lineInputs[0]);
    }
    {
        m_lineInputs[1].setupControl(this, currentNode, std::string("Line2"));
        m_controls.push_back(&m_lineInputs[1]);
    }
    {
        m_lineInputs[2].setupControl(this, currentNode, std::string("Line3"));
        m_controls.push_back(&m_lineInputs[2]);
    }
    {
        m_lineInputs[3].setupControl(this, currentNode, std::string("Line4"));
        m_controls.push_back(&m_lineInputs[3]);
    }
    return 1;
}

void UIScene_SignEntryMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, (int)IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_SignEntryMenu::tick()
{
    UIScene::tick();

    if (!m_confirmRequested)
        return;

    m_confirmRequested = false;
    for (int i = 0; i < 4; ++i)
    {
        std::wstring line(m_lineInputs[i].getLabel());
        m_sign.get()->SetMessage(i, line);
    }
    m_sign.get()->setChanged();

    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLevel->mIsClientSide)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[m_playerId];
        if (player.get() != NULL && player.get()->mPacketListener != NULL && player.get()->mPacketListener->isStarted())
        {
            BlockPos pos = m_sign.get()->getBlockPos();
            boost::shared_ptr<Packet> packet(
                new ServerboundSignUpdatePacket(pos, m_sign.get()->mMessages, gApp.IsConsoleChatCensored()));
            player.get()->mPacketListener->send(packet);
        }
    }

    gConsoleUIController.CloseUIScenes(m_playerId, false);
}

std::wstring UIScene_SignEntryMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return std::wstring(L"SignEntryMenuSplit");
    return std::wstring(L"SignEntryMenu");
}

void UIScene_SignEntryMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                        bool& handled)
{
    if (!m_confirmRequested && !m_keyboardActive)
    {
        gConsoleUIController.AnimateKeyPress(playerId, action, repeat, pressed, released);
        switch (action)
        {
        case EControllerActions_MenuCancel:
            if (pressed)
            {
                std::wstring empty(L"");
                m_sign.get()->SetMessage(0, empty);
                m_sign.get()->SetMessage(1, empty);
                m_sign.get()->SetMessage(2, empty);
                m_sign.get()->SetMessage(3, empty);
                navigateBack();
                gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
                handled = true;
            }
            break;
        case EControllerActions_MenuUp:
        case EControllerActions_MenuDown:
        case EControllerActions_MenuOk:
            sendInputToMovie(action, repeat, pressed, released);
            handled = true;
            break;
        }
    }
}

int UIScene_SignEntryMenu::KeyboardCompleteCallback(void* data, bool accepted)
{
    UIScene_SignEntryMenu* scene = static_cast<UIScene_SignEntryMenu*>(data);
    if (scene == NULL)
        return 0;

    scene->m_keyboardActive = false;
    if (accepted)
    {
        wchar_t text[128];
        std::memset(text, 0, sizeof(text));
        g_pInput->GetText(text, 0x7f);
        scene->m_lineInputs[scene->m_selectedLine].setLabel(UIString(text), false, false);
    }
    return 0;
}

void UIScene_SignEntryMenu::handlePress(int controlId, int)
{
    switch (controlId)
    {
    case 4:
        m_confirmRequested = true;
        break;
    case 0:
    case 1:
    case 2:
    case 3:
        m_selectedLine = controlId;
        m_keyboardActive = true;
        if (g_pInput->RequestKeyboard(L10N::GetString(IDS_SIGN_TITLE), m_lineInputs[m_selectedLine].getLabel(),
                                      m_playerId, 0xf, KeyboardCompleteCallback, this,
                                      static_cast<C_4JInput::EKeyboardMode>(3))
            != 0)
            m_keyboardActive = false;
        break;
    }
}

void UIScene_SignEntryMenu::handleDestroy()
{
    g_pInput->DestroyKeyboard();
}

int UIScene_SignEntryMenu::getSceneType()
{
    return EUIScene_SignEntryMenu;
}
