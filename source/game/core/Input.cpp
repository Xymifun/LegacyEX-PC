#include "game/core/Input.hpp"

#include "platform/wiiu/CInput.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/PowerupItems.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcInput.hpp"
#include "platform/wiiu/CProfile.hpp"

extern C_4JProfile* gProfile;
#endif

Input::Input()
{
    mMoveStrafe = 0.0f;
    mMoveForward = 0.0f;
    mWasJumping = false;
    mJumping = false;
    mSneaking = false;
    mMovementReady = false;
    mLookReady = false;
    mUp = false;
    mDown = false;
    mLeft = false;
    mRight = false;
    mSwimming = false;
}

Input::~Input()
{
}

void Input::tick(LocalPlayer* player)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    int playerIndex = player->mUserIndex;

    mLeft = false;
    mRight = false;

    if (minecraft->mGameModes[playerIndex]->isInputAllowed(EControllerActions_MoveLeft)
        || minecraft->mGameModes[playerIndex]->isInputAllowed(EControllerActions_MoveRight))
    {
        mMoveStrafe = -g_pInput->GetJoypadStick_LX(playerIndex, true);
        if (mMoveStrafe > 0.5f)
        {
            mLeft = true;
        }
        else if (mMoveStrafe < -0.5f)
        {
            mRight = true;
        }
    }
    else
    {
        mMoveStrafe = 0.0f;
    }

    mUp = false;
    mDown = false;

    if (minecraft->mGameModes[playerIndex]->isInputAllowed(EControllerActions_MoveForward)
        || minecraft->mGameModes[playerIndex]->isInputAllowed(EControllerActions_MoveBackward))
    {
        mMoveForward = g_pInput->GetJoypadStick_LY(playerIndex, true);
        if (mMoveForward > 0.0f)
        {
            mUp = true;
        }
        else if (mMoveForward < 0.0f)
        {
            mDown = true;
        }
    }
    else
    {
        mMoveForward = 0.0f;
    }

    if (!mMovementReady)
    {
        if (mMoveStrafe * mMoveStrafe + mMoveForward * mMoveForward == 0.0f)
        {
            mMovementReady = true;
        }
        mMoveStrafe = 0.0f;
        mMoveForward = 0.0f;
    }

    bool swimming = player->isInWater() && !player->mOnGround && !player->mAbilities.mFlying;
    if (swimming && (player->mButtonsDown & (1LL << EControllerActions_Sneak)) != 0)
    {
        mSwimming = true;
    }
    else
    {
        mSwimming = false;
    }

    if ((!player->mAbilities.mFlying && !swimming) || player->isPassenger())
    {
        if ((player->mButtonsDown & (1LL << EControllerActions_Sneak)) != 0
            && minecraft->mGameModes[playerIndex]->isInputAllowed(EControllerActions_Sneak))
        {
            mSneaking = !mSneaking;
        }
#ifdef MINECRAFT_PC
        if (playerIndex == gProfile->GetPrimaryPad())
        {
            static bool s_keyboardSneak = false;
            const bool held = PcInputSneakHeld();
            if (held)
                mSneaking = true;
            else if (s_keyboardSneak)
                mSneaking = false;
            s_keyboardSneak = held;
        }
#endif
    }

    if (mSneaking)
    {
        mMoveStrafe *= 0.3f;
        mMoveForward *= 0.3f;
    }

    float turnX = 0.0f;
    float turnY = 0.0f;

    if (minecraft->mGameModes[playerIndex]->isInputAllowed(EControllerActions_LookUp)
        || minecraft->mGameModes[playerIndex]->isInputAllowed(EControllerActions_LookDown))
    {
        turnX = g_pInput->GetJoypadStick_RX(playerIndex, true)
                * (static_cast<float>(GameSettings::GetGameSettings(playerIndex, eGameSetting_GameSensitivity))
                   / 100.0f);
    }

    if (minecraft->mGameModes[playerIndex]->isInputAllowed(EControllerActions_LookLeft)
        || minecraft->mGameModes[playerIndex]->isInputAllowed(EControllerActions_LookRight))
    {
        turnY = g_pInput->GetJoypadStick_RY(playerIndex, true)
                * (static_cast<float>(GameSettings::GetGameSettings(playerIndex, eGameSetting_GameSensitivity))
                   / 100.0f);
    }

    if (GameSettings::GetGameSettings(playerIndex, eGameSetting_InvertLook) != 0)
    {
        turnY = -turnY;
    }

    if (gApp.GetFirstMiniGameType() == eMiniGameId_Glide
        && player->CheckPowerup(PowerupItems::eGlide_Timed_Powerup_ID_InvertControls))
    {
        turnY = -turnY;
        turnX = -turnX;
    }

    if (!mLookReady)
    {
        if (turnX * turnX + turnY * turnY == 0.0f)
        {
            mLookReady = true;
        }
        turnX = turnY = 0.0f;
    }

    player->interpolateTurn(turnX * (turnX < 0.0f ? -turnX : turnX) * 50.0f,
                            turnY * (turnY < 0.0f ? -turnY : turnY) * 50.0f);

    if (g_pInput->GetValue(playerIndex, EControllerActions_Jump, false) != 0
        && minecraft->mGameModes[playerIndex]->isInputAllowed(EControllerActions_Jump))
    {
        mJumping = true;
    }
    else
    {
        mJumping = false;
    }

    if (gApp.GetFreezePlayers())
    {
        mMoveStrafe = 0.0f;
        mMoveForward = 0.0f;
        mJumping = false;
    }
}

phys::Vec2* Input::getMoveVector()
{
    return phys::Vec2::newTemp(mMoveStrafe, mMoveForward);
}
