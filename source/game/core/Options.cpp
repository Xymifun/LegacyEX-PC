#include "game/core/Options.hpp"

#include "game/client/audio/SoundEngine.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferedReader.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/FileInputStream.hpp"
#include "game/core/FileOutputStream.hpp"
#include "game/core/I18n.hpp"
#include "game/core/InputStreamReader.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"

template <typename T> std::wstring _toString(T value);
template <typename T> T _fromString(const std::wstring& value);

namespace
{
std::wstring renderDistanceNames[4] = {L"options.renderDistance.far", L"options.renderDistance.normal",
                                       L"options.renderDistance.short", L"options.renderDistance.tiny"};

std::wstring difficultyNames[4] = {L"options.difficulty.peaceful", L"options.difficulty.easy",
                                   L"options.difficulty.normal", L"options.difficulty.hard"};

std::wstring guiScaleNames[4] = {L"options.guiScale.auto", L"options.guiScale.small", L"options.guiScale.normal",
                                 L"options.guiScale.large"};

std::wstring framerateNames[3] = {L"performance.max", L"performance.balanced", L"performance.powersaver"};

std::wstring particleNames[3] = {L"options.particles.all", L"options.particles.decreased",
                                 L"options.particles.minimal"};

Options::Option items[18] = {
    Options::Option(L"options.music", true, false),           Options::Option(L"options.sound", true, false),
    Options::Option(L"options.invertMouse", false, true),     Options::Option(L"options.sensitivity", true, false),
    Options::Option(L"options.renderDistance", false, false), Options::Option(L"options.viewBobbing", false, true),
    Options::Option(L"options.anaglyph", false, true),        Options::Option(L"options.advancedOpengl", false, true),
    Options::Option(L"options.framerateLimit", false, false), Options::Option(L"options.difficulty", false, false),
    Options::Option(L"options.graphics", false, false),       Options::Option(L"options.ao", false, true),
    Options::Option(L"options.guiScale", false, false),       Options::Option(L"options.fov", true, false),
    Options::Option(L"options.gamma", true, false),           Options::Option(L"options.renderClouds", false, true),
    Options::Option(L"options.particles", false, false),      Options::Option(L"options.autoJump", false, true)};
}

const Options::Option* Options::Option::music = &::items[0];
const Options::Option* Options::Option::sound = &::items[1];
const Options::Option* Options::Option::invertMouse = &::items[2];
const Options::Option* Options::Option::sensitivity = &::items[3];
const Options::Option* Options::Option::renderDistance = &::items[4];
const Options::Option* Options::Option::viewBobbing = &::items[5];
const Options::Option* Options::Option::anaglyph = &::items[6];
const Options::Option* Options::Option::advancedOpengl = &::items[7];
const Options::Option* Options::Option::framerateLimit = &::items[8];
const Options::Option* Options::Option::difficulty = &::items[9];
const Options::Option* Options::Option::graphics = &::items[10];
const Options::Option* Options::Option::ambientOcclusion = &::items[11];
const Options::Option* Options::Option::guiScale = &::items[12];
const Options::Option* Options::Option::fov = &::items[13];
const Options::Option* Options::Option::gamma = &::items[14];
const Options::Option* Options::Option::renderClouds = &::items[15];
const Options::Option* Options::Option::particles = &::items[16];
const Options::Option* Options::Option::autoJump = &::items[17];

Options::Options(Minecraft* minecraft, File directory)
{
    init();
    mMinecraft = minecraft;
    mOptionsFile = File(directory, L"options.txt");
}

void Options::init()
{
    mMusicVolume = 1.0f;
    mSoundVolume = 1.0f;
    mSensitivity = 0.5f;
    mInvertYMouse = false;
    mViewDistance = 0;
    mViewBobbing = true;
    mAnaglyph3d = false;
    mAdvancedOpengl = false;
    mFpsLimit = 2;
    mFancyGraphics = true;
    mAmbientOcclusion = true;
    mClouds = true;
    mSkin = L"Default";

    mKeyForward = new KeyMapping(L"key.forward", 22);
    mKeyLeft = new KeyMapping(L"key.left", 0);
    mKeyBack = new KeyMapping(L"key.back", 18);
    mKeyRight = new KeyMapping(L"key.right", 3);
    mKeyJump = new KeyMapping(L"key.jump", 26);
    mKeyInventory = new KeyMapping(L"key.inventory", 4);
    mKeyDrop = new KeyMapping(L"key.drop", 16);
    mKeyChat = new KeyMapping(L"key.chat", 19);
    mKeySneak = new KeyMapping(L"key.sneak", 27);
    mKeyAttack = new KeyMapping(L"key.attack", -100);
    mKeyUse = new KeyMapping(L"key.use", -99);
    mKeyPlayerList = new KeyMapping(L"key.playerlist", 34);
    mKeyPickItem = new KeyMapping(L"key.pickItem", -98);
    mKeyFog = new KeyMapping(L"key.fog", 5);

    mKeyMappings[0] = mKeyAttack;
    mKeyMappings[1] = mKeyUse;
    mKeyMappings[2] = mKeyForward;
    mKeyMappings[3] = mKeyLeft;
    mKeyMappings[4] = mKeyBack;
    mKeyMappings[5] = mKeyRight;
    mKeyMappings[6] = mKeyJump;
    mKeyMappings[7] = mKeySneak;
    mKeyMappings[8] = mKeyDrop;
    mKeyMappings[9] = mKeyInventory;
    mKeyMappings[10] = mKeyChat;
    mKeyMappings[11] = mKeyPlayerList;
    mKeyMappings[12] = mKeyPickItem;
    mKeyMappings[13] = mKeyFog;

    mMinecraft = 0;
    mDifficulty = 2;
    mHideGui = false;
    mThirdPersonView = 0;
    mRenderDebug = 0;
    mShowInfoBars = false;
    mShowChunkInfoMap = 0;
    mLastServer = L"";
    mScrollHotbar = false;
    mUnknownFlag4 = false;
    mUnknownFlag5 = false;
    mAutoJump = true;
    mHotbarScroll = 1.0f;
    mUnknownScale2 = 1.0f;
    mGuiScale = 0;
    mParticles = 0;
    mFov = 0.0f;
    mGamma = 0.0f;
    mUnknownFlag6 = true;
}

std::wstring Options::getKeyDescription(int index)
{
    return Language::getInstance()->getElement(mKeyMappings[index]->mName);
}

std::wstring Options::getKeyMessage(int index)
{
    int key = mKeyMappings[index]->mKey;
    if (key < 0)
        return I18n::get(L"key.mouseButton", key + 101);
    return L"KEYNAME";
}

void Options::save()
{
    FileOutputStream fileOutput(mOptionsFile);
    DataOutputStream output(&fileOutput);

    output.writeChars(L"music:" + _toString<float>(mMusicVolume) + L"\n");
    output.writeChars(L"sound:" + _toString<float>(mSoundVolume) + L"\n");
    output.writeChars(L"invertYMouse:" + std::wstring(mInvertYMouse ? L"true" : L"false") + L"\n");
    output.writeChars(L"mouseSensitivity:" + _toString<float>(mSensitivity));
    output.writeChars(L"fov:" + _toString<float>(mFov));
    output.writeChars(L"gamma:" + _toString<float>(mGamma));
    output.writeChars(L"viewDistance:" + _toString<int>(mViewDistance));
    output.writeChars(L"guiScale:" + _toString<int>(mGuiScale));
    output.writeChars(L"particles:" + _toString<int>(mParticles));
    output.writeChars(L"bobView:" + std::wstring(mViewBobbing ? L"true" : L"false"));
    output.writeChars(L"anaglyph3d:" + std::wstring(mAnaglyph3d ? L"true" : L"false"));
    output.writeChars(L"advancedOpengl:" + std::wstring(mAdvancedOpengl ? L"true" : L"false"));
    output.writeChars(L"fpsLimit:" + _toString<int>(mFpsLimit));
    output.writeChars(L"difficulty:" + _toString<int>(mDifficulty));
    output.writeChars(L"fancyGraphics:" + std::wstring(mFancyGraphics ? L"true" : L"false"));
    output.writeChars(L"ao:" + std::wstring(mAmbientOcclusion ? L"true" : L"false"));
    output.writeChars(L"clouds:" + _toString<bool>(mClouds));
    output.writeChars(L"skin:" + mSkin);
    output.writeChars(L"lastServer:" + mLastServer);

    int i;
    for (i = 0; i < 14; ++i)
        output.writeChars(L"key_" + mKeyMappings[i]->mName + L":" + _toString<int>(mKeyMappings[i]->mKey));

    output.close();
}

void Options::setKey(int index, int key)
{
    mKeyMappings[index]->mKey = key;
    save();
}

void Options::set(const Option* option, float value)
{
    if (option == Option::music)
    {
        mMusicVolume = value;
        mMinecraft->mSoundEngine->updateMusicVolume(value);
    }
    if (option == Option::sound)
    {
        mSoundVolume = value;
        mMinecraft->mSoundEngine->updateSoundEffectVolume(value);
    }
    if (option == Option::sensitivity)
        mSensitivity = value;
    if (option == Option::fov)
        mFov = value;
    if (option == Option::gamma)
        mGamma = value;
}

void Options::toggle(const Option* option, int amount)
{
    if (option == Option::invertMouse)
        mInvertYMouse = !mInvertYMouse;
    if (option == Option::renderDistance)
        mViewDistance = (mViewDistance + amount) & 3;
    if (option == Option::guiScale)
        mGuiScale = (mGuiScale + amount) & 3;
    if (option == Option::particles)
        mParticles = (mParticles + amount) % 3;
    if (option == Option::viewBobbing)
        mViewBobbing = amount != 0;
    if (option == Option::renderClouds)
        mClouds = !mClouds;
    if (option == Option::advancedOpengl)
    {
        mAdvancedOpengl = !mAdvancedOpengl;
        mMinecraft->mLevelRenderer->allChanged();
    }
    if (option == Option::anaglyph)
    {
        mAnaglyph3d = !mAnaglyph3d;
        mMinecraft->mTextures->reloadAll();
    }
    if (option == Option::framerateLimit)
        mFpsLimit = (mFpsLimit + amount + 3) % 3;
    if (option == Option::difficulty)
        mDifficulty = amount & 3;
    if (option == Option::graphics)
    {
        mFancyGraphics = !mFancyGraphics;
        mMinecraft->mLevelRenderer->allChanged();
    }
    if (option == Option::ambientOcclusion)
    {
        mAmbientOcclusion = !mAmbientOcclusion;
        mMinecraft->mLevelRenderer->allChanged();
    }
    if (option == Option::autoJump)
        mAutoJump = !mAutoJump;
}

float Options::getProgressValue(const Option* option)
{
    if (option == Option::fov)
        return mFov;
    if (option == Option::gamma)
        return mGamma;
    if (option == Option::music)
        return mMusicVolume;
    if (option == Option::sound)
        return mSoundVolume;
    if (option == Option::sensitivity)
        return mSensitivity;
    return 0.0f;
}

bool Options::getBooleanValue(const Option* option)
{
    if (option == Option::invertMouse)
        return mInvertYMouse;
    if (option == Option::viewBobbing)
        return mViewBobbing;
    if (option == Option::anaglyph)
        return mAnaglyph3d;
    if (option == Option::advancedOpengl)
        return mAdvancedOpengl;
    if (option == Option::ambientOcclusion)
        return mAmbientOcclusion;
    if (option == Option::renderClouds)
        return mClouds;
    if (option == Option::autoJump)
        return mAutoJump;
    return false;
}

std::wstring Options::getMessage(const Option* option)
{
    Language* language = Language::getInstance();
    std::wstring caption = language->getElement(option->getCaptionId()) + L": ";

    if (option->isProgress())
    {
        float value = getProgressValue(option);
        if (option == Option::sensitivity)
        {
            if (value == 0.0f)
            {
                std::wstring key = L"options.sensitivity.min";
                return caption + language->getElement(key);
            }
            if (value == 1.0f)
            {
                std::wstring key = L"options.sensitivity.max";
                return caption + language->getElement(key);
            }
            return caption + _toString<int>(static_cast<int>(value * 200.0f)) + L"%";
        }
        else if (option == Option::fov)
        {
            if (value == 0.0f)
            {
                std::wstring key = L"options.fov.min";
                return caption + language->getElement(key);
            }
            if (value == 1.0f)
            {
                std::wstring key = L"options.fov.max";
                return caption + language->getElement(key);
            }
            return caption + _toString<int>(static_cast<int>(value * 40.0f + 70.0f));
        }
        else if (option == Option::gamma)
        {
            if (value == 0.0f)
            {
                std::wstring key = L"options.gamma.min";
                return caption + language->getElement(key);
            }
            if (value == 1.0f)
            {
                std::wstring key = L"options.gamma.max";
                return caption + language->getElement(key);
            }
            return caption + L"+" + _toString<int>(static_cast<int>(value * 100.0f)) + L"%";
        }
        else
        {
            if (value == 0.0f)
                return caption + language->getElement(L"options.off");
            return caption + _toString<int>(static_cast<int>(value * 100.0f)) + L"%";
        }
    }

    if (option->isBoolean())
    {
        if (getBooleanValue(option))
            return caption + language->getElement(L"options.on");
        return caption + language->getElement(L"options.off");
    }
    if (option == Option::renderDistance)
        return caption + language->getElement(renderDistanceNames[mViewDistance]);
    if (option == Option::difficulty)
        return caption + language->getElement(difficultyNames[mDifficulty]);
    if (option == Option::guiScale)
        return caption + language->getElement(guiScaleNames[mGuiScale]);
    if (option == Option::particles)
        return caption + language->getElement(particleNames[mParticles]);
    if (option == Option::framerateLimit)
        return caption + I18n::get(framerateNames[mFpsLimit]);
    if (option == Option::graphics)
    {
        if (mFancyGraphics)
            return caption + language->getElement(L"options.graphics.fancy");
        return caption + language->getElement(L"options.graphics.fast");
    }
    return caption;
}

float Options::readFloat(std::wstring value)
{
    if (value == L"true")
        return 1.0f;
    if (value == L"false")
        return 0.0f;
    return _fromString<float>(value);
}

void Options::load()
{
    if (!mOptionsFile.exists())
        return;

    FileInputStream* fileInput = new FileInputStream(mOptionsFile);
    InputStreamReader* input = new InputStreamReader(fileInput);
    BufferedReader* reader = new BufferedReader(input);
    std::wstring line = L"";

    while ((line = reader->readLine()) != L"")
    {
        std::wstring parts[2];
        std::wstring::size_type separator = line.find(L":");
        if (separator == std::wstring::npos)
        {
            parts[0] = line;
            parts[1] = L"";
        }
        else
        {
            parts[0] = line.substr(0, separator);
            parts[1] = line.substr(separator + 1);
        }

        if (parts[0] == L"music")
            mMusicVolume = readFloat(parts[1]);
        if (parts[0] == L"sound")
            mSoundVolume = readFloat(parts[1]);
        if (parts[0] == L"mouseSensitivity")
            mSensitivity = readFloat(parts[1]);
        if (parts[0] == L"fov")
            mFov = readFloat(parts[1]);
        if (parts[0] == L"gamma")
            mGamma = readFloat(parts[1]);
        if (parts[0] == L"invertYMouse")
            mInvertYMouse = parts[1] == L"true";
        if (parts[0] == L"viewDistance")
            mViewDistance = _fromString<int>(parts[1]);
        if (parts[0] == L"guiScale")
            mGuiScale = _fromString<int>(parts[1]);
        if (parts[0] == L"particles")
            mParticles = _fromString<int>(parts[1]);
        if (parts[0] == L"bobView")
            mViewBobbing = parts[1] == L"true";
        if (parts[0] == L"anaglyph3d")
            mAnaglyph3d = parts[1] == L"true";
        if (parts[0] == L"advancedOpengl")
            mAdvancedOpengl = parts[1] == L"true";
        if (parts[0] == L"fpsLimit")
            mFpsLimit = _fromString<int>(parts[1]);
        if (parts[0] == L"difficulty")
            mDifficulty = _fromString<int>(parts[1]);
        if (parts[0] == L"fancyGraphics")
            mFancyGraphics = parts[1] == L"true";
        if (parts[0] == L"ao")
            mAmbientOcclusion = parts[1] == L"true";
        if (parts[0] == L"clouds")
            mClouds = parts[1] == L"true";
        if (parts[0] == L"skin")
            mSkin = parts[1];
        if (parts[0] == L"lastServer")
            mLastServer = parts[1];

        int i;
        for (i = 0; i < 14; ++i)
        {
            if (parts[0] == L"key_" + mKeyMappings[i]->mName)
                mKeyMappings[i]->mKey = _fromString<int>(parts[1]);
        }
    }

    reader->close();
}

Options::Option::Option(const std::wstring& captionId, bool progress, bool boolean)
    : mProgress(progress),
      mBoolean(boolean)
{
    mCaptionId.assign(captionId, 0, 0xffffffff);
}

std::wstring Options::Option::getCaptionId() const
{
    return mCaptionId;
}

bool Options::Option::isProgress() const
{
    return mProgress;
}

bool Options::Option::isBoolean() const
{
    return mBoolean;
}

Options::Option* Options::Option::getItem(int id)
{
    return &::items[id];
}

int Options::Option::getId() const
{
    return static_cast<int>(this - ::items);
}
