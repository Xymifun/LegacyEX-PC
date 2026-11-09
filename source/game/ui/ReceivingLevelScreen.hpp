#ifndef GAME_UI_RECEIVINGLEVELSCREEN_HPP
#define GAME_UI_RECEIVINGLEVELSCREEN_HPP

#include "game/ui/Screen.hpp"

class Button;
class ClientPacketListener;

class ReceivingLevelScreen : public Screen
{
public:
    ReceivingLevelScreen(ClientPacketListener* connection);

    virtual void keyPressed(char key, int action);
    virtual void init();
    virtual void tick();
    virtual void buttonClicked(Button* button);
    virtual void render(int x, int y, float partialTick);

private:
    ClientPacketListener* mConnection;
    int mTickCount;
};

#endif
