#include "game/ui/ReceivingLevelScreen.hpp"

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/core/Language.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ClientboundKeepAlivePacket.hpp"

void ReceivingLevelScreen::keyPressed(char key, int action)
{
}

void ReceivingLevelScreen::init()
{
    mButtons.clear();
}

void ReceivingLevelScreen::tick()
{
    ++mTickCount;

    if (mTickCount % 20 == 0)
    {
        boost::shared_ptr<ClientboundKeepAlivePacket> packet(new ClientboundKeepAlivePacket());
        mConnection->send(packet);
    }

    if (mConnection != NULL)
    {
        mConnection->tick();
    }
}

void ReceivingLevelScreen::buttonClicked(Button* button)
{
}

void ReceivingLevelScreen::render(int x, int y, float partialTick)
{
    renderDirtBackground(0);

    Language* language = Language::getInstance();
    drawCenteredString(mFont, language->getElement(L"multiplayer.downloadingTerrain"), mWidth / 2, mHeight / 2 - 50,
                       0xffffff);

    Screen::render(x, y, partialTick);
}
