#include "game/ui/SelectWorldScreen.hpp"

#include <cstdio>
#include <string>

#include "game/core/Button.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/LevelStorageSource.hpp"
#include "game/ui/ConfirmScreen.hpp"
#include "game/ui/CreateWorldScreen.hpp"
#include "game/ui/RenameWorldScreen.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/world/LevelSummary.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/PlatformTiming.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

template <typename T> std::wstring _toString(T value);

SelectWorldScreen::SelectWorldScreen(Screen* parent) : Screen()
{
    mTitle = L"Select world";
    mSelecting = false;
    mSelectedWorld = 0;
    mWorldSelectionList = NULL;
    mConfirmingDelete = false;
    mRenameButton = NULL;
    mSelectButton = NULL;
    mDeleteButton = NULL;
    mParent = parent;
}

void SelectWorldScreen::loadLevelList()
{
    mLevelList = mMinecraft->getLevelSource()->getLevelList();
    mSelectedWorld = -1;
}

void SelectWorldScreen::init()
{
    Language* language = Language::getInstance();
    mTitle = language->getElement(L"selectWorld.title");
    mWorldName = language->getElement(L"selectWorld.world");
    mConversion = language->getElement(L"selectWorld.conversion");
    loadLevelList();
    mWorldSelectionList = new WorldSelectionList(this);
    mWorldSelectionList->init(&mButtons, 4, 5);
    postInit();
}

std::wstring SelectWorldScreen::getWorldId(int index)
{
    return mLevelList->at(index)->getLevelId();
}

std::wstring SelectWorldScreen::getWorldName(int index)
{
    std::wstring name = mLevelList->at(index)->getLevelName();
    if (name.empty())
    {
        Language* language = Language::getInstance();
        name = language->getElement(L"selectWorld.world") + L" " + _toString(index + 1);
    }
    return name;
}

void SelectWorldScreen::postInit()
{
    Language* language = Language::getInstance();
    mButtons.push_back(mSelectButton = new Button(1, mWidth / 2 - 154, mHeight - 52, 150, 20,
                                                  language->getElement(L"selectWorld.select")));
    mButtons.push_back(mRenameButton = new Button(6, mWidth / 2 - 154, mHeight - 28, 70, 20,
                                                  language->getElement(L"selectWorld.rename")));
    mButtons.push_back(mDeleteButton = new Button(2, mWidth / 2 - 74, mHeight - 28, 70, 20,
                                                  language->getElement(L"selectWorld.delete")));
    mButtons.push_back(
        new Button(3, mWidth / 2 + 4, mHeight - 52, 150, 20, language->getElement(L"selectWorld.create")));
    mButtons.push_back(new Button(0, mWidth / 2 + 4, mHeight - 28, 150, 20, language->getElement(L"gui.cancel")));
    mSelectButton->mActive = false;
    mRenameButton->mActive = false;
    mDeleteButton->mActive = false;
}

void SelectWorldScreen::worldSelected(int index)
{
    mMinecraft->setScreen(NULL);
    if (!mSelecting)
    {
        mSelecting = true;
        mMinecraft->mGameMode = NULL;
        std::wstring worldId = getWorldId(index);
        if (worldId == L"")
        {
            worldId = L"World" + _toString(index);
        }
    }
}

void SelectWorldScreen::buttonClicked(Button* button)
{
    if (!button->mActive)
        return;

    if (button->mId == 2)
    {
        std::wstring name = getWorldName(mSelectedWorld);
        if (name != L"")
        {
            mConfirmingDelete = true;
            Language* language = Language::getInstance();
            std::wstring message = language->getElement(L"selectWorld.deleteQuestion");
            std::wstring text = L"'" + name + L"' " + language->getElement(L"selectWorld.deleteWarning");
            std::wstring deleteLabel = language->getElement(L"selectWorld.deleteButton");
            std::wstring cancelLabel = language->getElement(L"gui.cancel");
            ConfirmScreen* confirm = new ConfirmScreen(this, message, text, deleteLabel, cancelLabel, mSelectedWorld);
            mMinecraft->setScreen(confirm);
        }
    }
    else if (button->mId == 1)
    {
        worldSelected(mSelectedWorld);
    }
    else if (button->mId == 3)
    {
        if (gProfile->IsFullVersion())
            mMinecraft->setScreen(new CreateWorldScreen(this));
        else
        {
            mMinecraft->setScreen(NULL);
            if (!mSelecting)
                mSelecting = true;
        }
    }
    else if (button->mId == 6)
    {
        mMinecraft->setScreen(new RenameWorldScreen(this, getWorldId(mSelectedWorld)));
    }
    else if (button->mId == 0)
    {
        mMinecraft->setScreen(mParent);
    }
    else
    {
        mWorldSelectionList->buttonClicked(button);
    }
}

void SelectWorldScreen::confirmResult(bool result, int id)
{
    if (mConfirmingDelete)
    {
        mConfirmingDelete = false;
        if (result)
        {
            LevelStorageSource* levelSource = mMinecraft->getLevelSource();
            levelSource->clearAll();
            levelSource->deleteLevel(getWorldId(id));
            loadLevelList();
        }
        mMinecraft->setScreen(this);
    }
}

void SelectWorldScreen::render(int x, int y, float partialTick)
{
    drawCenteredString(mFont, mTitle, mWidth / 2, 20, 0xffffff);
    Screen::render(x, y, partialTick);
    static int counter = 0;
    if (counter++ >= 100)
    {
        if (!mLevelList->empty())
        {
            if (mLevelList->size() > 1 && mDeleteButton->mActive)
            {
                mSelectedWorld = 1;
                counter = 0;
                buttonClicked(mDeleteButton);
                return;
            }
            if (mSelectButton->mActive)
            {
                mSelectedWorld = 0;
                buttonClicked(mSelectButton);
                return;
            }
            counter = 0;
            mSelectButton->mActive = true;
            mRenameButton->mActive = true;
            mDeleteButton->mActive = true;
            return;
        }
        mMinecraft->setScreen(new CreateWorldScreen(this));
    }
}

SelectWorldScreen::WorldSelectionList::WorldSelectionList(SelectWorldScreen* screen)
    : ScrolledSelectionList(screen->mMinecraft, screen->mWidth, screen->mHeight, 32, screen->mHeight - 64, 36),
      mScreen(screen)
{
}

int SelectWorldScreen::WorldSelectionList::getNumberOfItems()
{
    return (int)mScreen->mLevelList->size();
}

void SelectWorldScreen::WorldSelectionList::selectItem(int index, bool doubleClick)
{
    mScreen->mSelectedWorld = index;
    bool valid = index >= 0 && index < getNumberOfItems();
    mScreen->mSelectButton->mActive = valid;
    mScreen->mRenameButton->mActive = valid;
    mScreen->mDeleteButton->mActive = valid;
    if (doubleClick && valid)
        mScreen->worldSelected(index);
}

bool SelectWorldScreen::WorldSelectionList::isSelectedItem(int index)
{
    return index == mScreen->mSelectedWorld;
}

int SelectWorldScreen::WorldSelectionList::getMaxPosition()
{
    return (int)mScreen->mLevelList->size() * 36;
}

void SelectWorldScreen::WorldSelectionList::renderBackground()
{
    mScreen->renderBackground();
}

void SelectWorldScreen::WorldSelectionList::renderItem(int index, int x, int y, int height, Tesselator*)
{
    LevelSummary* summary = mScreen->mLevelList->at(index);
    std::wstring name = summary->getLevelName();
    if (name.empty())
        name = mScreen->mWorldName + L" " + _toString(index + 1);

    std::wstring id = summary->getLevelId();
    FILETIME fileTime;
    SYSTEMTIME systemTime;
    _LARGE_INTEGER lastPlayed;
    lastPlayed.QuadPart = summary->getLastPlayed() * 10000;
    fileTime.dwHighDateTime = lastPlayed.u.HighPart;
    fileTime.dwLowDateTime = lastPlayed.u.LowPart;
    FileTimeToSystemTime(&fileTime, &systemTime);
    wchar_t date[20];
    swprintf(date, 20, L"%d/%d/%d %d:%02d", systemTime.day, systemTime.month, systemTime.year, systemTime.hour,
             systemTime.minute);

    id = id + L" (" + date;
    long long size = summary->getSizeOnDisk();
    id = id + L", " + _toString((float)(size / 1024 * 100 / 1024) / 100.0f) + L" MB)";
    std::wstring conversion;
    if (summary->isRequiresConversion())
        conversion = mScreen->mConversion + L" " + conversion;

    mScreen->drawString(mScreen->mFont, name, x + 2, y + 1, 0xffffff);
    mScreen->drawString(mScreen->mFont, id, x + 2, y + 12, 0x808080);
    mScreen->drawString(mScreen->mFont, conversion, x + 2, y + 22, 0x808080);
}
