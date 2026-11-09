#ifndef GAME_UI_CONNECTSCREEN_HPP
#define GAME_UI_CONNECTSCREEN_HPP

#include <string>

#include "game/ui/Screen.hpp"

class Button;
class ClientPacketListener;
class Minecraft;

class ConnectScreen : public Screen
{
public:
    ConnectScreen(Minecraft* minecraft, const std::wstring& address, int port);

    virtual void render(int x, int y, float partialTick);
    virtual void keyPressed(wchar_t key, int action);
    virtual void buttonClicked(Button* button);
    virtual void init();
    virtual void tick();

private:
    ClientPacketListener* mConnection;
    bool mFinished;
};

#endif
