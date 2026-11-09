#ifndef GAME_UI_UISCENE_SUPERFLATMENU_HPP
#define GAME_UI_UISCENE_SUPERFLATMENU_HPP

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_CheckboxButtonList.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_LayerIconButtonList.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "game/ui/UIControl_Touch.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/ui/SuperflatMenu.hpp"

class FlatLayerInfo;
class SoundEvent;

class LaunchMoreOptionsMenuInitData;

class UIScene_SuperFlatMenu : public UIScene
{
public:
    enum eTabs
    {
        eTabLayers,
        eTabBiomes,
        eTabProperties
    };

    class TickAction
    {
    public:
        virtual ~TickAction();
        virtual void Execute(UIScene_SuperFlatMenu* scene) = 0;
    };

    class MethodAction : public TickAction
    {
    public:
        typedef void (UIScene_SuperFlatMenu::*Func)();
        MethodAction(Func function);
        virtual ~MethodAction();
        virtual void Execute(UIScene_SuperFlatMenu* scene);

    private:
        Func m_function;
    };

    template <class T> class MethodActionArg1 : public TickAction
    {
    public:
        typedef void (UIScene_SuperFlatMenu::*Func)(T);
        MethodActionArg1(Func function, T value) : m_function(function), m_value(value) {}
        virtual ~MethodActionArg1() {}
        virtual void Execute(UIScene_SuperFlatMenu* scene) { (scene->*m_function)(m_value); }

    private:
        Func m_function;
        T m_value;
    };

    template <class List> class TabControls
    {
    public:
        UIControl_Label tab;
        UIControl panel;
        List list;
        UIControl_Label helpTitle;
        UIControl_HTMLLabel help;
        UIControl timer;
    };

    UIScene_SuperFlatMenu(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_SuperFlatMenu();
    virtual unsigned int mapElementsAndNames();
    void PopulateBiomesList();
    int ParentParam(int index);
    void CreateParamMappings();
    void PopulateParamsList();
    void _init();
    void setLayerButton(int index, const FlatLayerInfo& layer);
    void PopulateLayersList();
    void UpdateBiomesList();
    void UpdateParamsList();
    void LoadPreset(SuperflatConfig* config);
    void SetActiveTab(eTabs tab);
    virtual void handleTouchInput(unsigned int playerId, int touchX, int touchY, int touchBox, bool pressed,
                                  bool repeat, bool released);
    virtual UIControl* GetMainPanel();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void tick();
    int GetLayerIndex(int index);
    void UpdateLayersHelpText();
    void UpdateBiomeHelpText();
    void UpdateParamHelpText();
    virtual void handleReload();
    void ExecuteOnTick(MethodAction::Func function);
    template <class T> void ExecuteOnTick(typename MethodActionArg1<T>::Func function, T value);
    void MoveCurrentLayer(int amount);
    void NavActiveTab(int direction);
    static int CbGenerationWarning(void* context, int pad, C4JStorage::EMessageResult result);
    void ReturnResults(bool accepted);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    static int CbLayerOperation(void* context, int pad, C4JStorage::EMessageResult result);
    void ChangeBiome(int biome);
    void HandleChangeParamList(int index);
    virtual void handlePress(int controlId, int childId);
    virtual void handleCheckboxElementToggled(int first, int second, bool checked);
    virtual void handleSliderElementMove(int first, int second, int value);
    virtual void handleSelectionChanged(int selection);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handleGainFocus(bool hadFocus);
    virtual void customDraw(const char* name, fuiRect* rect);
    void SetSelectedLayer(int index);
    void DeleteLayer();
    void WritePropertyChanges(int index);
    void PlaySound(const SoundEvent* sound);
    void OpenPresetsDialog();
    void OpenEditLayerDialog(bool insert);
    void OpenMaxHeightDialog();
    virtual std::wstring getMoviePath();
    virtual int getSceneType();
    void InsertNewLayer();
    static int CbPresetName(void* context, bool accepted);

private:
    std::vector<TickAction*> m_tickActions;
    UIControl_Touch m_layersTouch;
    UIControl_Touch m_biomesTouch;
    UIControl_Touch m_propertiesTouch;
    unsigned int m_activeTab;
    TabControls<UIControl_LayerIconButtonList> m_layers;
    TabControls<UIControl_CheckboxButtonList> m_biomes;
    TabControls<UIControl_MultiList> m_properties;
    bool m_generating;
    bool m_editingLayer;
    int m_pendingSelection;
    boost::unordered_map<int, SuperflatMenu::Mapping*> m_paramMappings;
    bool m_presetsFlag;
    SuperflatConfig* m_config;
    FlatGeneratorInfo m_generatorInfo;
    LaunchMoreOptionsMenuInitData* m_initData;
};

template <class T>
void UIScene_SuperFlatMenu::ExecuteOnTick(typename MethodActionArg1<T>::Func function, T value)
{
    m_tickActions.push_back(new MethodActionArg1<T>(function, value));
}

#endif
