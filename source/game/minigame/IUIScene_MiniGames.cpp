#include "game/minigame/IUIScene_MiniGames.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/IUIPartial_OnlineRestricted.hpp"
#include "game/minigame/MiniGameDef.hpp"

IUIScene_MiniGames::Variant::Variant() : mPlayLabel(-1), mDescription(-1), mPlayLabelAlt(-1), mDescriptionAlt(-1)
{
}

IUIScene_MiniGames::IUIScene_MiniGames()
{
}

unsigned int IUIScene_MiniGames::GetVariantPlayLabel(int index, bool alternate)
{
    return alternate ? mVariants[index].mPlayLabel : mVariants[index].mPlayLabelAlt;
}

unsigned int IUIScene_MiniGames::GetVariantDesc(int index, bool alternate)
{
    return alternate ? mVariants[index].mDescription : mVariants[index].mDescriptionAlt;
}

void IUIScene_MiniGames::SetUpVariants()
{
    MiniGameDef* miniGame = GetMiniGame();
    int optionsCount = miniGame->GetOptionsCount(eCustomGameModeHostOption_ModeVariant);
    switch (miniGame->GetId())
    {
    case 1:
        mVariants[0] = Variant(-1, -1, IDS_JOIN_TAB, IDS_MG_JOIN_ANY_GAME_DESC);
        if (optionsCount == 3)
        {
            mVariants[1].mPlayLabel = IDS_MG_CREATE_GAME;
            mVariants[1].mDescription = IDS_MG_CREATE_CASUAL_GAME_DESC;
            mVariants[1].mPlayLabelAlt = IDS_JOIN_TAB;
            mVariants[1].mDescriptionAlt = IDS_MG_JOIN_CASUAL_GAME_DESC;
            mVariants[2].mPlayLabel = IDS_MG_CREATE_GAME;
            mVariants[2].mDescription = IDS_MG_CREATE_COMPETITIVE_GAME_DESC;
            mVariants[2].mPlayLabelAlt = IDS_JOIN_TAB;
            mVariants[2].mDescriptionAlt = IDS_MG_JOIN_COMPETITIVE_GAME_DESC;
            mVariants[3].mPlayLabel = IDS_MG_CREATE_GAME;
            mVariants[3].mDescription = IDS_MG_CREATE_CUSTOM_GAME_DESC;
            mVariants[3].mPlayLabelAlt = IDS_JOIN_TAB;
            mVariants[3].mDescriptionAlt = IDS_MG_JOIN_CUSTOM_GAME_DESC;
        }
        else
        {
            mVariants[1] = Variant(IDS_MG_CREATE_GAME, 0xb6a97e24, -1, -1);
        }
        break;
    case 3:
        mVariants[0] = Variant(-1, -1, IDS_JOIN_TAB, 0x26fc3c0e);
        if (optionsCount == 2)
        {
            mVariants[1].mPlayLabel = IDS_MG_CREATE_GAME;
            mVariants[1].mDescription = 0xb746dec7;
            mVariants[1].mPlayLabelAlt = IDS_JOIN_TAB;
            mVariants[1].mDescriptionAlt = 0x4e09c09;
            mVariants[2].mPlayLabel = IDS_MG_CREATE_GAME;
            mVariants[2].mDescription = 0xb4516cb;
            mVariants[2].mPlayLabelAlt = IDS_JOIN_TAB;
            mVariants[2].mDescriptionAlt = 0x5840ccf5;
        }
        else
        {
            mVariants[1] = Variant(IDS_MG_CREATE_GAME, 0xb6a97e24, -1, -1);
        }
        break;
    case 2:
        mVariants[0] = Variant(-1, -1, IDS_JOIN_TAB, 0xf4cd599f);
        if (optionsCount == 4)
        {
            mVariants[1].mPlayLabel = IDS_MG_CREATE_GAME;
            mVariants[1].mDescription = 0x41afac5c;
            mVariants[1].mPlayLabelAlt = IDS_JOIN_TAB;
            mVariants[1].mDescriptionAlt = 0x4e43a572;
            mVariants[2].mPlayLabel = IDS_MG_CREATE_GAME;
            mVariants[2].mDescription = 0x9f01e42a;
            mVariants[2].mPlayLabelAlt = IDS_JOIN_TAB;
            mVariants[2].mDescriptionAlt = 0xc708bb04;
            mVariants[3].mPlayLabel = IDS_MG_CREATE_GAME;
            mVariants[3].mDescription = 0x89b0cf2c;
            mVariants[3].mPlayLabelAlt = IDS_JOIN_TAB;
            mVariants[3].mDescriptionAlt = 0x30045cc2;
            mVariants[4].mPlayLabel = IDS_MG_CREATE_GAME;
            mVariants[4].mDescription = IDS_MG_CREATE_CUSTOM_GAME_DESC;
            mVariants[4].mPlayLabelAlt = IDS_JOIN_TAB;
            mVariants[4].mDescriptionAlt = IDS_MG_JOIN_CUSTOM_GAME_DESC;
        }
        break;
    }
}
