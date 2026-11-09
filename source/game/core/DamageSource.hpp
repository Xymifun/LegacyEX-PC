#ifndef GAME_CORE_DAMAGESOURCE_HPP
#define GAME_CORE_DAMAGESOURCE_HPP

#include <boost/shared_ptr.hpp>

#include "game/network/ClientboundChatPacket.hpp"

class Arrow;
class Entity;
class EntityDamageSource;
class Explosion;
class Fireball;
class LivingEntity;
class Player;
class ServerPlayer;
class ThrownTrident;
class Vec3;

class DamageSource
{
public:
    static const DamageSource* const IN_FIRE;
    static const DamageSource* const LIGHTNING_BOLT;
    static const DamageSource* const ON_FIRE;
    static const DamageSource* const LAVA;
    static const DamageSource* const HOT_FLOOR;
    static const DamageSource* const IN_WALL;
    static const DamageSource* const CRAMMING;
    static const DamageSource* const DROWN;
    static const DamageSource* const STARVE;
    static const DamageSource* const CACTUS;
    static const DamageSource* const FALL;
    static const DamageSource* const FLY_INTO_WALL;
    static const DamageSource* const OUT_OF_WORLD;
    static const DamageSource* const CREATIVE_PLAYER;
    static const DamageSource* const GENERIC;
    static const DamageSource* const MAGIC;
    static const DamageSource* const DRAGON_BREATH;
    static const DamageSource* const WITHER;
    static const DamageSource* const ANVIL;
    static const DamageSource* const FALLING_BLOCK;
    static const DamageSource* const FIREWORKS;
    static const DamageSource* const ARROW;
    static const DamageSource* const THORNS;
    static const DamageSource* const POISON;
    static const DamageSource* const INSTANT_DAMAGE;
    static const DamageSource* const DRY_OUT;

    DamageSource(
        ClientboundChatPacket::EChatPacketMessage message,
        ClientboundChatPacket::EChatPacketMessage deathMessage = ClientboundChatPacket::eChatPacketMessage_Text);
    DamageSource(const DamageSource& source);
    virtual ~DamageSource() {}
    bool equals(const DamageSource* source) const;
    static EntityDamageSource* CreateMobAttack(boost::shared_ptr<LivingEntity> entity);
    DamageSource* setProjectile();
    static DamageSource* CreateIndirectMobAttack(const boost::shared_ptr<Entity>& source,
                                                 const boost::shared_ptr<LivingEntity>& target, bool projectile);
    static EntityDamageSource* CreatePlayerAttack(boost::shared_ptr<Player> player);
    void setIsFireProjectile();
    static DamageSource* CreateArrow(boost::shared_ptr<Arrow> arrow, boost::shared_ptr<Entity> owner, bool fire);
    static DamageSource* CreateTrident(boost::shared_ptr<ThrownTrident> trident, boost::shared_ptr<Entity> owner);
    DamageSource* setIsFire();
    static DamageSource* CreateFireball(boost::shared_ptr<Fireball> fireball, boost::shared_ptr<Entity> owner);
    static DamageSource* CreateThrown(boost::shared_ptr<Entity> projectile, boost::shared_ptr<Entity> owner);
    DamageSource* setMagic();
    DamageSource* bypassArmor();
    static DamageSource* CreateIndirectMagic(boost::shared_ptr<Entity> source, boost::shared_ptr<Entity> target,
                                             const DamageSource* baseSource);
    static EntityDamageSource* CreateThorns(boost::shared_ptr<Entity> entity);
    DamageSource* setExplosion();
    DamageSource* setScalesWithDifficulty();
    static DamageSource* CreateExplosion(Explosion* explosion);
    static DamageSource* CreateExplosion(const boost::shared_ptr<LivingEntity>& entity);
    virtual boost::shared_ptr<Entity> getDirectEntity() const;
    virtual boost::shared_ptr<Entity> getEntity() const;
    bool isProjectile() const;
    bool isExplosion() const;
    bool isFireProjectile() const;
    bool isBypassArmor() const;
    float getFoodExhaustion() const;
    bool isBypassInvul() const;
    bool isBypassGraceInvul() const;
    bool isBypassMagic() const;
    bool isMagic() const;
    virtual bool scalesWithDifficulty() const;
    void sendDamageIndicatorUpdate(boost::shared_ptr<ServerPlayer> player) const;
    virtual boost::shared_ptr<ClientboundChatPacket> getDeathMessagePacket(
        boost::shared_ptr<LivingEntity> entity) const;
    bool isFire() const;
    ClientboundChatPacket::EChatPacketMessage getMsgId() const;
    bool isCreativePlayer() const;
    virtual Vec3* getSourcePosition() const;
    virtual const DamageSource* GetBaseDamageSource() const;
    virtual DamageSource* copy() const;
    DamageSource* bypassInvul();
    DamageSource* bypassGraceInvul();
    DamageSource* bypassMagic();

protected:
    ClientboundChatPacket::EChatPacketMessage message;
    ClientboundChatPacket::EChatPacketMessage deathMessage;
    bool bypassArmorFlag;
    bool bypassInvulFlag;
    bool bypassGraceInvulFlag;
    bool bypassMagicFlag;
    float foodExhaustion;
    bool fire;
    bool projectile;
    bool scalesDifficulty;
    bool magic;
    bool explosion;
    bool fireProjectile;
};

#endif
