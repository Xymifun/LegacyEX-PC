#ifndef GAME_UI_UISCENE_HPP
#define GAME_UI_UISCENE_HPP

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/client/render/C4JRender.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/fuiRect.hpp"

class ConsoleUIController;
class ItemInstance;
class ItemRenderer;
class UIControl;
class UIControl_Base;
class UILayer;
struct fuiFile;

enum EUIMessage
{
    EUIMessage_Unknown = 0
};

class UIScene
{
    friend class ConsoleUIController;
    friend class UILayer;
    friend class UIControl_Button;
    friend class UIControl_ButtonList;
    friend class UIControl_CheckBox;
    friend class UIControl_DynamicLabel;
    friend class UIControl_HTMLLabel;
    friend class UIControl_LeaderboardList;
    friend class UIControl_List;
    friend class UIControl_PlayerButtonList;
    friend class UIControl_Slider;
    friend class UIControl_TextInput;
    friend class UIControl_TexturePackList;
    friend class UIControl_Touch;
    friend class UIControl_GridList;
    friend class UIControl_EnchantmentButton;
    friend class UIControl_Progress;

public:
    struct _TimerInfo
    {
        int duration;
        int triggerTime;
        bool active;
    };

    struct _CachedSlotDrawData
    {
    };

    UIScene(int playerId, UILayer* layer);

    virtual void reloadMovie(bool force);
    virtual bool needsReloaded();
    virtual bool hasMovie();
    virtual void updateSafeZone();
    virtual void updateViewportTouchOffset();
    virtual double getSafeZoneHalfHeight();
    virtual double getSafeZoneHalfWidth();
    virtual std::wstring getMoviePath() = 0;
    virtual unsigned int mapElementsAndNames();
    virtual ~UIScene();
    virtual int getSceneType() = 0;
    virtual unsigned int getSubSceneType() const;
    virtual void tick();
    virtual void SetFocusToElement(int focus);
    virtual void handleTimerComplete(int timerId);
    virtual bool stealsFocus();
    virtual bool hasFocus(int playerId);
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handleGainFocus(bool hadFocus);
    virtual void handleLoseFocus();
    virtual bool hidesLowerScenes();
    virtual bool blocksInput();
    virtual UIControl* GetMainPanel();
    virtual void render(int width, int height, C4JRender::eViewportType viewport);
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual bool allowRepeat(int action);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleDestroy();
    virtual void handlePreUnloadForReload();
    virtual void handlePreReload();
    virtual void handleReload();
    virtual void handlePress(int controlId, int childId);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handleInitFocus(int focus, int childFocus);
    virtual void handleCheckboxToggled(int id, bool checked);
    virtual void handleCheckboxElementToggled(int first, int second, bool checked);
    virtual void handleSliderMove(int id, int value);
    virtual void handleSliderElementMove(int first, int second, int value);
    virtual void handleAnimationEnd();
    virtual void handleSelectionChanged(int selection);
    virtual void handleRequestMoreData(int id, bool forward);
    virtual void handleTouchBoxRebuild();
    virtual void HandleDLCMountingComplete();
    virtual void HandleDLCInstalled();
    virtual void HandleDLCLicenseChange();
    virtual void HandleTMSBanFileRetrieved();
    virtual void HandleMessage(EUIMessage message, void* data);
    virtual void HandleSaveDeviceRemoved(int deviceIndex);
    virtual void handleUnlockFullVersion();
    virtual void handleTouchInput(unsigned int playerId, int touchX, int touchY, int touchBox, bool pressed,
                                  bool repeat, bool released);
    virtual bool isReadyToDelete();

    static void customDrawFui(void* context, const char* name, fuiRect* rect);
    void tickTimers();
    void navigateBack();
    bool hasRegisteredSubstitutionTexture(const std::wstring& name);
    void unregisterSubstitutionTexture(const std::wstring& name, bool deleteData);
    void registerSubstitutionTexture(const std::wstring& name, unsigned char* data, unsigned int size, bool deleteData);
    void loadMovie();
    void initialiseMovie();
    void addTimer(int timerId, int duration);
    void killTimer(int timerId);
    unsigned int convertGameActionToFuiKeycode(int action);
    void sendInputToMovie(int action, bool repeat, bool pressed, bool released);
    unsigned int GetCallbackUniqueId();
    bool controlHasFocus(int focus);
    unsigned int getControlChildFocus();
    void customDrawSlotControlFui(fuiRect* rect, int slot, not_null_ptr<ItemInstance> item, float partialTick,
                                  bool flat, bool drawDecorations, bool drawOverlay);
    void destroyMovie();
    void UpdateSceneControls();
    void removeControl(UIControl_Base* control, bool destroy);
    void doHorizontalResizeCheck();
    UIScene* getBackScene();
    void gainFocus();
    void loseFocus();
    void setBackScene(UIScene* scene);
    void setSafeZone(double top, double bottom, double left, double right);
    void resetHorizontalResizeCheck();
    void _handleFocusChange(int focus, int childFocus);
    void _handleInitFocus(int focus, int childFocus);
    bool controlHasFocus(UIControl_Base* control);
    int getControlFocus();
    void setOpacity(float opacity);
    void setVisible(bool visible);
    bool isVisible() { return m_visible; }
    bool hasTimer(int timerId);
    bool canCacheSWF();
    void slideLeft();
    void slideRight();

protected:
    std::wstring m_moviePath;
    fuiFile* m_movie;
    bool m_movieLoaded;
    ItemRenderer* m_itemRenderer;
    boost::unordered_map<std::wstring, bool> m_substitutionTextures;
    boost::unordered_map<int, _TimerInfo> m_timers;
    int m_controlFocus;
    int m_controlChildFocus;
    float m_opacity;
    bool m_opacityInitialised;
    bool m_visible;
    bool m_tickedLastFrame;
    UIScene* m_backScene;
    unsigned int m_callbackId;
    int m_movieResolution;
    bool m_reloading;
    bool m_hasGainedFocus;
    int m_stageWidth;
    int m_stageHeight;
    int m_originalStageWidth;
    int m_originalStageHeight;
    std::vector<UIControl*> m_controls;
    UILayer* m_layer;
    bool m_hasFocus;
    int m_playerId;
    bool m_hasTicked;
    bool m_slotDrawPending;
    bool m_updatePending;
    int m_slotDrawCount;
    std::vector<_CachedSlotDrawData*> m_cachedSlotDrawData;
    int m_unknownC0;
};

#endif
