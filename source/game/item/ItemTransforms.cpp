#include "game/item/ItemTransforms.hpp"
#include "game/core/EntityType.hpp"

#include <cstddef>

#include "game/client/render/BlockRenderer.hpp"
#include "game/client/render/SkullBlockRenderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemTransform.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

float ItemTransforms::offsetTranslateX = 0.0f;
float ItemTransforms::offsetTranslateY = 0.0f;
float ItemTransforms::offsetTranslateZ = 0.0f;
float ItemTransforms::offsetRotationX = 0.0f;
float ItemTransforms::offsetRotationY = 0.0f;
float ItemTransforms::offsetRotationZ = 0.0f;
float ItemTransforms::offsetScaleX = 0.0f;
float ItemTransforms::offsetScaleY = 0.0f;
float ItemTransforms::offsetScaleZ = 0.0f;

ItemTransform* ItemTransforms::getTransform(TransformType type) const
{
    switch (type)
    {
    case THIRD_PERSON_LEFT_HAND:
        return mThirdPersonLeftHand;
    case THIRD_PERSON_RIGHT_HAND:
        return mThirdPersonRightHand;
    case FIRST_PERSON_LEFT_HAND:
        return mFirstPersonLeftHand;
    case FIRST_PERSON_RIGHT_HAND:
        return mFirstPersonRightHand;
    case HEAD:
        return mHead;
    case GUI:
        return mGui;
    case GROUND:
        return mGround;
    case FIXED:
        return mFixed;
    default:
        return ItemTransform::NO_TRANSFORM;
    }
}

ItemTransforms* ItemTransforms::set_thirdperson_righthand(float rotationX, float rotationY, float rotationZ,
                                                          float translationX, float translationY, float translationZ,
                                                          float scaleX, float scaleY, float scaleZ)
{
    mThirdPersonRightHand = ItemTransform::deserialize(rotationX, rotationY, rotationZ, translationX, translationY,
                                                       translationZ, scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_thirdperson_lefthand(float rotationX, float rotationY, float rotationZ,
                                                         float translationX, float translationY, float translationZ,
                                                         float scaleX, float scaleY, float scaleZ)
{
    mThirdPersonLeftHand = ItemTransform::deserialize(rotationX, rotationY, rotationZ, translationX, translationY,
                                                      translationZ, scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_thirdperson_hands(float rotationX, float rotationY, float rotationZ,
                                                      float translationX, float translationY, float translationZ,
                                                      float scaleX, float scaleY, float scaleZ)
{
    set_thirdperson_righthand(rotationX, rotationY, rotationZ, translationX, translationY, translationZ, scaleX, scaleY,
                              scaleZ);
    set_thirdperson_lefthand(rotationX, rotationY, rotationZ, translationX, translationY, translationZ, scaleX, scaleY,
                             scaleZ);
    return this;
}

ItemTransforms::ItemTransforms(const ItemTransforms& other)
    : mThirdPersonLeftHand(other.mThirdPersonLeftHand),
      mThirdPersonRightHand(other.mThirdPersonRightHand),
      mFirstPersonLeftHand(other.mFirstPersonLeftHand),
      mFirstPersonRightHand(other.mFirstPersonRightHand),
      mHead(other.mHead),
      mGui(other.mGui),
      mGround(other.mGround),
      mFixed(other.mFixed)
{
}

ItemTransforms::ItemTransforms(ItemTransform* thirdPersonLeftHand, ItemTransform* thirdPersonRightHand,
                               ItemTransform* firstPersonLeftHand, ItemTransform* firstPersonRightHand,
                               ItemTransform* head, ItemTransform* gui, ItemTransform* ground, ItemTransform* fixed)
    : mThirdPersonLeftHand(thirdPersonLeftHand),
      mThirdPersonRightHand(thirdPersonRightHand),
      mFirstPersonLeftHand(firstPersonLeftHand),
      mFirstPersonRightHand(firstPersonRightHand),
      mHead(head),
      mGui(gui),
      mGround(ground),
      mFixed(fixed)
{
}

ItemTransforms* ItemTransforms::GetDefaultTransforms()
{
    static ItemTransforms* transforms = new ItemTransforms(
        new ItemTransform(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 3.0f, 1.0f), Vector3f(0.55f, 0.55f, 0.55f), true),
        new ItemTransform(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 3.0f, 1.0f), Vector3f(0.55f, 0.55f, 0.55f), true),
        new ItemTransform(Vector3f(0.0f, -90.0f, 25.0f), Vector3f(1.13f, 3.2f, 1.13f), Vector3f(0.68f, 0.68f, 0.68f),
                          true),
        new ItemTransform(Vector3f(0.0f, -90.0f, 25.0f), Vector3f(1.13f, 3.2f, 1.13f), Vector3f(0.68f, 0.68f, 0.68f),
                          true),
        new ItemTransform(Vector3f(0.0f, 180.0f, 0.0f), Vector3f(0.0f, 13.0f, 7.0f), Vector3f(1.0f, 1.0f, 1.0f), true),
        new ItemTransform(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.9f, 0.9f, 0.9f), true),
        new ItemTransform(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 2.0f, 0.0f), Vector3f(0.5f, 0.5f, 0.5f), true),
        ItemTransform::NO_TRANSFORM);
    return transforms;
}

ItemTransforms* ItemTransforms::getArmourStandTransforms(const ItemInstance* item, LivingEntity* entity)
{
    switch (Item::getId(item->getItem()))
    {
    case 390:
    {
        static ItemTransforms* transforms = (new ItemTransforms(*GetDefaultTransforms()))
                                                ->set_thirdperson_hands(0.0f, 0.0f, 0.0f, 0.0f, 1.75f, 1.0f, 0.55f,
                                                                        0.55f, 0.55f);
        return transforms;
    }
    case 371:
    {
        static ItemTransforms* transforms = (new ItemTransforms(*GetDefaultTransforms()))
                                                ->set_thirdperson_hands(0.0f, 0.0f, 0.0f, 0.0f, 1.2f, 1.0f, 0.55f,
                                                                        0.55f, 0.55f);
        return transforms;
    }
    case 452:
    {
        static ItemTransforms* transforms = (new ItemTransforms(*GetDefaultTransforms()))
                                                ->set_thirdperson_hands(0.0f, 0.0f, 0.0f, 0.0f, 0.65f, 1.0f, 0.55f,
                                                                        0.55f, 0.55f);
        return transforms;
    }
    case 378:
    {
        static ItemTransforms* transforms = (new ItemTransforms(*GetDefaultTransforms()))
                                                ->set_thirdperson_hands(0.0f, 0.0f, 0.0f, 0.0f, 2.32f, 1.0f, 0.55f,
                                                                        0.55f, 0.55f);
        return transforms;
    }
    case 281:
    case 282:
    case 413:
    case 436:
    {
        static ItemTransforms* transforms = (new ItemTransforms(*GetDefaultTransforms()))
                                                ->set_thirdperson_hands(0.0f, 0.0f, 0.0f, 0.0f, 1.7f, 1.0f, 0.55f,
                                                                        0.55f, 0.55f);
        return transforms;
    }
    case 111:
    case 263:
    case 264:
    case 287:
    case 318:
    case 319:
    case 320:
    case 321:
    case 325:
    case 326:
    case 327:
    case 328:
    case 334:
    case 335:
    case 342:
    case 343:
    case 344:
    case 354:
    case 358:
    case 363:
    case 364:
    case 373:
    case 375:
    case 376:
    case 377:
    case 382:
    case 388:
    case 389:
    case 392:
    case 393:
    case 394:
    case 395:
    case 400:
    case 407:
    case 408:
    case 409:
    case 415:
    case 420:
    case 426:
    case 432:
    case 434:
    case 438:
    case 441:
    case 459:
    case 460:
    case 461:
    case 462:
    {
        static ItemTransforms* transforms = (new ItemTransforms(*GetDefaultTransforms()))
                                                ->set_thirdperson_hands(0.0f, 0.0f, 0.0f, 0.0f, 2.85f, 1.0f, 0.55f,
                                                                        0.55f, 0.55f);
        return transforms;
    }
    case 288:
    case 289:
    case 296:
    case 329:
    case 331:
    case 332:
    case 333:
    case 337:
    case 341:
    case 345:
    case 348:
    case 349:
    case 350:
    case 353:
    case 357:
    case 367:
    case 385:
    case 406:
    case 410:
    case 411:
    case 412:
    case 433:
    case 444:
    case 445:
    case 446:
    case 447:
    case 448:
    {
        static ItemTransforms* transforms = (new ItemTransforms(*GetDefaultTransforms()))
                                                ->set_thirdperson_hands(0.0f, 0.0f, 0.0f, 0.0f, 2.3f, 1.0f, 0.55f,
                                                                        0.55f, 0.55f);
        return transforms;
    }
    case 265:
    case 266:
    case 295:
    case 336:
    case 338:
    case 351:
    case 359:
    case 361:
    case 391:
    case 396:
    case 405:
    case 435:
    {
        static ItemTransforms* transforms = (new ItemTransforms(*GetDefaultTransforms()))
                                                ->set_thirdperson_hands(0.0f, 0.0f, 0.0f, 0.0f, 1.5f, 1.0f, 0.55f,
                                                                        0.55f, 0.55f);
        return transforms;
    }
    case 355:
    case 362:
    case 370:
    case 372:
    case 414:
    case 417:
    case 418:
    case 419:
    case 453:
    {
        static ItemTransforms* transforms = (new ItemTransforms(*GetDefaultTransforms()))
                                                ->set_thirdperson_hands(0.0f, 0.0f, 0.0f, 0.0f, 1.2f, 1.0f, 0.55f,
                                                                        0.55f, 0.55f);
        return transforms;
    }
    default:
        return NULL;
    }
}

ItemTransforms* ItemTransforms::set_firstperson_lefthand(float rotationX, float rotationY, float rotationZ,
                                                         float translationX, float translationY, float translationZ,
                                                         float scaleX, float scaleY, float scaleZ)
{
    mFirstPersonLeftHand = ItemTransform::deserialize(rotationX, rotationY, rotationZ, translationX, translationY,
                                                      translationZ, scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_firstperson_righthand(float rotationX, float rotationY, float rotationZ,
                                                          float translationX, float translationY, float translationZ,
                                                          float scaleX, float scaleY, float scaleZ)
{
    mFirstPersonRightHand = ItemTransform::deserialize(rotationX, rotationY, rotationZ, translationX, translationY,
                                                       translationZ, scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms::ItemTransforms()
    : mThirdPersonLeftHand(ItemTransform::NO_TRANSFORM),
      mThirdPersonRightHand(ItemTransform::NO_TRANSFORM),
      mFirstPersonLeftHand(ItemTransform::NO_TRANSFORM),
      mFirstPersonRightHand(ItemTransform::NO_TRANSFORM),
      mHead(ItemTransform::NO_TRANSFORM),
      mGui(ItemTransform::NO_TRANSFORM),
      mGround(ItemTransform::NO_TRANSFORM),
      mFixed(ItemTransform::NO_TRANSFORM)
{
}

ItemTransforms* ItemTransforms::set_ground(float rotationX, float rotationY, float rotationZ, float translationX,
                                           float translationY, float translationZ, float scaleX, float scaleY,
                                           float scaleZ)
{
    mGround = ItemTransform::deserialize(rotationX, rotationY, rotationZ, translationX, translationY, translationZ,
                                         scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_fixed(float rotationX, float rotationY, float rotationZ, float translationX,
                                          float translationY, float translationZ, float scaleX, float scaleY,
                                          float scaleZ)
{
    mFixed = ItemTransform::deserialize(rotationX, rotationY, rotationZ, translationX, translationY, translationZ,
                                        scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_gui(float rotationX, float rotationY, float rotationZ, float translationX,
                                        float translationY, float translationZ, float scaleX, float scaleY,
                                        float scaleZ)
{
    mGui = ItemTransform::deserialize(rotationX, rotationY, rotationZ, translationX, translationY, translationZ, scaleX,
                                      scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_head(float rotationX, float rotationY, float rotationZ, float translationX,
                                         float translationY, float translationZ, float scaleX, float scaleY,
                                         float scaleZ)
{
    mHead = ItemTransform::deserialize(rotationX, rotationY, rotationZ, translationX, translationY, translationZ,
                                       scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_firstperson_hands(float rotationX, float rotationY, float rotationZ,
                                                      float translationX, float translationY, float translationZ,
                                                      float scaleX, float scaleY, float scaleZ)
{
    set_firstperson_righthand(rotationX, rotationY, rotationZ, translationX, translationY, translationZ, scaleX, scaleY,
                              scaleZ);
    set_firstperson_lefthand(rotationX, rotationY, rotationZ, translationX, translationY, translationZ, scaleX, scaleY,
                             scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::getTransforms(const ItemInstance* item, LivingEntity* entity)
{
    if (entity != NULL)
    {
        bool isArmourStand = eTYPE_DERIVED_FROM(EntityType::ARMOR_STAND, entity->GetType());
        if (isArmourStand)
        {
            ItemTransforms* armourStandTransforms = getArmourStandTransforms(item, entity);
            if (armourStandTransforms != NULL)
                return armourStandTransforms;
        }
    }

    switch (Item::getId(item->getItem()))
    {
    case 261:
    {
        static ItemTransforms* bow =
            (new ItemTransforms())
                ->set_thirdperson_righthand(-80.0f, 260.0f, -40.0f, -1.0f, -2.0f, 2.5f, 0.9f, 0.9f, 0.9f)
                ->set_thirdperson_lefthand(-80.0f, -280.0f, 40.0f, -1.0f, -2.0f, 2.5f, 0.9f, 0.9f, 0.9f)
                ->set_firstperson_righthand(0.0f, -90.0f, 25.0f, 1.13f, 3.2f, 1.13f, 0.68f, 0.68f, 0.68f)
                ->set_firstperson_lefthand(0.0f, 90.0f, -25.0f, 1.13f, 3.2f, 1.13f, 0.68f, 0.68f, 0.68f);
        return bow;
    }
    case 397:
    {
        static ItemTransforms* skull = (new ItemTransforms())
                                           ->set_gui(30.0f, 45.0f, 0.0f, 0.0f, 3.0f, 0.0f, 1.0f, 1.0f, 1.0f)
                                           ->set_fixed(0.0f, 180.0f, 0.0f, 0.0f, 4.0f, 0.0f, 1.0f, 1.0f, 1.0f)
                                           ->set_ground(0.0f, 0.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.5f, 0.5f, 0.5f)
                                           ->set_thirdperson_hands(45.0f, 45.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.5f, 0.5f,
                                                                   0.5f);
        if (item->getAuxValue() == 5)
        {
            static ItemTransforms* dragonHead =
                (new ItemTransforms(*skull))->set_gui(30.0f, 45.0f, 0.0f, -2.0f, 2.0f, 0.0f, 0.6f, 0.6f, 0.6f);
            if (!SkullBlockRenderer::instance->dragonJawCheck())
            {
                dragonHead->set_gui(30.0f, 45.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.6f, 0.6f, 0.6f)
                    ->set_fixed(0.0f, 180.0f, 0.0f, 0.0f, 2.0f, -3.0f, 1.0f, 1.0f, 1.0f);
                return dragonHead;
            }
            dragonHead->set_gui(30.0f, 45.0f, 0.0f, -2.0f, 2.0f, 0.0f, 0.6f, 0.6f, 0.6f);
            return dragonHead;
        }
        return skull;
    }
    case 442:
    {
        static ItemTransforms* shield =
            (new ItemTransforms())
                ->set_thirdperson_righthand(0.0f, 90.0f, 0.0f, 10.0f, 6.0f, -4.0f, 1.0f, 1.0f, 1.0f)
                ->set_thirdperson_lefthand(0.0f, 90.0f, 0.0f, 10.0f, 6.0f, 12.0f, 1.0f, 1.0f, 1.0f)
                ->set_firstperson_righthand(0.0f, 180.0f, 5.0f, -10.0f, 2.0f, -10.0f, 1.25f, 1.25f, 1.25f)
                ->set_firstperson_lefthand(0.0f, 180.0f, 5.0f, 10.0f, 0.0f, -10.0f, 1.25f, 1.25f, 1.25f)
                ->set_gui(15.0f, -25.0f, -5.0f, 3.0f, 3.0f, 0.0f, 0.65f, 0.65f, 0.65f)
                ->set_fixed(0.0f, 180.0f, 0.0f, -4.0f, 4.0f, -5.0f, 0.5f, 0.5f, 0.5f)
                ->set_ground(0.0f, 0.0f, 0.0f, 2.0f, 4.0f, 2.0f, 0.25f, 0.25f, 0.25f);
        static ItemTransforms* blockingShield =
            (new ItemTransforms())
                ->set_thirdperson_righthand(45.0f, 135.0f, 0.0f, 3.51f, 11.0f, -2.0f, 1.0f, 1.0f, 1.0f)
                ->set_thirdperson_lefthand(45.0f, 135.0f, 0.0f, 13.51f, 3.0f, 5.0f, 1.0f, 1.0f, 1.0f)
                ->set_firstperson_righthand(0.0f, 180.0f, -5.0f, -15.0f, 5.0f, -11.0f, 1.25f, 1.25f, 1.25f)
                ->set_firstperson_lefthand(0.0f, 180.0f, -5.0f, 5.0f, 5.0f, -11.0f, 1.25f, 1.25f, 1.25f)
                ->set_gui(15.0f, -25.0f, -5.0f, 2.0f, 3.0f, 0.0f, 0.65f, 0.65f, 0.65f);
        if (entity != NULL && entity->isBlocking())
            return blockingShield;
        return shield;
    }
    case 425:
    {
        static ItemTransforms* banner =
            (new ItemTransforms())
                ->set_thirdperson_hands(0.0f, 90.0f, 0.0f, 0.0f, 2.0f, 0.5f, 0.375f, 0.375f, 0.375f)
                ->set_firstperson_hands(0.0f, 90.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.375f, 0.375f, 0.375f)
                ->set_gui(30.0f, 20.0f, 0.0f, 0.0f, -3.25f, 0.0f, 0.5325f, 0.5325f, 0.5325f)
                ->set_ground(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.25f, 0.25f, 0.25f)
                ->set_head(0.0f, 180.0f, 0.0f, 0.0f, 16.0f, 7.0f, 1.5f, 1.5f, 1.5f)
                ->set_fixed(0.0f, 180.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f);
        return banner;
    }
    case 346:
    case 398:
    {
        static ItemTransforms* rod =
            (new ItemTransforms())
                ->set_thirdperson_righthand(0.0f, 90.0f, 55.0f, 0.0f, 4.0f, 2.5f, 0.85f, 0.85f, 0.85f)
                ->set_thirdperson_lefthand(0.0f, -90.0f, -55.0f, 0.0f, 4.0f, 2.5f, 0.85f, 0.85f, 0.85f)
                ->set_firstperson_righthand(0.0f, 90.0f, 25.0f, 0.0f, 1.6f, 0.8f, 0.68f, 0.68f, 0.68f)
                ->set_firstperson_lefthand(0.0f, -90.0f, -25.0f, 0.0f, 1.6f, 0.8f, 0.68f, 0.68f, 0.68f)
                ->set_ground(0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.5f, 0.5f, 0.5f)
                ->set_head(0.0f, 180.0f, 0.0f, 0.0f, 13.0f, 7.0f, 1.0f, 1.0f, 1.0f);
        return rod;
    }
    case 390:
    {
        static ItemTransforms* flowerPot = (new ItemTransforms(*GetDefaultTransforms()))
                                               ->set_thirdperson_hands(0.0f, 0.0f, 0.0f, 0.0f, 2.75f, 1.0f, 0.55f,
                                                                       0.55f, 0.55f);
        return flowerPot;
    }
    case 371:
    {
        static ItemTransforms* goldNugget = (new ItemTransforms(*GetDefaultTransforms()))
                                                ->set_thirdperson_hands(0.0f, 0.0f, 0.0f, 0.0f, 2.2f, 1.0f, 0.55f,
                                                                        0.55f, 0.55f);
        return goldNugget;
    }
    case 452:
    {
        static ItemTransforms* ironNugget = (new ItemTransforms(*GetDefaultTransforms()))
                                                ->set_thirdperson_hands(0.0f, 0.0f, 0.0f, 0.0f, 1.65f, 1.0f, 0.55f,
                                                                        0.55f, 0.55f);
        return ironNugget;
    }
    case 281:
    {
        static ItemTransforms* bowl = (new ItemTransforms(*GetDefaultTransforms()))
                                          ->set_thirdperson_hands(0.0f, 0.0f, 0.0f, 0.0f, 2.7f, 1.0f, 0.55f, 0.55f,
                                                                  0.55f);
        return bowl;
    }
    case 454:
    {
        static ItemTransforms* trident =
            (new ItemTransforms(*GetDefaultTransforms()))
                ->set_firstperson_righthand(0.0f, -90.0f, -155.0f, -3.0f, -4.0f, -10.0f, 1.0f, 1.0f, 1.0f)
                ->set_thirdperson_righthand(0.0f, -25.0f, -180.0f, -10.5f, 0.0f, 5.0f, 1.0f, 1.0f, 1.0f);
        static ItemTransforms* tridentThrowing =
            (new ItemTransforms(*GetDefaultTransforms()))
                ->set_firstperson_righthand(0.0f, -90.0f, -155.0f, -3.0f, -4.0f, -10.0f, 1.0f, 1.0f, 1.0f)
                ->set_thirdperson_righthand(0.0f, -25.0f, 0.0f, 4.0f, 0.0f, 12.0f, 1.0f, 1.0f, 1.0f);
        static ItemTransforms* tridentSneaking =
            (new ItemTransforms(*GetDefaultTransforms()))
                ->set_firstperson_righthand(0.0f, -90.0f, -155.0f, -3.0f, -4.0f, -10.0f, 1.0f, 1.0f, 1.0f)
                ->set_thirdperson_righthand(0.0f, -25.0f, -180.0f, -10.5f, 0.0f, 5.0f, 1.0f, 1.0f, 1.0f);
        static ItemTransforms* tridentSneakingThrowing =
            (new ItemTransforms(*GetDefaultTransforms()))
                ->set_firstperson_righthand(0.0f, -90.0f, -155.0f, -3.0f, -4.0f, -10.0f, 1.0f, 1.0f, 1.0f)
                ->set_thirdperson_righthand(0.0f, -25.0f, 0.0f, 4.0f, 0.0f, 18.0f, 1.0f, 1.0f, 1.0f);
        if (entity != NULL)
        {
            if (entity->isSneaking())
            {
                if (entity->isUsingItem())
                    return tridentSneakingThrowing;
                return tridentSneaking;
            }
            if (entity->isUsingItem())
                return tridentThrowing;
        }
        return trident;
    }
    default:
        break;
    }

    Block* block = Block::byItem(item->getItem());
    if (item->getItem()->getIconType() == 0 && block != NULL
        && BlockRenderer::canRender(block->defaultBlockState()->getRenderShape()))
    {
        static ItemTransforms* blockTransforms =
            (new ItemTransforms())
                ->set_gui(30.0f, 225.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.55f, 0.55f, 0.55f)
                ->set_ground(0.0f, 0.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.25f, 0.75f, 0.25f)
                ->set_fixed(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f)
                ->set_thirdperson_hands(75.0f, 45.0f, 0.0f, 0.0f, 2.5f, 0.0f, 0.375f, 0.375f, 0.375f)
                ->set_firstperson_righthand(0.0f, 45.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.4f, 0.4f)
                ->set_firstperson_lefthand(0.0f, 225.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.4f, 0.4f);

        int renderShape = block->defaultBlockState()->getRenderShape();
        if (renderShape == BLOCK_RENDER_WALL)
        {
            static ItemTransforms* transforms = (new ItemTransforms(*blockTransforms))
                                                    ->set_gui(30.0f, 135.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.55f, 0.55f,
                                                              0.55f)
                                                    ->set_fixed(0.0f, 90.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f);
            return transforms;
        }
        if (renderShape == BLOCK_RENDER_FENCE)
        {
            static ItemTransforms* transforms = (new ItemTransforms(*blockTransforms))
                                                    ->set_gui(30.0f, 135.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.625f, 0.625f,
                                                              0.625f)
                                                    ->set_fixed(0.0f, 90.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f);
            return transforms;
        }
        if (renderShape == BLOCK_RENDER_FENCE_GATE)
        {
            static ItemTransforms* transforms =
                (new ItemTransforms(*blockTransforms))
                    ->set_gui(30.0f, 45.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.8f, 0.8f, 0.8f)
                    ->set_fixed(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f)
                    ->set_thirdperson_hands(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.375f, 0.375f, 0.375f)
                    ->set_firstperson_righthand(0.0f, -45.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.4f, 0.4f);
            return transforms;
        }
        if (renderShape == BLOCK_RENDER_STAIRS)
        {
            static ItemTransforms* transforms =
                (new ItemTransforms(*blockTransforms))
                    ->set_gui(30.0f, 135.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.55f, 0.55f, 0.55f)
                    ->set_head(0.0f, -90.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f)
                    ->set_thirdperson_lefthand(75.0f, -135.0f, 0.0f, 0.0f, 2.5f, 0.0f, 0.375f, 0.375f, 0.375f);
            return transforms;
        }
        if (renderShape == BLOCK_RENDER_ANVIL)
        {
            static ItemTransforms* transforms = (new ItemTransforms(*blockTransforms))
                                                    ->set_fixed(0.0f, 90.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f);
            return transforms;
        }
        if (renderShape == BLOCK_RENDER_END_ROD)
        {
            static ItemTransforms* transforms = (new ItemTransforms(*blockTransforms))
                                                    ->set_thirdperson_hands(30.0f, 45.0f, 0.0f, 0.0f, 2.0f, 1.0f,
                                                                            0.375f, 0.375f, 0.375f);
            return transforms;
        }
        if (renderShape == BLOCK_RENDER_CONDUIT)
        {
            static ItemTransforms* transforms =
                (new ItemTransforms())
                    ->set_ground(0.0f, 0.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.5f, 0.5f, 0.5f)
                    ->set_gui(30.0f, 45.0f, 0.0f, 0.0f, 3.0f, 0.0f, 1.0f, 1.0f, 1.0f)
                    ->set_thirdperson_righthand(45.0f, 45.0f, 0.0f, 0.0f, 2.0f, 1.0f, 0.5f, 0.5f, 0.5f)
                    ->set_firstperson_righthand(0.0f, 45.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.4f, 0.4f, 0.4f)
                    ->set_fixed(0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 1.0f, 1.0f, 1.0f);
            return transforms;
        }
        if (renderShape == BLOCK_RENDER_TURTLE_EGG)
        {
            static ItemTransforms* transforms =
                (new ItemTransforms(*blockTransforms))
                    ->set_thirdperson_righthand(45.0f, 45.0f, 0.0f, 0.0f, 2.0f, 1.0f, 0.5f, 0.5f, 0.5f)
                    ->set_firstperson_righthand(0.0f, 45.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.4f, 0.4f, 0.4f)
                    ->set_fixed(0.0f, 0.0f, 0.0f, 0.0f, 2.5f, 0.0f, 0.5f, 0.5f, 0.5f);
            return transforms;
        }

        if (block == Blocks::stone_button || block == Blocks::wooden_button || block == Blocks::acacia_button
            || block == Blocks::birch_button || block == Blocks::dark_oak_button || block == Blocks::jungle_button
            || block == Blocks::spruce_button)
        {
            static ItemTransforms* button = (new ItemTransforms(*blockTransforms))
                                                ->set_thirdperson_hands(75.0f, 45.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.375f,
                                                                        0.375f, 0.375f);
            return button;
        }

        if (block == Blocks::pumpkin || block == Blocks::carved_pumpkin || block == Blocks::lit_pumpkin
            || block == Blocks::dropper || block == Blocks::dispenser || block == Blocks::furnace
            || block == Blocks::lit_furnace || block == Blocks::white_glazed_terracotta
            || block == Blocks::orange_glazed_terracotta || block == Blocks::magenta_glazed_terracotta
            || block == Blocks::light_blue_glazed_terracotta || block == Blocks::yellow_glazed_terracotta
            || block == Blocks::lime_glazed_terracotta || block == Blocks::pink_glazed_terracotta
            || block == Blocks::gray_glazed_terracotta || block == Blocks::silver_glazed_terracotta
            || block == Blocks::cyan_glazed_terracotta || block == Blocks::purple_glazed_terracotta
            || block == Blocks::blue_glazed_terracotta || block == Blocks::brown_glazed_terracotta
            || block == Blocks::green_glazed_terracotta || block == Blocks::red_glazed_terracotta
            || block == Blocks::black_glazed_terracotta || block == Blocks::observer)
        {
            static ItemTransforms* facingBlock = (new ItemTransforms(*blockTransforms))
                                                     ->set_firstperson_hands(0.0f, 135.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f,
                                                                             0.4f, 0.4f);
            return facingBlock;
        }
        return blockTransforms;
    }

    if (item->getItem()->isHandEquipped())
    {
        static ItemTransforms* handEquipped =
            (new ItemTransforms())
                ->set_thirdperson_righthand(0.0f, -90.0f, 55.0f, 0.0f, 4.0f, 0.5f, 0.85f, 0.85f, 0.85f)
                ->set_thirdperson_lefthand(0.0f, 90.0f, -55.0f, 0.0f, 4.0f, 0.5f, 0.85f, 0.85f, 0.85f)
                ->set_firstperson_righthand(0.0f, -90.0f, 25.0f, 1.13f, 3.2f, 1.13f, 0.68f, 0.68f, 0.68f)
                ->set_firstperson_lefthand(0.0f, 90.0f, -25.0f, 1.13f, 3.2f, 1.13f, 0.68f, 0.68f, 0.68f)
                ->set_ground(0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.5f, 0.5f, 0.5f)
                ->set_head(0.0f, 180.0f, 0.0f, 0.0f, 13.0f, 7.0f, 1.0f, 1.0f, 1.0f);
        return handEquipped;
    }
    return GetDefaultTransforms();
}

void ItemTransforms::apply(ItemTransform* transform, bool leftHand)
{
    if (transform == ItemTransform::NO_TRANSFORM)
        return;

    int side = leftHand ? -1 : 1;
    GlStateManager::translatef(static_cast<float>(side) * (offsetTranslateX + transform->translation.x),
                               offsetTranslateY + transform->translation.y,
                               offsetTranslateZ + transform->translation.z);
    float rotationX = offsetRotationX + transform->rotation.x;
    float rotationY = offsetRotationY + transform->rotation.y;
    float rotationZ = offsetRotationZ + transform->rotation.z;
    if (leftHand)
    {
        rotationY = -rotationY;
        rotationZ = -rotationZ;
    }
    GlStateManager::rotatef(rotationX, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(rotationY, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(rotationZ, 0.0f, 0.0f, 1.0f);
    GlStateManager::scalef(offsetScaleX + transform->scale.x, offsetScaleY + transform->scale.y,
                           offsetScaleZ + transform->scale.z);
}

void ItemTransforms::apply(TransformType type)
{
    apply(getTransform(type), false);
}

bool ItemTransforms::hasTransform(TransformType type)
{
    return getTransform(type) != ItemTransform::NO_TRANSFORM;
}
