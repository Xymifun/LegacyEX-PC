#include "game/core/DamageSource.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/core/Explosion.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/IndirectEntityDamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/entity/projectile/Fireball.hpp"
#include "game/network/ClientboundDamageIndicatorPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/Player.hpp"

const DamageSource* const DamageSource::IN_FIRE =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathInFire,
                      ClientboundChatPacket::eChatPacketMessage_DeathInFireFighting))
        ->setIsFire();
const DamageSource* const DamageSource::LIGHTNING_BOLT =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathLightningBolt,
                      ClientboundChatPacket::eChatPacketMessage_DeathLightningBoltAlternate))
        ->bypassGraceInvul();
const DamageSource* const DamageSource::ON_FIRE =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathOnFire,
                      ClientboundChatPacket::eChatPacketMessage_DeathOnFireFighting))
        ->bypassArmor()
        ->setIsFire();
const DamageSource* const DamageSource::LAVA =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathLava,
                      ClientboundChatPacket::eChatPacketMessage_DeathLavaEscaping))
        ->setIsFire();
const DamageSource* const DamageSource::HOT_FLOOR =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathHotFloor,
                      ClientboundChatPacket::eChatPacketMessage_DeathHotFloorEscaping))
        ->setIsFire();
const DamageSource* const DamageSource::IN_WALL =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathInWall))->bypassArmor();
const DamageSource* const DamageSource::CRAMMING =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathCramming))->bypassArmor();
const DamageSource* const DamageSource::DROWN =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathDrown,
                      ClientboundChatPacket::eChatPacketMessage_DeathDrownEscaping))
        ->bypassArmor();
const DamageSource* const DamageSource::STARVE =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathStarve))->bypassArmor()->bypassMagic();
const DamageSource* const DamageSource::CACTUS = new DamageSource(
    ClientboundChatPacket::eChatPacketMessage_DeathCactus,
    ClientboundChatPacket::eChatPacketMessage_DeathCactusEscaping);
const DamageSource* const DamageSource::FALL =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathFall))->bypassArmor();
const DamageSource* const DamageSource::FLY_INTO_WALL =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathFlyIntoWall))->bypassArmor();
const DamageSource* const DamageSource::OUT_OF_WORLD =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathOutOfWorld))->bypassArmor()->bypassInvul();
const DamageSource* const DamageSource::CREATIVE_PLAYER =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathCustom))->bypassArmor()->bypassInvul();
const DamageSource* const DamageSource::GENERIC =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathGeneric))->bypassArmor();
const DamageSource* const DamageSource::MAGIC =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathMagic))->bypassArmor()->setMagic();
const DamageSource* const DamageSource::DRAGON_BREATH =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathDragonBreath))->bypassArmor();
const DamageSource* const DamageSource::WITHER =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathWither))->bypassArmor();
const DamageSource* const DamageSource::ANVIL = new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathAnvil);
const DamageSource* const DamageSource::FALLING_BLOCK = new DamageSource(
    ClientboundChatPacket::eChatPacketMessage_DeathFallingBlock);
const DamageSource* const DamageSource::FIREWORKS =
    (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathExplosionAlternate))->setExplosion();
const DamageSource* const DamageSource::ARROW = DamageSource::CreateArrow(boost::shared_ptr<Arrow>(),
                                                                          boost::shared_ptr<Entity>(), false);
const DamageSource* const DamageSource::THORNS = DamageSource::CreateThorns(boost::shared_ptr<Entity>());
const DamageSource* const DamageSource::POISON = DamageSource::MAGIC->copy();
const DamageSource* const DamageSource::INSTANT_DAMAGE = DamageSource::MAGIC->copy();
const DamageSource* const DamageSource::DRY_OUT = new DamageSource(
    ClientboundChatPacket::eChatPacketMessage_DeathDryOut);

DamageSource::DamageSource(ClientboundChatPacket::EChatPacketMessage messageValue,
                           ClientboundChatPacket::EChatPacketMessage deathMessageValue)
    : bypassArmorFlag(false),
      bypassInvulFlag(false),
      bypassGraceInvulFlag(false),
      bypassMagicFlag(false),
      foodExhaustion(FoodConstants::ExhaustionAttack()),
      fire(false),
      projectile(false),
      scalesDifficulty(false),
      magic(false),
      explosion(false),
      fireProjectile(false)
{
    message = messageValue;
    deathMessage = deathMessageValue;
}

DamageSource::DamageSource(const DamageSource& source)
    : message(source.message),
      deathMessage(source.deathMessage),
      bypassArmorFlag(source.bypassArmorFlag),
      bypassInvulFlag(source.bypassInvulFlag),
      bypassGraceInvulFlag(source.bypassGraceInvulFlag),
      bypassMagicFlag(source.bypassMagicFlag),
      foodExhaustion(source.foodExhaustion),
      fire(source.fire),
      projectile(source.projectile),
      scalesDifficulty(source.scalesDifficulty),
      magic(source.magic),
      explosion(source.explosion),
      fireProjectile(source.fireProjectile)
{
}

bool DamageSource::equals(const DamageSource* source) const
{
    return message == source->message && deathMessage == source->deathMessage;
}

EntityDamageSource* DamageSource::CreateMobAttack(boost::shared_ptr<LivingEntity> entity)
{
    return new EntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathMob,
                                  ClientboundChatPacket::eChatPacketMessage_DeathMob, entity);
}

DamageSource* DamageSource::setProjectile()
{
    projectile = true;
    return this;
}

DamageSource* DamageSource::CreateIndirectMobAttack(const boost::shared_ptr<Entity>& source,
                                                    const boost::shared_ptr<LivingEntity>& target, bool projectileValue)
{
    DamageSource* result = new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathMob,
                                                          ClientboundChatPacket::eChatPacketMessage_DeathMob, source,
                                                          target, 0);
    if (projectileValue)
        result->setProjectile();
    return result;
}

EntityDamageSource* DamageSource::CreatePlayerAttack(boost::shared_ptr<Player> player)
{
    return new EntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathPlayer,
                                  ClientboundChatPacket::eChatPacketMessage_DeathMobItem, player);
}

void DamageSource::setIsFireProjectile()
{
    fireProjectile = true;
}

DamageSource* DamageSource::CreateArrow(boost::shared_ptr<Arrow> arrow, boost::shared_ptr<Entity> owner, bool fireValue)
{
    DamageSource* result = (new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathArrow,
                                                           ClientboundChatPacket::eChatPacketMessage_DeathArrowItem,
                                                           arrow, owner, 0))
                               ->setProjectile();
    if (fireValue)
        result->setIsFireProjectile();
    return result;
}

DamageSource* DamageSource::CreateTrident(boost::shared_ptr<ThrownTrident> trident, boost::shared_ptr<Entity> owner)
{
    return (new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathTrident,
                                           ClientboundChatPacket::eChatPacketMessage_DeathTridentItem, trident, owner,
                                           0))
        ->setProjectile();
}

DamageSource* DamageSource::setIsFire()
{
    fire = true;
    return this;
}

DamageSource* DamageSource::CreateFireball(boost::shared_ptr<Fireball> fireball, boost::shared_ptr<Entity> owner)
{
    if (owner)
        return (new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathFireball,
                                               ClientboundChatPacket::eChatPacketMessage_DeathArrowItem, fireball,
                                               owner, 0))
            ->setIsFire()
            ->setProjectile();
    return (new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathOnFire,
                                           ClientboundChatPacket::eChatPacketMessage_DeathOnFire, fireball, fireball,
                                           0))
        ->setIsFire()
        ->setProjectile();
}

DamageSource* DamageSource::CreateThrown(boost::shared_ptr<Entity> projectileEntity, boost::shared_ptr<Entity> owner)
{
    return (new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathThrown,
                                           ClientboundChatPacket::eChatPacketMessage_DeathThrownItem, projectileEntity,
                                           owner, 0))
        ->setProjectile();
}

DamageSource* DamageSource::setMagic()
{
    magic = true;
    return this;
}
DamageSource* DamageSource::bypassArmor()
{
    bypassArmorFlag = true;
    foodExhaustion = 0.0f;
    return this;
}

DamageSource* DamageSource::CreateIndirectMagic(boost::shared_ptr<Entity> source, boost::shared_ptr<Entity> target,
                                                const DamageSource* baseSource)
{
    if (eTYPE_DERIVED_FROM(EntityType::LLAMA_SPIT, source->GetType()))
    {
        DamageSource* result = new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathPlayer,
                                                              ClientboundChatPacket::eChatPacketMessage_DeathMobItem,
                                                              source, target, baseSource);
        result->bypassArmor();
        result->setMagic();
        result->setProjectile();
        return result;
    }

    DamageSource* result = new IndirectEntityDamageSource(
        ClientboundChatPacket::eChatPacketMessage_DeathIndirectMagic,
        ClientboundChatPacket::eChatPacketMessage_DeathIndirectMagicItem, source, target, baseSource);
    result->bypassArmor();
    result->setMagic();
    return result;
}

EntityDamageSource* DamageSource::CreateThorns(boost::shared_ptr<Entity> entity)
{
    return static_cast<EntityDamageSource*>(
        (new EntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathThorns,
                                ClientboundChatPacket::eChatPacketMessage_DeathThorns, entity))
            ->setThorns()
            ->setMagic());
}

DamageSource* DamageSource::setExplosion()
{
    explosion = true;
    return this;
}
DamageSource* DamageSource::setScalesWithDifficulty()
{
    scalesDifficulty = true;
    return this;
}

DamageSource* DamageSource::CreateExplosion(Explosion* explosionValue)
{
    if (explosionValue != 0 && explosionValue->getSourceMob() && explosionValue->getDirectSource()
        && explosionValue->getDirectSource() != explosionValue->getSourceMob())
        return (new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathExplosionBy,
                                               ClientboundChatPacket::eChatPacketMessage_DeathExplosionBy,
                                               explosionValue->getDirectSource(), explosionValue->getSourceMob(), 0))
            ->setScalesWithDifficulty()
            ->setExplosion();

    if (!(explosionValue != 0 && explosionValue->getSourceMob()))
        return (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathExplosion,
                                 ClientboundChatPacket::eChatPacketMessage_Text))
            ->setScalesWithDifficulty()
            ->setExplosion();

    return (new EntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathExplosionBy,
                                   ClientboundChatPacket::eChatPacketMessage_DeathExplosionBy,
                                   explosionValue->getSourceMob()))
        ->setScalesWithDifficulty()
        ->setExplosion();
}

DamageSource* DamageSource::CreateExplosion(const boost::shared_ptr<LivingEntity>& entity)
{
    if (!entity)
        return (new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathExplosion,
                                 ClientboundChatPacket::eChatPacketMessage_Text))
            ->setScalesWithDifficulty()
            ->setExplosion();

    return (new EntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathExplosionBy,
                                   ClientboundChatPacket::eChatPacketMessage_DeathExplosionBy, entity))
        ->setScalesWithDifficulty()
        ->setExplosion();
}

boost::shared_ptr<Entity> DamageSource::getDirectEntity() const
{
    return getEntity();
}

boost::shared_ptr<Entity> DamageSource::getEntity() const
{
    return boost::shared_ptr<Entity>();
}
bool DamageSource::isProjectile() const
{
    return projectile;
}
bool DamageSource::isExplosion() const
{
    return explosion;
}
bool DamageSource::isFireProjectile() const
{
    return fireProjectile;
}
bool DamageSource::isBypassArmor() const
{
    return bypassArmorFlag;
}
float DamageSource::getFoodExhaustion() const
{
    return foodExhaustion;
}
bool DamageSource::isBypassInvul() const
{
    return bypassInvulFlag;
}
bool DamageSource::isBypassGraceInvul() const
{
    return bypassGraceInvulFlag;
}
bool DamageSource::isBypassMagic() const
{
    return bypassMagicFlag;
}
bool DamageSource::isMagic() const
{
    return magic;
}
bool DamageSource::scalesWithDifficulty() const
{
    return scalesDifficulty;
}

void DamageSource::sendDamageIndicatorUpdate(boost::shared_ptr<ServerPlayer> player) const
{
    boost::shared_ptr<Entity> source = getEntity();
    if (!source)
    {
        player->mConnection->send(
            boost::shared_ptr<ClientboundDamageIndicatorPacket>(new ClientboundDamageIndicatorPacket()));
    }
    else
    {
        Vec3* direction = Vec3::newTemp(player->mX - source->mX, 0.0, player->mZ - source->mZ);
        direction->normalize();
        player->mConnection->send(boost::shared_ptr<ClientboundDamageIndicatorPacket>(
            new ClientboundDamageIndicatorPacket((float)direction->x, (float)direction->z)));
    }
}

boost::shared_ptr<ClientboundChatPacket> DamageSource::getDeathMessagePacket(
    boost::shared_ptr<LivingEntity> entity) const
{
    boost::shared_ptr<LivingEntity> killer = entity->getKillCredit();
    if (killer)
    {
        std::wstring entityName = entity->getNetworkName();
        return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(
            entityName, deathMessage != ClientboundChatPacket::eChatPacketMessage_Text ? deathMessage : message, killer,
            killer->getNetworkName()));
    }

    std::wstring entityName = entity->getNetworkName();
    return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(entityName, message, -1));
}

bool DamageSource::isFire() const
{
    return fire;
}
ClientboundChatPacket::EChatPacketMessage DamageSource::getMsgId() const
{
    return message;
}
bool DamageSource::isCreativePlayer() const
{
    boost::shared_ptr<Entity> source = getEntity();
    return source && eTYPE_DERIVED_FROM(EntityType::PLAYER, source->GetType())
           && boost::dynamic_pointer_cast<Player>(source)->mAbilities.mInstabuild;
}
Vec3* DamageSource::getSourcePosition() const
{
    return 0;
}
const DamageSource* DamageSource::GetBaseDamageSource() const
{
    return this;
}
DamageSource* DamageSource::copy() const
{
    return new DamageSource(*this);
}
DamageSource* DamageSource::bypassInvul()
{
    bypassInvulFlag = true;
    return this;
}
DamageSource* DamageSource::bypassGraceInvul()
{
    bypassGraceInvulFlag = true;
    return this;
}
DamageSource* DamageSource::bypassMagic()
{
    bypassMagicFlag = true;
    foodExhaustion = 0.0f;
    return this;
}
