#include "game/ui/CreateWorldScreen.hpp"

#include <string>
#include <sstream>

#include "game/core/Button.hpp"
#include "game/core/EditBox.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/LevelStorageSource.hpp"
#include "game/util/Random.hpp"
#include "game/core/StringUtils.hpp"

template <typename T>
T _fromString(const std::wstring& value);

extern const wchar_t illegalFileCharacters[15] = {L'/', L'\n', L'\r', L'\t', L'\0', L'\f', L'`', L'?',
                                                  L'*', L'\\', L'<',  L'>',  L'|',  L'\"', L':'};

CreateWorldScreen::CreateWorldScreen(Screen* parentScreen)
{
    mCreatingWorld = false;
    mParentScreen = parentScreen;
}

void CreateWorldScreen::tick()
{
    mNameEdit->tick();
    mSeedEdit->tick();
    static int tickCount = 0;
    if (tickCount++ == 100)
    {
        buttonClicked(mButtons[0]);
    }
}

std::wstring CreateWorldScreen::findAvailableFolderName(LevelStorageSource* levelSource, const std::wstring& folderName)
{
    static_cast<void>(levelSource);
    std::wstring name = folderName;
    return name;
}

void CreateWorldScreen::updateResultFolder()
{
    mResultFolder = trimString(mNameEdit->getValue());

    for (int i = 0; i < 15; ++i)
    {
        std::wstring::size_type position;
        while ((position = mResultFolder.find(illegalFileCharacters[i])) != std::wstring::npos)
        {
            mResultFolder[position] = L'_';
        }
    }

    if (mResultFolder.empty())
    {
        mResultFolder = L"World";
    }
    mResultFolder = findAvailableFolderName(mMinecraft->getLevelSource(), mResultFolder);
}

void CreateWorldScreen::init()
{
    Language* language = Language::getInstance();
    mButtons.clear();
    mButtons.push_back(new Button(0, mWidth / 2 - 100, mHeight / 4 + 108, language->getElement(L"selectWorld.create")));
    mButtons.push_back(new Button(1, mWidth / 2 - 100, mHeight / 4 + 132, language->getElement(L"gui.cancel")));

    mNameEdit = new EditBox(this, mFont, mWidth / 2 - 100, 60, 200, 20, language->getElement(L"testWorld"));
    mNameEdit->mFocused = true;
    mNameEdit->setMaxLength(32);
    mSeedEdit = new EditBox(this, mFont, mWidth / 2 - 100, 116, 200, 20, L"");
    updateResultFolder();
}

void CreateWorldScreen::removed()
{
}

void CreateWorldScreen::buttonClicked(Button* button)
{
    if (button->mActive)
    {
        if (button->mId == 1)
        {
            mMinecraft->setScreen(mParentScreen);
            return;
        }
        if (button->mId == 0)
        {
            mMinecraft->setScreen(0);
            if (!mCreatingWorld)
            {
                mCreatingWorld = true;
                Random* random = new Random();
                random->nextLong();
                std::wstring seedText = mSeedEdit->getValue();
                if (!seedText.empty())
                {
                    _fromString<long long>(seedText);
                }
            }
        }
    }
}

void CreateWorldScreen::keyPressed(wchar_t key, int action)
{
    if (mNameEdit->mFocused)
    {
        mNameEdit->keyPressed(key, action);
    }
    else
    {
        mSeedEdit->keyPressed(key, action);
    }
    if (key == 13)
    {
        buttonClicked(mButtons[0]);
    }
    mButtons[0]->mActive = mNameEdit->getValue().size() != 0;
    updateResultFolder();
}

void CreateWorldScreen::mouseClicked(int x, int y, int button)
{
    Screen::mouseClicked(x, y, button);
    mNameEdit->mouseClicked(x, y, button);
    mSeedEdit->mouseClicked(x, y, button);
}

void CreateWorldScreen::render(int x, int y, float partialTick)
{
    Language* language = Language::getInstance();
    renderBackground();
    drawCenteredString(mFont, language->getElement(L"selectWorld.create"), mWidth / 2, mHeight / 4 - 40, 0xffffff);
    drawString(mFont, language->getElement(L"selectWorld.enterName"), mWidth / 2 - 100, 47, 0xa0a0a0);
    drawString(mFont, language->getElement(L"selectWorld.resultFolder") + L" " + mResultFolder, mWidth / 2 - 100, 85,
               0xa0a0a0);
    drawString(mFont, language->getElement(L"selectWorld.enterSeed"), mWidth / 2 - 100, 104, 0xa0a0a0);
    drawString(mFont, language->getElement(L"selectWorld.seedInfo"), mWidth / 2 - 100, 140, 0xa0a0a0);
    mNameEdit->render();
    mSeedEdit->render();
    Screen::render(x, y, partialTick);
}

void CreateWorldScreen::tabPressed()
{
    if (mNameEdit->mFocused)
    {
        mNameEdit->focus(false);
        mSeedEdit->focus(true);
    }
    else
    {
        mNameEdit->focus(true);
        mSeedEdit->focus(false);
    }
}
