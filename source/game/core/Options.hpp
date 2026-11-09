#ifndef GAME_CORE_OPTIONS_HPP
#define GAME_CORE_OPTIONS_HPP

#include <string>

#include "game/core/File.hpp"
#include "game/core/KeyMapping.hpp"

class Minecraft;
class GameRenderer;

class Options
{
public:
    friend class Minecraft;
    friend class TextureUtil;
    friend class GameRenderer;
    friend class BlockRenderer;
    friend class Font;
    friend class LevelRenderer;
    friend class GameSettings;
    friend class Textures;

    class Option
    {
    public:
        Option(const std::wstring& captionId, bool progress, bool boolean);

        std::wstring getCaptionId() const;
        bool isProgress() const;
        bool isBoolean() const;
        static Option* getItem(int id);
        int getId() const;

        static const Option* music;
        static const Option* sound;
        static const Option* invertMouse;
        static const Option* sensitivity;
        static const Option* renderDistance;
        static const Option* viewBobbing;
        static const Option* anaglyph;
        static const Option* advancedOpengl;
        static const Option* framerateLimit;
        static const Option* difficulty;
        static const Option* graphics;
        static const Option* ambientOcclusion;
        static const Option* guiScale;
        static const Option* fov;
        static const Option* gamma;
        static const Option* renderClouds;
        static const Option* particles;
        static const Option* autoJump;

    private:
        bool mProgress;
        bool mBoolean;
        std::wstring mCaptionId;
    };

    Options(Minecraft* minecraft, File directory);
    Options();

    void init();
    std::wstring getKeyDescription(int index);
    std::wstring getKeyMessage(int index);
    void save();
    void setKey(int index, int key);
    void set(const Option* option, float value);
    void toggle(const Option* option, int amount);
    float getProgressValue(const Option* option);
    bool getBooleanValue(const Option* option);
    std::wstring getMessage(const Option* option);
    float readFloat(std::wstring value);
    void load();
    bool isCloudsOn();

private:
    float mMusicVolume;
    float mSoundVolume;
    float mSensitivity;
    bool mInvertYMouse;

public:
    int mViewDistance;

private:
    bool mViewBobbing;
    bool mAnaglyph3d;
    bool mAdvancedOpengl;
    int mFpsLimit;

public:
    bool mFancyGraphics;

private:
    bool mAmbientOcclusion;
    bool mClouds;
    bool mAutoJump;
    std::wstring mSkin;
    KeyMapping* mKeyForward;
    KeyMapping* mKeyLeft;
    KeyMapping* mKeyBack;
    KeyMapping* mKeyRight;
    KeyMapping* mKeyJump;
    KeyMapping* mKeyInventory;
    KeyMapping* mKeyDrop;
    KeyMapping* mKeyChat;
    KeyMapping* mKeySneak;
    KeyMapping* mKeyAttack;
    KeyMapping* mKeyUse;
    KeyMapping* mKeyPlayerList;
    KeyMapping* mKeyPickItem;
    KeyMapping* mKeyFog;
    KeyMapping* mKeyMappings[14];
    Minecraft* mMinecraft;
    File mOptionsFile;
    int mDifficulty;

public:
    bool mHideGui;
    int mThirdPersonView;

private:
    int mRenderDebug;
    bool mShowInfoBars;
    int mShowChunkInfoMap;

public:
    std::wstring mLastServer;

private:
    bool mScrollHotbar;
    bool mUnknownFlag4;
    bool mUnknownFlag5;
    float mHotbarScroll;
    float mUnknownScale2;

public:
    int mGuiScale;

public:
    int mParticles;

private:
    float mFov;
    float mGamma;
    bool mUnknownFlag6;
};

#endif
