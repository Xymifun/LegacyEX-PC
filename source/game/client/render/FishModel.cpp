#include "game/client/render/FishModel.hpp"

#include "game/client/render/FishRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Fish.hpp"
#include "game/entity/mob/Entity.hpp"

FishModel::FishModel(const ModelDefinition& modelDefinition, FishRenderer::FishType fishType, int variant)
{
    mPart0 = NULL;
    mPart1 = NULL;
    mPart2 = NULL;
    mPart3 = NULL;
    mPart4 = NULL;
    mPart5 = NULL;
    mPart6 = NULL;
    mPart7 = NULL;
    mPart8 = NULL;
    mPart9 = NULL;
    mPart10 = NULL;
    mPart11 = NULL;
    mPart12 = NULL;
    mPart13 = NULL;
    mPart14 = NULL;
    mPart15 = NULL;
    mPart16 = NULL;
    mPart17 = NULL;
    mPart18 = NULL;
    mPart19 = NULL;
    mPart20 = NULL;
    mFishType = fishType;
    mVariant = variant;
    if (modelDefinition.DoesOverrideModel())
    {
        switch (fishType)
        {
        case FishRenderer::FISH_TYPE_COD:
            mPart0 = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
            mPart1 = new ModelPartSkin(this, L"tailfin", modelDefinition, mPart0, 0.0f, NULL);
            mPart4 = new ModelPartSkin(this, L"head", modelDefinition, mPart0, 0.0f, NULL);
            mPart5 = new ModelPartSkin(this, L"waist", modelDefinition, mPart0, 0.0f, NULL);
            break;
        case FishRenderer::FISH_TYPE_PUFFERFISH:
            mPart0 = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
            mPart1 = new ModelPartSkin(this, L"tailfin", modelDefinition, mPart0, 0.0f, NULL);
            mPart9 = new ModelPartSkin(this, L"spines_top_front", modelDefinition, mPart0, 0.0f, NULL);
            mPart10 = new ModelPartSkin(this, L"spines_top_mid", modelDefinition, mPart0, 0.0f, NULL);
            mPart11 = new ModelPartSkin(this, L"spines_top_back", modelDefinition, mPart0, 0.0f, NULL);
            mPart12 = new ModelPartSkin(this, L"spines_bottom_front", modelDefinition, mPart0, 0.0f, NULL);
            mPart13 = new ModelPartSkin(this, L"spines_bottom_mid", modelDefinition, mPart0, 0.0f, NULL);
            mPart14 = new ModelPartSkin(this, L"spines_bottom_back", modelDefinition, mPart0, 0.0f, NULL);
            mPart15 = new ModelPartSkin(this, L"spines_left_front", modelDefinition, mPart0, 0.0f, NULL);
            mPart16 = new ModelPartSkin(this, L"spines_left_mid", modelDefinition, mPart0, 0.0f, NULL);
            mPart17 = new ModelPartSkin(this, L"spines_left_back", modelDefinition, mPart0, 0.0f, NULL);
            mPart18 = new ModelPartSkin(this, L"spines_right_front", modelDefinition, mPart0, 0.0f, NULL);
            mPart19 = new ModelPartSkin(this, L"spines_right_mid", modelDefinition, mPart0, 0.0f, NULL);
            mPart20 = new ModelPartSkin(this, L"spines_right_back", modelDefinition, mPart0, 0.0f, NULL);
            break;
        case FishRenderer::FISH_TYPE_SALMON:
            mPart0 = new ModelPartSkin(this, L"body_front", modelDefinition, NULL, 0.0f, NULL);
            mPart6 = new ModelPartSkin(this, L"body_back", modelDefinition, mPart0, 0.0f, NULL);
            mPart7 = new ModelPartSkin(this, L"dorsal_front", modelDefinition, mPart0, 0.0f, NULL);
            mPart8 = new ModelPartSkin(this, L"dorsal_back", modelDefinition, mPart6, 0.0f, NULL);
            mPart1 = new ModelPartSkin(this, L"tailfin", modelDefinition, mPart6, 0.0f, NULL);
            mPart4 = new ModelPartSkin(this, L"head", modelDefinition, mPart0, 0.0f, NULL);
            break;
        case FishRenderer::FISH_TYPE_TROPICAL_FISH:
            mPart0 = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
            mPart1 = new ModelPartSkin(this, L"tailfin", modelDefinition, mPart0, 0.0f, NULL);
            break;
        }
        mPart2 = new ModelPartSkin(this, L"leftFin", modelDefinition, mPart0, 0.0f, NULL);
        mPart3 = new ModelPartSkin(this, L"rightFin", modelDefinition, mPart0, 0.0f, NULL);
    }
    else
    {
        mTexWidth = 32;
        mTexHeight = 32;
        switch (fishType)
        {
        case FishRenderer::FISH_TYPE_COD:
            mPart0 = new ModelPartSkin(this, 0, 0, NULL);
            mPart0->addBox(-1.0f, -4.0f, 1.0f, 2, 4, 7);
            mPart0->addBoxMapTex(0.0f, -5.0f, 0.0f, 0, 1, 6, 20, -6, 0.0f);
            mPart0->addBoxMapTex(0.0f, 0.0f, 3.0f, 0, 1, 2, 22, -1, 0.0f);
            mPart0->setPos(0.0f, 24.0f, -2.0f);
            mPart4 = new ModelPartSkin(this, 0, 0, NULL);
            mPart4->addBox(-1.0f, -2.0f, -3.0f, 2, 3, 1);
            mPart4->addBoxMapTex(-1.0f, -2.0f, -2.0f, 2, 4, 3, 11, 0, 0.0f);
            mPart4->setPos(0.0f, -2.0f, 0.0f);
            mPart0->addChild(mPart4);
            mPart2 = new ModelPartSkin(this, 24, 4, NULL);
            mPart2->addBox(0.0f, 0.0f, 0.0f, 2, 1, 2);
            mPart2->mZRot = 0.61f;
            mPart2->setPos(1.0f, -1.0f, 0.0f);
            mPart0->addChild(mPart2);
            mPart3 = new ModelPartSkin(this, 24, 1, NULL);
            mPart3->addBox(-2.0f, 0.0f, 0.0f, 2, 1, 2);
            mPart3->mZRot = -0.61f;
            mPart3->setPos(-1.0f, -1.0f, 0.0f);
            mPart0->addChild(mPart3);
            mPart1 = new ModelPartSkin(this, 20, 1, NULL);
            mPart1->addBox(0.0f, -4.0f, 0.0f, 0, 4, 6);
            mPart1->setPos(0.0f, 0.0f, 8.0f);
            mPart0->addChild(mPart1);
            break;
        case FishRenderer::FISH_TYPE_PUFFERFISH:
            switch (variant)
            {
            case 0:
                mPart0 = new ModelPartSkin(this, 0, 27, NULL);
                mPart0->addBox(-1.5f, 0.0f, -1.5f, 3, 2, 3);
                mPart0->addBoxMapTex(0.5f, -1.0f, -1.5f, 1, 1, 1, 24, 6, 0.0f);
                mPart0->addBoxMapTex(-1.5f, -1.0f, -1.5f, 1, 1, 1, 28, 6, 0.0f);
                mPart0->setPos(0.0f, 21.0f, 0.0f);
                mPart2 = new ModelPartSkin(this, 25, 0, NULL);
                mPart2->addBox(1.5f, 0.0f, -1.5f, 1, 1, 2);
                mPart2->setPos(0.0f, 1.0f, 0.0f);
                mPart2->mirror();
                mPart0->addChild(mPart2);
                mPart3 = new ModelPartSkin(this, 25, 0, NULL);
                mPart3->addBox(-2.5f, 0.0f, -1.5f, 1, 1, 2);
                mPart3->setPos(0.0f, 1.0f, 0.0f);
                mPart0->addChild(mPart3);
                mPart1 = new ModelPartSkin(this, -3, 0, NULL);
                mPart1->addBox(-1.5f, 1.0f, 1.5f, 3, 0, 3);
                mPart1->setPos(0.0f, 0.0f, 0.0f);
                mPart0->addChild(mPart1);
                break;
            case 1:
                mPart0 = new ModelPartSkin(this, 12, 22, NULL);
                mPart0->setPos(0.0f, 24.0f, 0.0f);
                mPart0->addBox(-2.5f, -6.0f, -2.5f, 5, 5, 5);
                mPart2 = new ModelPartSkin(this, 24, 3, NULL);
                mPart2->setPos(2.5f, -5.0f, 0.5f);
                mPart2->addBox(0.0f, -0.0f, -2.0f, 2, 1, 2);
                mPart0->addChild(mPart2);
                mPart3 = new ModelPartSkin(this, 24, 0, NULL);
                mPart3->setPos(-2.5f, -5.0f, 0.5f);
                mPart3->addBox(-2.0f, -0.0f, -2.0f, 2, 1, 2);
                mPart0->addChild(mPart3);
                mPart9 = new ModelPartSkin(this, 19, 17, NULL);
                mPart9->setPos(0.0f, -6.0f, -2.5f);
                mPart9->mXRot = 45.0f * Mth::DEG_TO_RAD;
                mPart9->addBox(-2.5f, -1.0f, 0.0f, 5, 1, 0);
                mPart0->addChild(mPart9);
                mPart10 = new ModelPartSkin(this, 0, 0, NULL);
                mPart10->setPos(0.0f, -24.0f, 0.0f);
                mPart0->addChild(mPart10);
                mPart11 = new ModelPartSkin(this, 11, 17, NULL);
                mPart11->setPos(0.0f, -6.0f, 2.5f);
                mPart11->mXRot = -45.0f * Mth::DEG_TO_RAD;
                mPart11->addBox(-2.5f, -1.0f, 0.0f, 5, 1, 0);
                mPart0->addChild(mPart11);
                mPart12 = new ModelPartSkin(this, 18, 20, NULL);
                mPart12->setPos(0.0f, -1.0f, -2.5f);
                mPart12->mXRot = -45.0f * Mth::DEG_TO_RAD;
                mPart12->addBox(-2.5f, -0.0f, 0.0f, 5, 1, 0);
                mPart0->addChild(mPart12);
                mPart13 = new ModelPartSkin(this, 0, 0, NULL);
                mPart13->setPos(0.0f, -24.0f, 0.0f);
                mPart0->addChild(mPart13);
                mPart14 = new ModelPartSkin(this, 18, 20, NULL);
                mPart14->setPos(0.0f, -1.0f, 2.5f);
                mPart14->mXRot = 45.0f * Mth::DEG_TO_RAD;
                mPart14->addBox(-2.5f, -0.0f, 0.0f, 5, 1, 0);
                mPart0->addChild(mPart14);
                mPart15 = new ModelPartSkin(this, 1, 17, NULL);
                mPart15->setPos(2.5f, 0.0f, -2.5f);
                mPart15->mYRot = 45.0f * Mth::DEG_TO_RAD;
                mPart15->addBox(0.0f, -6.0f, 0.0f, 1, 5, 0);
                mPart0->addChild(mPart15);
                mPart16 = new ModelPartSkin(this, 0, 0, NULL);
                mPart16->setPos(0.0f, -24.0f, 0.0f);
                mPart0->addChild(mPart16);
                mPart17 = new ModelPartSkin(this, 1, 17, NULL);
                mPart17->setPos(2.5f, 0.0f, 2.5f);
                mPart17->mYRot = -45.0f * Mth::DEG_TO_RAD;
                mPart17->addBox(0.0f, -6.0f, 0.0f, 1, 5, 0);
                mPart0->addChild(mPart17);
                mPart18 = new ModelPartSkin(this, 5, 17, NULL);
                mPart18->setPos(-2.5f, 0.0f, -2.5f);
                mPart18->mYRot = -45.0f * Mth::DEG_TO_RAD;
                mPart18->addBox(-1.0f, -6.0f, 0.0f, 1, 5, 0);
                mPart0->addChild(mPart18);
                mPart19 = new ModelPartSkin(this, 0, 0, NULL);
                mPart19->setPos(0.0f, -24.0f, 0.0f);
                mPart0->addChild(mPart19);
                mPart20 = new ModelPartSkin(this, 9, 17, NULL);
                mPart20->setPos(-2.5f, 0.0f, 2.5f);
                mPart20->mYRot = 45.0f * Mth::DEG_TO_RAD;
                mPart20->addBox(-1.0f, -6.0f, 0.0f, 1, 5, 0);
                mPart0->addChild(mPart20);
                mPart1 = NULL;
                break;
            case 2:
                mPart0 = new ModelPartSkin(this, 0, 0, NULL);
                mPart0->setPos(0.0f, 24.0f, 0.0f);
                mPart0->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8);
                mPart2 = new ModelPartSkin(this, 24, 3, NULL);
                mPart2->setPos(4.0f, -7.0f, 3.0f);
                mPart2->addBox(0.0f, -0.0f, -5.99f, 2, 1, 2);
                mPart0->addChild(mPart2);
                mPart3 = new ModelPartSkin(this, 24, 0, NULL);
                mPart3->setPos(-4.0f, -7.0f, 1.0f);
                mPart3->addBox(-1.997f, -0.0f, -3.992f, 2, 1, 2);
                mPart0->addChild(mPart3);
                mPart9 = new ModelPartSkin(this, 14, 16, NULL);
                mPart9->setPos(-4.0f, -8.0f, -4.0f);
                mPart9->mXRot = 45.0f * Mth::DEG_TO_RAD;
                mPart9->addBox(0.0f, -1.0f, 0.0f, 8, 1, 1);
                mPart0->addChild(mPart9);
                mPart10 = new ModelPartSkin(this, 14, 16, NULL);
                mPart10->setPos(0.0f, -8.0f, 0.0f);
                mPart10->addBox(-4.0f, -1.0f, 0.0f, 8, 1, 1);
                mPart0->addChild(mPart10);
                mPart11 = new ModelPartSkin(this, 14, 16, NULL);
                mPart11->setPos(0.0f, -8.0f, 4.0f);
                mPart11->mXRot = -45.0f * Mth::DEG_TO_RAD;
                mPart11->addBox(-4.0f, -1.0f, 0.0f, 8, 1, 1);
                mPart0->addChild(mPart11);
                mPart12 = new ModelPartSkin(this, 14, 19, NULL);
                mPart12->setPos(0.0f, 0.0f, -4.0f);
                mPart12->mXRot = -45.0f * Mth::DEG_TO_RAD;
                mPart12->addBox(-4.0f, -0.0f, 0.0f, 8, 1, 1);
                mPart0->addChild(mPart12);
                mPart13 = new ModelPartSkin(this, 14, 19, NULL);
                mPart13->setPos(0.0f, 1.0f, 0.0f);
                mPart13->addBox(-4.0f, -1.0f, 0.0f, 8, 1, 1);
                mPart0->addChild(mPart13);
                mPart14 = new ModelPartSkin(this, 14, 19, NULL);
                mPart14->setPos(0.0f, 0.0f, 4.0f);
                mPart14->mXRot = 45.0f * Mth::DEG_TO_RAD;
                mPart14->addBox(-4.0f, -0.0f, 0.0f, 8, 1, 1);
                mPart0->addChild(mPart14);
                mPart15 = new ModelPartSkin(this, 0, 16, NULL);
                mPart15->setPos(4.0f, 0.0f, -4.0f);
                mPart15->mYRot = 45.0f * Mth::DEG_TO_RAD;
                mPart15->addBox(0.0f, -8.0f, 0.0f, 1, 8, 1);
                mPart0->addChild(mPart15);
                mPart16 = new ModelPartSkin(this, 4, 16, NULL);
                mPart16->setPos(4.0f, 0.0f, 0.0f);
                mPart16->addBox(0.0f, -8.0f, 0.0f, 1, 8, 1, true);
                mPart0->addChild(mPart16);
                mPart17 = new ModelPartSkin(this, 8, 16, NULL);
                mPart17->setPos(4.0f, 0.0f, 4.0f);
                mPart17->mYRot = -45.0f * Mth::DEG_TO_RAD;
                mPart17->addBox(0.0f, -8.0f, 0.0f, 1, 8, 1, true);
                mPart0->addChild(mPart17);
                mPart18 = new ModelPartSkin(this, 4, 16, NULL);
                mPart18->setPos(-4.0f, 0.0f, -4.0f);
                mPart18->mYRot = -45.0f * Mth::DEG_TO_RAD;
                mPart18->addBox(-1.0f, -8.0f, 0.0f, 1, 8, 1);
                mPart0->addChild(mPart18);
                mPart19 = new ModelPartSkin(this, 8, 16, NULL);
                mPart19->setPos(-4.0f, 0.0f, 0.0f);
                mPart19->addBox(-1.0f, -8.0f, 0.0f, 1, 8, 1);
                mPart0->addChild(mPart19);
                mPart20 = new ModelPartSkin(this, 8, 16, NULL);
                mPart20->setPos(-4.0f, 0.0f, 4.0f);
                mPart20->mYRot = 45.0f * Mth::DEG_TO_RAD;
                mPart20->addBox(-1.0f, -8.0f, 0.0f, 1, 8, 1);
                mPart0->addChild(mPart20);
                mPart1 = NULL;
                break;
            }
            break;
        case FishRenderer::FISH_TYPE_SALMON:
            mPart0 = new ModelPartSkin(this, 0, 0, NULL);
            mPart0->addBox(-1.5f, -8.5f, 0.0f, 3, 5, 8);
            mPart0->setPos(0.0f, 26.0f, -4.0f);
            mPart6 = new ModelPartSkin(this, 0, 13, NULL);
            mPart6->addBox(-1.5f, -8.5f, 0.0f, 3, 5, 8);
            mPart6->setPos(0.0f, 0.0f, 8.0f);
            mPart0->addChild(mPart6);
            mPart7 = new ModelPartSkin(this, 4, 2, NULL);
            mPart7->addBox(0.0f, -5.5f, 0.0f, 0, 2, 2);
            mPart7->setPos(0.0f, -5.0f, 6.0f);
            mPart0->addChild(mPart7);
            mPart8 = new ModelPartSkin(this, 2, 3, NULL);
            mPart8->addBox(0.0f, -5.5f, 0.0f, 0, 2, 3);
            mPart8->setPos(0.0f, -5.0f, 0.0f);
            mPart6->addChild(mPart8);
            mPart1 = new ModelPartSkin(this, 20, 10, NULL);
            mPart1->addBox(0.0f, -8.5f, 0.0f, 0, 5, 6);
            mPart1->setPos(0.0f, 0.0f, 8.0f);
            mPart6->addChild(mPart1);
            mPart4 = new ModelPartSkin(this, 22, 0, NULL);
            mPart4->addBox(-1.0f, -5.5f, -3.0f, 2, 4, 3);
            mPart4->setPos(0.0f, -3.0f, 0.0f);
            mPart0->addChild(mPart4);
            mPart2 = new ModelPartSkin(this, 2, 0, NULL);
            mPart2->addBox(-2.0f, -2.87f, 0.0f, 2, 0, 2);
            mPart2->mZRot = 0.61f;
            mPart2->setPos(1.5f, -1.0f, 0.0f);
            mPart0->addChild(mPart2);
            mPart3 = new ModelPartSkin(this, -2, 0, NULL);
            mPart3->addBox(0.01f, -2.87f, 0.0f, 2, 0, 2);
            mPart3->mZRot = -0.61f;
            mPart3->setPos(-1.5f, -1.0f, 0.0f);
            mPart0->addChild(mPart3);
            break;
        case FishRenderer::FISH_TYPE_TROPICAL_FISH:
            switch (variant)
            {
            case 0:
                mPart0 = new ModelPartSkin(this, 0, 0, NULL);
                mPart0->addBox(-0.5f, -3.0f, -3.0f, 2, 3, 6);
                mPart0->addBoxMapTex(0.5f, -7.0f, -3.0f, 0, 4, 6, 10, -6, 0.0f);
                mPart0->setPos(-0.5f, 24.0f, 0.0f);
                mPart1 = new ModelPartSkin(this, 24, -4, NULL);
                mPart1->addBox(0.0f, -3.0f, 0.0f, 0, 3, 4);
                mPart1->setPos(0.5f, 0.0f, 3.0f);
                mPart0->addChild(mPart1);
                mPart2 = new ModelPartSkin(this, 2, 12, NULL);
                mPart2->addBox(-0.164f, -2.0f, -1.106f, 2, 2, 0);
                mPart2->mYRot = -0.611f;
                mPart2->setPos(1.0f, 0.0f, 1.0f);
                mPart0->addChild(mPart2);
                mPart3 = new ModelPartSkin(this, 2, 16, NULL);
                mPart3->addBox(-1.836f, -2.0f, -1.106f, 2, 2, 0);
                mPart3->mYRot = 0.611f;
                mPart3->setPos(0.0f, 0.0f, 1.0f);
                mPart0->addChild(mPart3);
                break;
            case 1:
                mPart0 = new ModelPartSkin(this, 0, 20, NULL);
                mPart0->addBox(-0.5f, -6.0f, -0.0008f, 2, 6, 6);
                mPart0->addBoxMapTex(0.5f, 0.0f, -0.0008f, 0, 5, 6, 20, 21, 0.0f);
                mPart0->addBoxMapTex(0.5f, -11.0f, -0.0008f, 0, 5, 6, 20, 10, 0.0f);
                mPart0->setPos(-0.5f, 24.0f, -3.0f);
                mPart1 = new ModelPartSkin(this, 21, 16, NULL);
                mPart1->addBox(0.0f, -6.0f, 0.0f, 0, 6, 5);
                mPart1->setPos(0.5f, 0.0f, 6.0f);
                mPart0->addChild(mPart1);
                mPart2 = new ModelPartSkin(this, 2, 12, NULL);
                mPart2->addBox(1.557f, -2.0f, 1.351f, 2, 2, 0);
                mPart2->mYRot = -0.611f;
                mPart2->setPos(1.0f, 0.0f, 1.0f);
                mPart0->addChild(mPart2);
                mPart3 = new ModelPartSkin(this, 2, 16, NULL);
                mPart3->addBox(-3.557f, -2.0f, 1.351f, 2, 2, 0);
                mPart3->mYRot = 0.611f;
                mPart3->setPos(0.0f, 0.0f, 1.0f);
                mPart0->addChild(mPart3);
                break;
            }
            break;
        }
    }
    compileSkinnedDrawlist();
}

void FishModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                       float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    boost::shared_ptr<Fish> fish = boost::static_pointer_cast<Fish>(entity);
    setupAnim(Mth::Lerp(fish->mSwimRotationOld, fish->mSwimRotation, 0.05f), limbSwingAmount, ageInTicks, netHeadYaw,
              headPitch, scale, entity, _SkinAdjustments());
    if (fish->mSwimFlag)
    {
        GlStateManager::pushMatrix();
        GlStateManager::scalef(fish->mSwimSpeed, fish->mSwimSpeed, fish->mSwimSpeed);
        GlStateManager::translatef(0.0f, static_cast<float>(fish->mSwimTicks) * scale, 0.0f);
        mPart0->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
        callSkinnedDrawlist();
    }
    else
    {
        mPart0->render(scale, useDisplayList, false);
        callSkinnedDrawlist();
    }
}

void FishModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                          float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    float swing = Mth::cos(limbSwing * Mth::DEG_TO_RAD * mAnimationScale);
    switch (mFishType)
    {
    case FishRenderer::FISH_TYPE_COD:
        mPart4->mYRot = swing * Mth::DEG_TO_RAD * 4.0f;
        mPart0->mYRot = swing * Mth::DEG_TO_RAD * 2.0f;
        mPart1->mYRot = swing * Mth::DEG_TO_RAD * -25.75f;
        break;
    case FishRenderer::FISH_TYPE_PUFFERFISH:
        if (mPart1 != NULL)
            mPart1->mYRot = swing * Mth::DEG_TO_RAD * -5.75f;
        break;
    case FishRenderer::FISH_TYPE_SALMON:
        mPart0->mYRot = swing * Mth::DEG_TO_RAD * 4.0f;
        mPart6->mYRot = swing * Mth::DEG_TO_RAD * -21.75f;
        mPart1->mYRot = swing * Mth::DEG_TO_RAD * -40.0f;
        break;
    case FishRenderer::FISH_TYPE_TROPICAL_FISH:
        mPart0->mYRot = swing * Mth::DEG_TO_RAD * 2.0f;
        mPart1->mYRot = swing * Mth::DEG_TO_RAD * -25.75f;
        break;
    }
}

void FishModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                float partialTicks)
{
    if (!entity->isInWater())
    {
        mPart0->mZRot = Mth::cos(static_cast<float>(entity->mTickCount) * 0.25f) * 90.0f * Mth::DEG_TO_RAD;
        mAnimationScale = 45.0f;
        mAnimationOffset = 45.0f;
        return;
    }
    mAnimationScale = 30.0f;
    mAnimationOffset = 30.0f;
    mPart0->mZRot = 0.0f;
}

std::wstring FishModel::getName()
{
    switch (mFishType)
    {
    case FishRenderer::FISH_TYPE_COD:
        return L"cod";
    case FishRenderer::FISH_TYPE_PUFFERFISH:
        switch (mVariant)
        {
        case 0:
            return L"pufferfish.small";
        case 1:
            return L"pufferfish.mid";
        case 2:
            return L"pufferfish.large";
        }
        return L"pufferfish";
    case FishRenderer::FISH_TYPE_SALMON:
        return L"salmon";
    case FishRenderer::FISH_TYPE_TROPICAL_FISH:
        switch (mVariant)
        {
        case 0:
            return L"tropicalfish_a";
        case 1:
            return L"tropicalfish_b";
        }
        break;
    }
    return L"cod";
}
