#ifndef GAME_UI_SCREEN_HPP
#define GAME_UI_SCREEN_HPP

#include <string>
#include <vector>

#include "game/core/GuiComponent.hpp"

class Button;
class Font;
class GuiParticles;
class Minecraft;

class Screen : public GuiComponent
{
public:
    Screen();
    virtual ~Screen() {}

    virtual void render(int x, int y, float partialTick);
    virtual void keyPressed(wchar_t key, int action);
    virtual void mouseClicked(int x, int y, int button);
    virtual void mouseReleased(int x, int y, int button);
    virtual void buttonClicked(Button* button);
    virtual void init(Minecraft* minecraft, int width, int height);
    virtual void init();
    virtual void setSize(int width, int height);
    virtual void updateEvents();
    virtual void mouseEvent();
    virtual void keyboardEvent();
    virtual void tick();
    virtual void removed();
    virtual void renderBackground();
    virtual void renderBackground(int shade);
    virtual void renderDirtBackground(int shade);
    virtual bool isPauseScreen();
    virtual void confirmResult(bool result, int id);
    virtual void tabPressed();

    static std::wstring getClipboard();
    static void setClipboard(const std::wstring& text);

    Minecraft* mMinecraft;
    int mWidth;
    int mHeight;
    std::vector<Button*> mButtons;
    bool mPassEvents;
    Font* mFont;
    GuiParticles* mGuiParticles;
    Button* mSelectedButton;
};

#endif
