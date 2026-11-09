#include "game/core/DispenserBootstrap.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"
#include "game/world/generation/Player.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Position.hpp"
#include "game/core/PrimedTnt.hpp"
#include "game/core/Snowball.hpp"
#include "game/core/ThrownEgg.hpp"
#include "game/core/ThrownExpBottle.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/projectile/AbstractProjectileDispenseBehavior.hpp"
#include "game/entity/projectile/AbstractRotatedItemProjectile.hpp"
#include "game/entity/projectile/Projectile.hpp"
#include "game/entity/projectile/SmallFireball.hpp"
#include "game/entity/projectile/SpectralArrow.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/BucketItem.hpp"
#include "game/item/DyeItem.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/DispenserBlockEntity.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PumpkinBlock.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/block/ShulkerBoxBlockEntity.hpp"
#include "game/world/block/SkullBlock.hpp"
#include "game/world/block/SkullBlockEntity.hpp"
#include "game/world/block/TntBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

void DispenserBootstrap::StaticCtor()
{
    class ArrowDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~ArrowDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position,
                                                            not_null_ptr<ItemInstance>)
        {
            boost::shared_ptr<TippableArrow> arrow(
                new TippableArrow(level, position->getX(), position->getY(), position->getZ()));
            arrow->setPickup(AbstractRotatedItemProjectile::ePickup_Allowed);
            return arrow;
        }
    };
    class TippedArrowDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~TippedArrowDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position,
                                                            not_null_ptr<ItemInstance> itemInstance)
        {
            boost::shared_ptr<TippableArrow> arrow(
                new TippableArrow(level, position->getX(), position->getY(), position->getZ()));
            arrow->setEffectsFromItem(itemInstance);
            arrow->setPickup(AbstractRotatedItemProjectile::ePickup_Allowed);
            return arrow;
        }
    };
    class SpectralArrowDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~SpectralArrowDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position,
                                                            not_null_ptr<ItemInstance>)
        {
            boost::shared_ptr<SpectralArrow> arrow(
                new SpectralArrow(level, position->getX(), position->getY(), position->getZ()));
            arrow->setPickup(AbstractRotatedItemProjectile::ePickup_Allowed);
            return arrow;
        }
    };
    class TridentDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~TridentDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position,
                                                            not_null_ptr<ItemInstance> itemInstance)
        {
            boost::shared_ptr<ThrownTrident> trident(
                new ThrownTrident(level, itemInstance, position->getX(), position->getY(), position->getZ()));
            trident->setPickup(AbstractRotatedItemProjectile::ePickup_Allowed);
            return trident;
        }
    };
    class EggDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~EggDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position,
                                                            not_null_ptr<ItemInstance>)
        {
            return boost::shared_ptr<Projectile>(
                new ThrownEgg(level, position->getX(), position->getY(), position->getZ()));
        }
    };
    class SnowballDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~SnowballDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position,
                                                            not_null_ptr<ItemInstance>)
        {
            return boost::shared_ptr<Projectile>(
                new Snowball(level, position->getX(), position->getY(), position->getZ()));
        }
    };
    class ExpBottleDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~ExpBottleDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position,
                                                            not_null_ptr<ItemInstance>)
        {
            return boost::shared_ptr<Projectile>(
                new ThrownExpBottle(level, position->getX(), position->getY(), position->getZ()));
        }
        virtual float getUncertainty() { return AbstractProjectileDispenseBehavior::getUncertainty() * 0.5F; }
        virtual float getPower() { return AbstractProjectileDispenseBehavior::getPower() * 1.25F; }
    };
    class ThrownPotionDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~ThrownPotionDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position,
                                                            not_null_ptr<ItemInstance> itemInstance)
        {
            return boost::shared_ptr<Projectile>(
                new ThrownPotion(level, position->getX(), position->getY(), position->getZ(), itemInstance->copy()));
        }
        virtual float getUncertainty() { return AbstractProjectileDispenseBehavior::getUncertainty() * 0.5F; }
        virtual float getPower() { return AbstractProjectileDispenseBehavior::getPower() * 1.25F; }
    };
    class PotionDispenseBehavior : public DispenseItemBehavior
    {
    public:
        virtual ~PotionDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> dispense(BlockSource* source, not_null_ptr<ItemInstance> item)
        {
            return mDefaultDispenseItemBehavior.dispense(source, item);
        }

    private:
        DefaultDispenseItemBehavior mDefaultDispenseItemBehavior;
    };
    class SpawnEggDispenseBehavior : public DefaultDispenseItemBehavior
    {
    public:
        virtual ~SpawnEggDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item,
                                                   eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            double x = source->getX() + direction->getStepX();
            float y = (float)source->getPos().y + 0.2f;
            double z = source->getZ() + direction->getStepZ();
            int error = 0;
            boost::shared_ptr<Entity> entity = SpawnEggItem::getMob(level, SpawnEggItem::getNamedIdFrom(item), x, y, z,
                                                                    &error, L"");
            if (entity)
            {
                if (direction == Direction::UP)
                {
                    entity->absMoveTo(entity->mX, entity->mY + 1.0, entity->mZ);
                }
                else if (direction == Direction::DOWN)
                {
                    entity->absMoveTo(entity->mX, entity->mY - std::ceil(static_cast<double>(entity->mBbHeight)),
                                      entity->mZ);
                }

                if (eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()))
                {
                    boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
                    level->addEntity(mob);
                    mob->playAmbientSound();
                    if (item->hasCustomHoverName())
                    {
                        mob->setCustomName(item->getHoverName(), item->getOwnerUUID(),
                                           item->wasCreatedByRestrictedPlayer());
                    }
                }
                outcome = eOUTCOME_Success;
                SpawnEggItem::updateCustomEntityTag(level, boost::shared_ptr<Player>(), item, entity);
                item->shrink(1);
                return item;
            }
            outcome = eOUTCOME_Failure;
            return item;
        }
    };
    class FireworksDispenseBehavior : public DefaultDispenseItemBehavior
    {
    public:
        virtual ~FireworksDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item,
                                                   eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            if (level->countInstanceOf(EntityType::PROJECTILE, false, NULL, NULL) >= MAX_PROJECTILES)
            {
                outcome = eOUTCOME_Failure;
                return item;
            }
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            double x = source->getX() + direction->getStepX();
            float y = source->getPos().y + direction->getStepY() + 0.2f;
            double z = source->getZ() + direction->getStepZ();
            boost::shared_ptr<FireworksRocketEntity> rocket(
                new FireworksRocketEntity(level, x, y, z, direction->getStepX(), direction->getStepY(),
                                          direction->getStepZ(), item, true, boost::shared_ptr<Player>()));
            source->getWorld()->addEntity(rocket);
            outcome = eOUTCOME_Success;
            item->shrink(1);
            return item;
        }
        virtual void playSound(BlockSource* source, eOUTCOME outcome)
        {
            if (outcome == eOUTCOME_Success)
                source->getWorld()->levelEvent(LevelEvent::SOUND_FIREWORK_SHOOT, source->getPos(), 0);
            else
                DefaultDispenseItemBehavior::playSound(source, outcome);
        }
    };
    class FireballDispenseBehavior : public DefaultDispenseItemBehavior
    {
    public:
        virtual ~FireballDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item,
                                                   eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            if (static_cast<unsigned int>(level->countInstanceOf(EntityType::SMALL_FIREBALL, true, NULL, NULL)) >= 200)
            {
                outcome = eOUTCOME_Failure;
                return item;
            }
            outcome = eOUTCOME_Success;
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            Position* position = DispenserBlock::getDispensePosition(source);
            double x = position->getX() + (double)((float)direction->getStepX() * 0.3f);
            double y = position->getY() + (double)((float)direction->getStepX() * 0.3f);
            double z = position->getZ() + (double)((float)direction->getStepZ() * 0.3f);
            delete position;
            Random* random = level->mRandom;
            double xd = random->nextGaussian() * 0.05 + direction->getStepX();
            double yd = random->nextGaussian() * 0.05 + direction->getStepY();
            double zd = random->nextGaussian() * 0.05 + direction->getStepZ();
            level->addEntity(boost::shared_ptr<SmallFireball>(new SmallFireball(level, x, y, z, xd, yd, zd)));
            item->shrink(1);
            return item;
        }
        virtual void playSound(BlockSource* source, eOUTCOME outcome)
        {
            if (outcome == eOUTCOME_Success)
                source->getWorld()->levelEvent(LevelEvent::SOUND_BLAZE_FIREBALL, source->getPos(), 0);
            else
                DefaultDispenseItemBehavior::playSound(source, outcome);
        }
    };
    class FilledBucketDispenseBehavior : public DefaultDispenseItemBehavior
    {
    public:
        virtual ~FilledBucketDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item,
                                                   eOUTCOME& outcome)
        {
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            BucketItem* bucket = static_cast<BucketItem*>(item->getItem());
            BlockPos position = source->getPos().relative(direction);
            boost::shared_ptr<Player> player = source->getWorld()->getNearestPlayer(
                static_cast<double>(source->getPos().x), static_cast<double>(source->getPos().y),
                static_cast<double>(source->getPos().z), 128.0, false, -1.0);
            if (!bucket->emptyBucket(player, source->getWorld(), position, item))
                return mDefaultDispenseItemBehavior.dispense(source, item);
            outcome = eOUTCOME_Success;
            return not_null_ptr<ItemInstance>(new ItemInstance(Items::bucket));
        }

    private:
        DefaultDispenseItemBehavior mDefaultDispenseItemBehavior;
    };
    class EmptyBucketDispenseBehavior : public DefaultDispenseItemBehavior
    {
    public:
        virtual ~EmptyBucketDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item,
                                                   eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            BlockPos position = source->getPos().relative(direction);
            bool extraLayer = false;
            const BlockState* state = level->getBlockState(position);
            Block* block = state->getBlock();
            if (!block->isLiquidBlock())
            {
                state = level->getExtraBlockState(position);
                block = state->getBlock();
                extraLayer = true;
            }
            Material* material = block->mMaterial;
            Item* filled;
            if (Material::WATER == material && block->isLiquidBlock() && state->getValue(LiquidBlock::LEVEL) == 0)
                filled = Items::waterBucket;
            else if (Material::LAVA == material && block->isLiquidBlock() && state->getValue(LiquidBlock::LEVEL) == 0)
                filled = Items::lavaBucket;
            else
                return DefaultDispenseItemBehavior::execute(source, item, outcome);
            if (extraLayer)
                level->setExtraBlock(position, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
            else
                level->removeBlock(position);
            item->shrink(1);
            if (item->isEmpty())
                return not_null_ptr<ItemInstance>(new ItemInstance(filled));
            bool full;
            {
                boost::shared_ptr<BlockEntity> blockEntity = source->getEntity();
                boost::shared_ptr<DispenserBlockEntity> dispenser = boost::static_pointer_cast<DispenserBlockEntity>(
                    blockEntity);
                full = dispenser->addItem(not_null_ptr<ItemInstance>(new ItemInstance(filled))) < 0;
            }
            if (full)
                mDefaultDispenseItemBehavior.dispense(source, not_null_ptr<ItemInstance>(new ItemInstance(filled)));
            outcome = eOUTCOME_Success;
            return item;
        }

    private:
        DefaultDispenseItemBehavior mDefaultDispenseItemBehavior;
    };
    class FlintAndSteelDispenseBehavior : public OptionalDispenseItemBehavior
    {
    public:
        virtual ~FlintAndSteelDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item,
                                                   eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            outcome = eOUTCOME_Success;
            BlockPos position = source->getPos().relative(source->getBlockState()->getValue(DispenserBlock::FACING));
            if (level->isEmptyBlock(position))
            {
                level->setBlockAndUpdate(position, Blocks::fire->defaultBlockState());
                if (item->hurt(1, level->mRandom))
                    item->setCount(0);
            }
            else if (level->getBlockState(position)->getBlock() == Blocks::tnt)
            {
                Blocks::tnt->destroy(level, position,
                                     Blocks::tnt->defaultBlockState()->setValue(TntBlock::EXPLODE, true));
                level->removeBlock(position);
            }
            else
                outcome = eOUTCOME_Failure;
            return item;
        }
    };
    class DyeDispenseBehavior : public OptionalDispenseItemBehavior
    {
    public:
        virtual ~DyeDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item,
                                                   eOUTCOME& outcome)
        {
            outcome = eOUTCOME_Success;
            if (DyeColor::WHITE == DyeColor::byItemData(item->getAuxValue()))
            {
                Level* level = source->getWorld();
                const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
                BlockPos position = source->getPos().relative(direction);
                if (DyeItem::growCrop(item, level, position, false))
                {
                    if (!level->mIsClientSide)
                        level->levelEvent(LevelEvent::PARTICLES_PLANT_GROWTH, position, 0);
                    outcome = eOUTCOME_Success;
                }
                else
                    outcome = eOUTCOME_Failure;
                return item;
            }
            return DefaultDispenseItemBehavior::execute(source, item, outcome);
        }
    };
    class TntDispenseBehavior : public DefaultDispenseItemBehavior
    {
    public:
        virtual ~TntDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item,
                                                   eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            if (level->newPrimedTntAllowed() && gApp.GetGameHostOption(eGameHostOption_TNT))
            {
                const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
                BlockPos position = source->getPos().relative(direction);
                boost::shared_ptr<PrimedTnt> tnt(new PrimedTnt(level, position.x + 0.5, position.y, position.z + 0.5,
                                                               boost::shared_ptr<LivingEntity>()));
                level->addEntity(tnt);
                level->playSound(boost::shared_ptr<Player>(), tnt->mX, tnt->mY, tnt->mZ, SoundEvents::TNT_PRIMED,
                                 SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, 16.0f);
                outcome = eOUTCOME_Success;
                item->shrink(1);
            }
            else
            {
                outcome = eOUTCOME_Failure;
            }
            return item;
        }
    };
    class SkullDispenseBehavior : public OptionalDispenseItemBehavior
    {
    public:
        virtual ~SkullDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item,
                                                   eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            BlockPos position = source->getPos().relative(direction);
            SkullBlock* block = static_cast<SkullBlock*>(Blocks::skull);
            if (level->isEmptyBlock(position) && block->canSpawnMob(level, position, item))
            {
                if (!level->mIsClientSide)
                {
                    level->setBlock(position, block->defaultBlockState()->setValue(SkullBlock::FACING, Direction::UP),
                                    Block::UPDATE_ALL, false);
                    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(position);
                    bool isSkull = eTYPE_DERIVED_FROM(BlockEntityType::SKULL, blockEntity->GetType());
                    if (isSkull)
                    {
                        boost::shared_ptr<SkullBlockEntity> skull = boost::static_pointer_cast<SkullBlockEntity>(
                            blockEntity);
                        skull->setSkullType(item->getAuxValue(), L"");
                        skull->setRotation(direction->getOpposite()->get2DDataValue() * 4);
                        block->checkMobSpawn(level, position, skull, boost::shared_ptr<Player>());
                    }
                    item->shrink(1);
                    outcome = eOUTCOME_Success;
                }
            }
            else if (ArmorItem::dispenseArmor(source, item, outcome)->isEmpty())
            {
                item = DefaultDispenseItemBehavior::execute(source, item, outcome);
            }
            return item;
        }
    };
    class PumpkinDispenseBehavior : public OptionalDispenseItemBehavior
    {
    public:
        virtual ~PumpkinDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item,
                                                   eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            BlockPos position = source->getPos().relative(direction);
            PumpkinBlock* block = static_cast<PumpkinBlock*>(Blocks::pumpkin);
            outcome = eOUTCOME_Success;
            if (level->isEmptyBlock(position) && block->canSpawnGolem(level, position))
            {
                if (!level->mIsClientSide)
                    level->setBlock(position, block->defaultBlockState(), Block::UPDATE_ALL, false);
                item->shrink(1);
            }
            else
            {
                not_null_ptr<ItemInstance> armor = ArmorItem::dispenseArmor(source, item, outcome);
                if (armor->isEmpty())
                {
                    DefaultDispenseItemBehavior::execute(source, item, outcome);
                }
            }
            return item;
        }
        virtual void playSound(BlockSource* source, eOUTCOME outcome)
        {
            if (outcome == eOUTCOME_Success)
                source->getWorld()->levelEvent(LevelEvent::SOUND_DISPENSER_DISPENSE, source->getPos(), 0);
            else
                source->getWorld()->levelEvent(LevelEvent::SOUND_DISPENSER_FAIL, source->getPos(), 0);
        }
    };

    DispenserBlock::DISPENSER_REGISTRY.add(Items::arrow, new ArrowDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::tippedArrow, new TippedArrowDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::spectralArrow, new SpectralArrowDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::trident, new TridentDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::egg, new EggDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::snowball, new SnowballDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::experienceBottle, new ExpBottleDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::splashPotion, new ThrownPotionDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::lingeringPotion, new ThrownPotionDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::potion, new PotionDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::spawnEgg, new SpawnEggDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::fireworks, new FireworksDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::fireCharge, new FireballDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::boat, new BoatDispenseBehavior(Boat::OAK));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::spruceBoat, new BoatDispenseBehavior(Boat::SPRUCE));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::birchBoat, new BoatDispenseBehavior(Boat::BIRCH));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::jungleBoat, new BoatDispenseBehavior(Boat::JUNGLE));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::darkOakBoat, new BoatDispenseBehavior(Boat::DARK_OAK));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::acaciaBoat, new BoatDispenseBehavior(Boat::ACACIA));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::lavaBucket, new FilledBucketDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::waterBucket, new FilledBucketDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::fishBucket, new FilledBucketDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::pufferBucket, new FilledBucketDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::salmonBucket, new FilledBucketDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::tropicalBucket, new FilledBucketDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::bucket, new EmptyBucketDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::flintAndSteel, new FlintAndSteelDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::dye, new DyeDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::tnt), new TntDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::skull, new SkullDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::pumpkin), new PumpkinDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::carved_pumpkin), new PumpkinDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::lit_pumpkin), new PumpkinDispenseBehavior());
    for (std::vector<const DyeColor*>::iterator it = DyeColor::COLORS.begin(); it != DyeColor::COLORS.end(); ++it)
    {
        DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(ShulkerBoxBlock::getBlockByColor(*it)),
                                               new ShulkerBoxDispenseBehavior());
    }
}

DispenserBootstrap::BoatDispenseBehavior::BoatDispenseBehavior(Boat::Type type) : mType(type)
{
}

DispenserBootstrap::BoatDispenseBehavior::~BoatDispenseBehavior()
{
}

not_null_ptr<ItemInstance> DispenserBootstrap::BoatDispenseBehavior::execute(BlockSource* source,
                                                                             not_null_ptr<ItemInstance> item,
                                                                             eOUTCOME& outcome)
{
    const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
    Level* level = source->getWorld();
    double x = source->getX() + direction->getStepX() * 1.125F;
    double y = source->getY() + direction->getStepY() * 1.125F;
    double z = source->getZ() + direction->getStepZ() * 1.125F;
    BlockPos position = source->getPos().relative(direction);
    Material* material = level->getBlockState(position)->getBlock()->mMaterial;
    double yOffset;
    if (static_cast<unsigned int>(level->countInstanceOf(EntityType::BOAT, true, NULL, NULL)) >= 40)
        return mDefaultDispenseItemBehavior.dispense(source, item);
    if (material == Material::WATER)
    {
        yOffset = 1.0;
    }
    else if (material == Material::AIR
             && level->getBlockState(position.below())->getBlock()->mMaterial == Material::WATER)
    {
        yOffset = 0.0;
    }
    else
    {
        return mDefaultDispenseItemBehavior.dispense(source, item);
    }
    outcome = eOUTCOME_Success;
    boost::shared_ptr<Boat> boat(new Boat(level, x, y + yOffset, z));
    boat->setType(mType);
    boat->mYRot = direction->getOpposite()->toYRot();
    level->addEntity(boat);
    item->shrink(1);
    return item;
}

void DispenserBootstrap::BoatDispenseBehavior::playSound(BlockSource* source, eOUTCOME outcome)
{
    DefaultDispenseItemBehavior::playSound(source, outcome);
}

void DispenserBootstrap::OptionalDispenseItemBehavior::playSound(BlockSource* source, eOUTCOME outcome)
{
    source->getWorld()->levelEvent(outcome == eOUTCOME_Failure ? LevelEvent::SOUND_DISPENSER_DISPENSE
                                                               : LevelEvent::SOUND_DISPENSER_FAIL,
                                   source->getPos(), 0);
}

DispenserBootstrap::ShulkerBoxDispenseBehavior::~ShulkerBoxDispenseBehavior()
{
}

not_null_ptr<ItemInstance> DispenserBootstrap::ShulkerBoxDispenseBehavior::execute(BlockSource* source,
                                                                                   not_null_ptr<ItemInstance> item,
                                                                                   eOUTCOME&)
{
    Block* block = Block::byItem(item->getItem());
    Level* level = source->getWorld();
    const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
    BlockPos position = source->getPos().relative(direction);
    if (level->mayPlace(block, position, false, Direction::DOWN, boost::shared_ptr<Entity>()))
    {
        const Direction* facing = level->isEmptyBlock(position.below()) ? direction : Direction::UP;
        const BlockState* state = block->defaultBlockState()->setValue(DirectionalBlock::FACING, facing);
        level->setBlockAndUpdate(position, state);
        boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(position);
        not_null_ptr<ItemInstance> placed = item->split(1);
        if (placed->hasTag())
        {
            boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::static_pointer_cast<ShulkerBoxBlockEntity>(entity);
            box->loadFromTag(placed->getTag()->getCompound(BlockItem::BLOCK_ENTITY_TAG));
        }
        if (placed->hasCustomHoverName())
        {
            boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::static_pointer_cast<ShulkerBoxBlockEntity>(entity);
            box->setCustomName(placed->getHoverName(), placed->getOwnerUUID(), placed->wasCreatedByRestrictedPlayer(),
                               placed->wasCreatedOnHost());
        }
        level->updateNeighbourForOutputSignal(position, state->getBlock());
    }
    return item;
}
