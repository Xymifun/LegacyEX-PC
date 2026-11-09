#ifndef GAME_UI_SELECTWORLDSCREEN_HPP
#define GAME_UI_SELECTWORLDSCREEN_HPP

#include <string>
#include <vector>

#include "game/ui/Screen.hpp"
#include "game/core/ScrolledSelectionList.hpp"

class Button;
class LevelSummary;
class Tesselator;

class SelectWorldScreen : public Screen
{
public:
    class WorldSelectionList : public ScrolledSelectionList
    {
    public:
        WorldSelectionList(SelectWorldScreen* screen);

        virtual int getNumberOfItems();
        virtual void selectItem(int index, bool doubleClick);
        virtual bool isSelectedItem(int index);
        virtual int getMaxPosition();
        virtual void renderBackground();
        virtual void renderItem(int index, int x, int y, int height, Tesselator* tesselator);

    private:
        SelectWorldScreen* mScreen;
    };

    SelectWorldScreen(Screen* parent);

    virtual void render(int x, int y, float partialTick);
    virtual void buttonClicked(Button* button);
    virtual void init();
    virtual void confirmResult(bool result, int id);

    void loadLevelList();
    std::wstring getWorldId(int index);
    std::wstring getWorldName(int index);
    virtual void postInit();
    void worldSelected(int index);

private:
    Screen* mParent;
    std::wstring mTitle;
    bool mSelecting;
    int mSelectedWorld;
    std::vector<LevelSummary*>* mLevelList;
    WorldSelectionList* mWorldSelectionList;
    std::wstring mWorldName;
    std::wstring mConversion;
    bool mConfirmingDelete;
    Button* mRenameButton;
    Button* mSelectButton;
    Button* mDeleteButton;
};

#endif
