#include "game/ui/RenameWorldScreen.hpp"

#include <string>

#include "game/core/Button.hpp"
#include "game/core/EditBox.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/LevelStorageSource.hpp"
#include "game/core/StringUtils.hpp"

RenameWorldScreen::RenameWorldScreen(Screen* parentScreen, const std::wstring& levelId) : Screen()
{
    mEditBox = NULL;
    mParentScreen = parentScreen;
    mLevelId = levelId;
}

void RenameWorldScreen::tick()
{
    mEditBox->tick();
}

void RenameWorldScreen::init()
{
}

void RenameWorldScreen::removed()
{
}

void RenameWorldScreen::buttonClicked(Button* button)
{
    if (!button->mActive)
        return;

    if (button->mId != 1)
    {
        if (button->mId != 0)
            return;

        LevelStorageSource* levelSource = mMinecraft->getLevelSource();
        levelSource->renameLevel(mLevelId, trimString(mEditBox->getValue()));
    }

    mMinecraft->setScreen(mParentScreen);
}

void RenameWorldScreen::keyPressed(wchar_t key, int action)
{
    mEditBox->keyPressed(key, action);

    mButtons[0]->mActive = trimString(mEditBox->getValue()).size() != 0;

    if (key == 13)
        buttonClicked(mButtons[0]);
}

void RenameWorldScreen::mouseClicked(int x, int y, int button)
{
    Screen::mouseClicked(x, y, button);
    mEditBox->mouseClicked(x, y, button);
}

void RenameWorldScreen::render(int x, int y, float partialTick)
{
    Language* language = Language::getInstance();
    renderBackground();
    drawCenteredString(mFont, language->getElement(L"selectWorld.renameTitle"), mWidth / 2, mHeight / 4 - 40, 0xffffff);
    drawString(mFont, language->getElement(L"selectWorld.enterName"), mWidth / 2 - 100, 47, 0xa0a0a0);
    mEditBox->render();
    Screen::render(x, y, partialTick);
}
