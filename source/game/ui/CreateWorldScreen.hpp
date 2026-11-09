#ifndef GAME_UI_CREATEWORLDSCREEN_HPP
#define GAME_UI_CREATEWORLDSCREEN_HPP

#include <string>

#include "game/ui/Screen.hpp"

class EditBox;
class LevelStorageSource;

class CreateWorldScreen : public Screen
{
public:
    CreateWorldScreen(Screen* parentScreen);

    virtual void render(int x, int y, float partialTick);
    virtual void keyPressed(wchar_t key, int action);
    virtual void mouseClicked(int x, int y, int button);
    virtual void buttonClicked(Button* button);
    virtual void init();
    virtual void tick();
    virtual void removed();
    virtual void tabPressed();

    static std::wstring findAvailableFolderName(LevelStorageSource* levelSource, const std::wstring& folderName);

    void updateResultFolder();

private:
    Screen* mParentScreen;
    EditBox* mNameEdit;
    EditBox* mSeedEdit;
    std::wstring mResultFolder;
    bool mCreatingWorld;
};

#endif
