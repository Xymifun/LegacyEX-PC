#ifndef GAME_UI_UISCENE_SUPERFLATPRESETS_HPP
#define GAME_UI_UISCENE_SUPERFLATPRESETS_HPP

#include <string>
#include <vector>

#include "platform/wiiu/C4JStorage.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_LeftIconButtonList.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"

class SuperflatConfig;
class UILayer;
struct fuiRect;

class UIScene_SuperFlatPresets : public UIScene
{
public:
    UIScene_SuperFlatPresets(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_SuperFlatPresets();

    virtual unsigned int mapElementsAndNames();
    int GetPresetCount() const;
    int SafeGetIndex(unsigned int count, int start, int index) const;
    int GetFunctionPresetId(int index) const;
    int GetCustomPresetId(int index) const;
    int GetDefaultPresetId(int index) const;
    std::wstring GetDisplayName(int index) const;
    void PopulatePresetList();
    unsigned int GetFunction(int function) const;
    int GetTooltip(int function) const;
    virtual void updateTooltips();
    virtual void updateComponents();
    SuperflatConfig* GetSuperflatConfig(int index) const;
    static int LoadConfirmMessageCallback(void* data, int unknown, C4JStorage::EMessageResult result);
    void HandlePresetListPress(int function);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handlePress(int controlId, int childId);
    int GetIcon(int index) const;
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual std::wstring getMoviePath();
    virtual int getSceneType();
    void NewPreset(std::wstring preset);

private:
    FlatGeneratorInfo m_generator;
    std::vector<int> m_functionPresetIds;
    std::vector<SuperflatConfig*> m_customPresets;
    std::vector<SuperflatConfig*> m_defaultPresets;
    UIControl_LeftIconButtonList m_presetList;
    UIControl_Label m_presetTitle;
    int m_selectedPreset;
};

#endif
