#ifndef GAME_UI_UISCENE_BIOMEPREVIEWMENU_HPP
#define GAME_UI_UISCENE_BIOMEPREVIEWMENU_HPP

#include "game/ui/UIVec2D.hpp"
#include <string>
#include <vector>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_Cursor.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class C4JThreadImpl;
class LaunchMoreOptionsMenuInitData;

class UIScene_BiomePreviewMenu : public UIScene
{
public:
    class CustomImage
    {
    public:
        CustomImage();
        ~CustomImage();

        unsigned char mTextureLoaded;
        unsigned char mPadding[3];
        UIControl_BitmapIcon mImage;
        int mTexture;
        arrayWithLength<unsigned char> mPixels;
    };

    enum ENavDir
    {
        NAV_LEFT,
        NAV_RIGHT,
        NAV_UP,
        NAV_DOWN
    };

    UIScene_BiomePreviewMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_BiomePreviewMenu();

    void setNewPointerPos(float x, float y);
    void setNewPointerPos(const _UIVec2D& position);
    void UpdatePixelCaches(int sectionX, int sectionZ, bool atVerticalEdge, bool atHorizontalEdge);
    static unsigned int UpdateRegenBiomes(void* data);
    static C4JThreadImpl* s_regenThread;

    CustomImage* getImage(bool wholeMap);
    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual std::wstring getMoviePath();
    virtual void customDraw(const char* name, fuiRect* rect);
    void dmouse(float& x, float& y, float& width, float& height);
    void componentExtents(UIControl& control, float& left, float& right, float& top, float& bottom);
    bool isOverComponent(UIControl& control, float x, float y);
    void onMouseTick();
    void UpdatePreviewImage(const arrayWithLength<unsigned char>& pixels, bool wholeMap);
    virtual void tick();
    void updateMapNavigatorFromOverrideSettings();
    void handleScale(bool increase);
    void handleZoom(bool zoomOut);
    void handleNavigate(ENavDir direction);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual UIControl* GetMainPanel();
    virtual int getSceneType();
    void SetMapNavigator(int x, int z, int scale);

private:
    CustomImage mBiomeMap;
    CustomImage mNavigatorMap;
    UIControl mMainPanel;
    UIControl mTimer;
    UIControl mScrollUp;
    UIControl mScrollDown;
    UIControl mScrollLeft;
    UIControl mScrollRight;
    UIControl_Label mPlayableAreaLabel;
    UIControl_Label mCoordinatesLabel;
    UIControl_Label mFullMapLabel;
    UIControl_HTMLLabel mHelpLabel;
    UIControl_Cursor mCursor;
    LaunchMoreOptionsMenuInitData* mInitData;
    BiomeSource::LayerOverrideSettings mOverrideSettings;
    int mPreviewZoomLevel;
    int mTargetZoomLevel;
    bool mImageReady;
    bool mSectionsChanged;
    bool mRegenerate;
    float mPointerOffsetX;
    float mPointerOffsetY;
    float mMapLeft;
    float mMapRight;
    float mMapTop;
    float mMapBottom;
    _UIVec2D mPointer;
    bool mStopRegen;
    OSFastMutex mCriticalSection;
};

#endif
