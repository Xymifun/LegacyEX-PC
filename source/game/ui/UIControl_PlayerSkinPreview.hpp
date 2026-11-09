#ifndef GAME_UI_UICONTROL_PLAYERSKINPREVIEW_HPP
#define GAME_UI_UICONTROL_PLAYERSKINPREVIEW_HPP

#include <string>
#include <vector>

#include "game/ui/UIControl.hpp"
#include "game/client/render/Textures.hpp"
#include "vendor/_SkinAdjustments.hpp"

class EntityRenderer;
class ModelPart;
class PlayerModel;
class UIScene_SkinSelectMenu;
struct fuiRect;

class UIControl_PlayerSkinPreview : public UIControl
{
public:
    friend class UIScene_SkinSelectMenu;

    enum ESkinPreviewFacing
    {
        eFacingFront,
        eFacingRight,
        eFacingLeft
    };

    UIControl_PlayerSkinPreview();

    virtual void tick();
    unsigned int bindTexture(const std::wstring& name, int texture);
    unsigned int bindTexture(const std::wstring& name, const std::wstring& path);
    void render(EntityRenderer* renderer, double x, double y, double z, float yaw, float partialTick);
    void render(fuiRect* rect);
    void SetTexture(const std::wstring& texture, _TEXTURE_NAME textureName);
    void SetFacing(ESkinPreviewFacing facing, bool animate);
    void CycleNextAnimation();
    void CyclePreviousAnimation();
    void SetSkinAdjustments(_SkinAdjustments adjustments);

private:
    int mUnknown50;
    float mUnknown54;
    float mUnknown58;
    std::wstring mTexturePath;
    int mTextureName;
    std::wstring mCapePath;
    _SkinAdjustments mSkinAdjustments;
    float mWidth;
    float mHeight;
    float mWidthScale;
    float mHeightScale;
    int mRotation;
    int mPitch;
    int mInitialRotation;
    int mFacingRotation;
    float mUnknown110;
    float mLimbSwingAmountOld;
    float mLimbSwingAmount;
    float mLimbSwing;
    bool mCapeVisible;
    bool mFacingLeft;
    signed char mCapeSwingTicks;
    int mAnimationTicks;
    bool mAnimating;
    float mAnimationAngle;
    int mAnimation;
    std::vector<ModelPart*>* mAdditionalModelParts;
    bool mRotating;
    bool mRotateBack;
    bool mRotateForward;
    bool mRotateBackward;
#ifdef MINECRAFT_PC
    void PcTickStep();
    long long mPcTickTime;
    int mPcTickCarry;
    long long mPcRenderTime;
    int mPcRenderCarry;
#endif
};

#endif
