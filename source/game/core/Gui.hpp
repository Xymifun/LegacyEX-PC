#ifndef GAME_CORE_GUI_HPP
#define GAME_CORE_GUI_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/core/GuiComponent.hpp"
#include "game/core/GuiMessage.hpp"

class BossHealthOverlay;
class ItemRenderer;
class Minecraft;
class Random;
class ThermalAreaRuleDefinition;
class UIComponent_Chat;
class UIScene_HUD;

class Gui : public GuiComponent
{
    friend class UIComponent_Chat;
    friend class UIScene_DeathMenu;
    friend class UIScene_HUD;

public:
    explicit Gui(Minecraft* minecraft);

    BossHealthOverlay* getBossOverlay(int index);
    void renderVignette(float partialTick, int width, int height);
    void renderPumpkin(int width, int height);
    void renderPortalOverlay(float portalTime, int width, int height);
    void renderThermalOverlay(ThermalAreaRuleDefinition* rule, float partialTick, int width, int height, float alpha);
    void render(float partialTick, bool hideGui, int width, int height);
    void renderSlot(int playerIndex, int x, int y, float partialTick);
    void tick();
    void clearMessages(int playerIndex);
    void addMessage(const std::wstring& message, int playerIndex, bool overlay, bool filtered, bool append, bool force);
    void clearSingleMessages();
    void clearSingleMessages(int playerIndex);
    void setSingleMessage(const std::wstring& message, int playerIndex, int duration);
    void setNowPlaying(const std::wstring& message);
    void displayClientMessage(int stringId, int playerIndex);
    void onDisconnected();
    float getOpacity(int playerIndex, unsigned int messageIndex);
    float getJukeboxOpacity(int playerIndex);
    void ShowCharacterDisplay(int playerIndex);
    Minecraft* GetMinecraft();
    void DrawString(const std::wstring& text, int x, int y, int color);
#ifdef MINECRAFT_PC
    void renderChat(int playerIndex, int screenHeight);
    void renderScriptHud(float scaleX, float scaleY);
#endif

private:
    ItemRenderer* mItemRenderer;
    std::vector<GuiMessage> mMessages[4];
    std::wstring mNowPlaying[4];
    Random* mRandom;
    Minecraft* mMinecraft;
    int mUnknownD0;
    short mUnknownD4;
    short mUnknownD6;
    int mUnknownD8;
    int mUnknownDC;
    int mUnknownE0;
    int mUnknownE4;
    int mUnknownE8;
    int mUnknownEC;
    int mTickCount;
    std::wstring mSingleMessages[4];
    int mSingleMessageTime[4];
    bool mSingleMessageVisible;
    BossHealthOverlay* mBossOverlays[4];
    float mJukeboxTime;
    float mUnknown19C;
    ThermalAreaRuleDefinition* mThermalOverlayRules[4];
    int mThermalOverlayFade[4];
    int mCharacterDisplayTime[4];
    float mUnknown1D0;
    float mVignetteBrightness;
};

#endif
