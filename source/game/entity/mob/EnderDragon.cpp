#include "game/entity/mob/EnderDragon.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"
#include "game/world/generation/Player.hpp"

#include <cmath>

#include "game/data/DataFixerUpper.hpp"
#include "game/core/BinaryHeap.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/L10N.hpp"
#include "game/util/Vec3.hpp"
#include "game/core/Node.hpp"
#include "game/core/Path.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MultiEntityMobPart.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/MoverType.hpp"
#include "game/entity/projectile/DragonFireball.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/entity/mob/EndDragonFight.hpp"
#include "game/world/TheEndDimension.hpp"
#include "game/util/AABB.hpp"
#include "game/core/Mth.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"

#include <limits>
#include <typeinfo>

static const EntityDataAccessor<int>* DATA_DRAGON_PHASE = NULL;
static double SITTING_TARGET_RANGE = 20.0;

EnderDragon::EnderDragon(Level* level) : Mob(level)
{
    _init();
    setSize(16.0f, 8.0f);
    mNoPhysics = true;
    mFireImmune = true;
    mNoCulling = true;
    mYTarget = 100.0;
    mGrowlTime = 100;
    if (!level->mIsClientSide && dynamic_cast<TheEndDimension*>(level->mDimension) != NULL)
        mDragonFight = static_cast<TheEndDimension*>(level->mDimension)->getDragonFight();
    else
        mDragonFight = NULL;
}

EnderDragon::~EnderDragon()
{
    if (mNodes->data != NULL)
    {
        for (unsigned int i = 0; i < mNodes->length; ++i)
        {
            if (mNodes->data[i] != NULL)
                delete mNodes->data[i];
        }
        delete[] mNodes->data;
    }
    delete mOpenSet;
    if (mPath != NULL)
        delete mPath;
}

std::wstring EnderDragon::getDisplayName()
{
    if (hasCustomName())
        return getCustomName();
    return std::wstring(L10N::GetString(IDS_ENDERDRAGON));
}

float EnderDragon::getMaxHealth()
{
    return LivingEntity::getMaxHealth();
}
float EnderDragon::getHealth()
{
    return LivingEntity::getHealth();
}
int EnderDragon::getDimension()
{
    return mDimension;
}
Level* EnderDragon::getLevel()
{
    return mLevel;
}

unsigned int EnderDragon::getSynchedAction()
{
    return static_cast<unsigned int>(mEntityData->get(DATA_DRAGON_PHASE));
}

bool EnderDragon::setSynchedAction(EEnderdragonAction action, bool force)
{
    bool validTransition = false;
    switch (getSynchedAction())
    {
    case ACTION_HOLDING_PATTERN:
        switch (action)
        {
        case ACTION_STRAFE_PLAYER:
        case ACTION_LANDING_APPROACH:
            validTransition = true;
            break;
        }
        break;
    case ACTION_STRAFE_PLAYER:
        switch (action)
        {
        case ACTION_HOLDING_PATTERN:
        case ACTION_LANDING_APPROACH:
            validTransition = true;
            break;
        }
        break;
    case ACTION_LANDING_APPROACH:
        switch (action)
        {
        case ACTION_LANDING:
            validTransition = true;
            break;
        }
        break;
    case ACTION_LANDING:
        switch (action)
        {
        case ACTION_SITTING_FLAMING:
        case ACTION_SITTING_SCANNING:
            validTransition = true;
            break;
        }
        break;
    case ACTION_TAKEOFF:
        switch (action)
        {
        case ACTION_HOLDING_PATTERN:
            validTransition = true;
            break;
        }
        break;
    case ACTION_SITTING_FLAMING:
        switch (action)
        {
        case ACTION_TAKEOFF:
        case ACTION_SITTING_SCANNING:
        case ACTION_SITTING_ATTACKING:
            validTransition = true;
            break;
        }
        break;
    case ACTION_SITTING_SCANNING:
        switch (action)
        {
        case ACTION_TAKEOFF:
        case ACTION_SITTING_FLAMING:
        case ACTION_SITTING_ATTACKING:
            validTransition = true;
            break;
        }
        break;
    case ACTION_SITTING_ATTACKING:
        switch (action)
        {
        case ACTION_TAKEOFF:
        case ACTION_SITTING_FLAMING:
        case ACTION_SITTING_SCANNING:
            validTransition = true;
            break;
        }
        break;
    }

    if (validTransition || force)
        mEntityData->set(DATA_DRAGON_PHASE, static_cast<int>(action), true);

    return force || validTransition;
}

unsigned int EnderDragon::findClosestNode(double x, double y, double z)
{
    float closestDistance = 100.0f;
    unsigned int closest = 0;
    Node* target = new Node(static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y)),
                            static_cast<int>(std::floor(z)));
    int first = 0;
    if (mDragonFight == NULL || mDragonFight->getCrystalsAlive() == 0)
        first = OUTER_RING_NODE_COUNT;

    for (unsigned int i = first; i < PATH_NODE_COUNT; ++i)
    {
        if (mNodes->data[i] != NULL)
        {
            float distance = mNodes->data[i]->distanceTo(target);
            if (distance < closestDistance)
            {
                closestDistance = distance;
                closest = i;
            }
        }
    }

    delete target;
    return closest;
}

unsigned int EnderDragon::findClosestNode()
{
    if (mNodes->data[0] == NULL)
    {
        for (unsigned int i = 0; i < PATH_NODE_COUNT; ++i)
        {
            int heightOffset = 5;
            int x;
            int z;
            if (i < 12)
            {
                const int ringIndex = i;
                const float angle = 2.0f * (-3.1415927f + 0.2617994f * static_cast<float>(ringIndex));
                x = static_cast<int>(60.0f * Mth::cos(angle));
                z = static_cast<int>(60.0f * Mth::sin(angle));
            }
            else if (i < 20)
            {
                const int ringIndex = i - 12;
                const float angle = 2.0f * (-3.1415927f + 0.3926991f * static_cast<float>(ringIndex));
                x = static_cast<int>(40.0f * Mth::cos(angle));
                z = static_cast<int>(40.0f * Mth::sin(angle));
                heightOffset += 10;
            }
            else
            {
                const int ringIndex = i - 20;
                const float angle = 2.0f * (-3.1415927f + 0.7853982f * static_cast<float>(ringIndex));
                x = static_cast<int>(20.0f * Mth::cos(angle));
                z = static_cast<int>(20.0f * Mth::sin(angle));
            }

            const int minY = mLevel->mSeaLevel + 10;
            const BlockPos top = mLevel->getTopSolidBlockPos(BlockPos(x, 0, z));
            int y = top.y + heightOffset;
            if (y < minY)
                y = minY;
            mNodes->data[i] = new Node(x, y, z);
        }

        mNodeAdjacency[0] = 0x1802;
        mNodeAdjacency[1] = 0x2005;
        mNodeAdjacency[2] = 0x200a;
        mNodeAdjacency[3] = 0x4014;
        mNodeAdjacency[4] = 0x8028;
        mNodeAdjacency[5] = 0x8050;
        mNodeAdjacency[6] = 0x100a0;
        mNodeAdjacency[7] = 0x20140;
        mNodeAdjacency[8] = 0x20280;
        mNodeAdjacency[9] = 0x40500;
        mNodeAdjacency[10] = 0x80a00;
        mNodeAdjacency[11] = 0x80401;
        mNodeAdjacency[12] = 0x182001;
        mNodeAdjacency[13] = 0x305006;
        mNodeAdjacency[14] = 0x20a008;
        mNodeAdjacency[15] = 0x614030;
        mNodeAdjacency[16] = 0x428040;
        mNodeAdjacency[17] = 0xc50180;
        mNodeAdjacency[18] = 0x8a0200;
        mNodeAdjacency[19] = 0x941c00;
        mNodeAdjacency[20] = 0xe83000;
        mNodeAdjacency[21] = 0xd0e000;
        mNodeAdjacency[22] = 0xb38000;
        mNodeAdjacency[23] = 0x7e0000;
    }

    return findClosestNode(mX, mY, mZ);
}

double EnderDragon::getHeadYRotDiff(float)
{
    return 0.0;
}

Vec3* EnderDragon::getHeadLookVector(float partialTicks)
{
    Vec3* result;
    if (getSynchedAction() == ACTION_LANDING || getSynchedAction() == ACTION_TAKEOFF)
    {
        const BlockPos top = mLevel->getTopSolidBlockPos(BlockPos(0, 0, 0));
        float scale = static_cast<float>(std::sqrt(distanceToSqr(0.0, static_cast<double>(top.y), 0.0))) * 0.25f;
        if (scale < 1.0f)
            scale = 1.0f;

        const float pitch = 6.0f / scale;
        const float xRot = mXRot;
        const float yRot = mYRot;
        mXRot = -pitch * 1.5f * 5.0f;
        mYRot = yRot + static_cast<float>(getHeadYRotDiff(partialTicks));
        result = getViewVector(partialTicks);
        mXRot = xRot;
        mYRot = yRot;
    }
    else if (getSynchedAction() == ACTION_SITTING_FLAMING || getSynchedAction() == ACTION_SITTING_SCANNING
             || getSynchedAction() == ACTION_SITTING_ATTACKING)
    {
        const float xRot = mXRot;
        mXRot = -45.0f;
        const float yRot = mYRot;
        mYRot = mYRot + static_cast<float>(getHeadYRotDiff(partialTicks));
        result = getViewVector(partialTicks);
        mXRot = xRot;
        mYRot = yRot;
    }
    else
    {
        result = getViewVector(partialTicks);
    }
    return result;
}

Path* EnderDragon::reconstruct_path(Node*, Node* end)
{
    int count = 1;
    for (Node* node = end; node->cameFrom != NULL; node = node->cameFrom)
        ++count;

    arrayWithLength<Node*> nodes(count, true);
    Node* node = end;
    --count;
    for (nodes[count] = end; node->cameFrom != NULL; nodes[count] = node)
    {
        node = node->cameFrom;
        --count;
    }

    Path* path = new Path(nodes);
    delete[] nodes.data;
    return path;
}

Path* EnderDragon::findPath(int start, int end, Node* target)
{
    for (int i = 0; i < PATH_NODE_COUNT; ++i)
    {
        Node* node = mNodes->data[i];
        node->closed = false;
        node->f = 0.0f;
        node->g = 0.0f;
        node->h = 0.0f;
        node->cameFrom = NULL;
        node->heapIdx = -1;
    }

    Node* startNode = mNodes->data[start];
    Node* endNode = mNodes->data[end];
    startNode->g = 0.0f;
    startNode->h = static_cast<float>(startNode->distanceTo(endNode));
    startNode->f = startNode->h;
    mOpenSet->clear();
    mOpenSet->insert(startNode);

    Node* closest = startNode;
    int first = 0;
    if (mDragonFight != NULL && mDragonFight->getCrystalsAlive() == 0)
        first = OUTER_RING_NODE_COUNT;

    while (!mOpenSet->isEmpty())
    {
        Node* current = mOpenSet->pop();
        if (current->equals(endNode))
        {
            if (target != NULL)
            {
                target->cameFrom = endNode;
                endNode = target;
            }
            return reconstruct_path(startNode, endNode);
        }

        if (current->distanceTo(endNode) < closest->distanceTo(endNode))
            closest = current;
        current->closed = true;

        int currentIndex = 0;
        for (int i = 0; i < PATH_NODE_COUNT; ++i)
        {
            if (mNodes->data[i] == current)
            {
                currentIndex = i;
                break;
            }
        }

        for (int i = first; i < PATH_NODE_COUNT; ++i)
        {
            if ((mNodeAdjacency[currentIndex] & (1 << i)) == 0)
                continue;

            Node* neighbour = mNodes->data[i];
            if (neighbour->closed)
                continue;

            const float g = static_cast<float>(current->g + current->distanceTo(neighbour));
            if (!neighbour->inOpenSet() || g < neighbour->g)
            {
                neighbour->cameFrom = current;
                neighbour->g = g;
                neighbour->h = static_cast<float>(neighbour->distanceTo(endNode));
                if (neighbour->inOpenSet())
                {
                    mOpenSet->changeCost(neighbour, neighbour->g + neighbour->h);
                }
                else
                {
                    neighbour->f = neighbour->g + neighbour->h;
                    mOpenSet->insert(neighbour);
                }
            }
        }
    }

    if (closest == startNode)
        return NULL;

    if (target != NULL)
    {
        target->cameFrom = closest;
        closest = target;
    }
    return reconstruct_path(startNode, closest);
}

void EnderDragon::navigateToNextPathNode()
{
    if (mPath == NULL || mPath->isDone())
        return;

    Vec3* position = mPath->currentPos();
    mPath->next();
    mXTarget = position->x;
    if (getSynchedAction() == ACTION_LANDING_APPROACH && mPath->isDone())
    {
        mYTarget = position->y;
        mZTarget = position->z;
    }
    else
    {
        do
        {
            mYTarget = position->y + mRandom->nextFloat() * 20.0f;
        } while (mYTarget < position->y);
        mZTarget = position->z;
    }
}

void EnderDragon::strafeAttackTarget()
{
    const unsigned int start = findClosestNode();
    const unsigned int end = findClosestNode(mAttackTarget->mX, mAttackTarget->mY, mAttackTarget->mZ);
    const double targetX = mAttackTarget->mX;
    const double targetZ = mAttackTarget->mZ;
    const double dx = targetX - mX;
    const double dz = targetZ - mZ;
    double height = 0.4f + std::sqrt(dx * dx + dz * dz) / 80.0 - 1.0;
    if (height > 10.0)
        height = 10.0;

    Node target(static_cast<int>(targetX), static_cast<int>(mAttackTarget->mBoundingBox->mMinY + height),
                static_cast<int>(targetZ));
    if (mPath != NULL)
        delete mPath;
    mPath = findPath(start, end, &target);
    if (mPath != NULL)
    {
        mPath->next();
        navigateToNextPathNode();
    }
}

void EnderDragon::findNewTarget()
{
    boost::shared_ptr<Player> player;
    const unsigned int action = getSynchedAction();

    switch (action)
    {
    case ACTION_HOLDING_PATTERN:
    case ACTION_TAKEOFF:
        if (!mNewTarget && mPath != NULL && mPath->isDone())
        {
            int y = mLevel->mSeaLevel + 5;
            const BlockPos top = mLevel->getTopSolidBlockPos(BlockPos(0, 0, 0));
            if (top.y > y)
                y = top.y;
            player = mLevel->getNearestPlayer(0.0, static_cast<double>(y), 0.0, 64.0, false, 64.0);

            double distance = 64.0;
            if (player != NULL)
                distance = player->distanceToSqr(0.0, static_cast<double>(y), 0.0) / 512.0;

            if (mDragonFight != NULL)
            {
                if (mRandom->nextInt(mDragonFight->getCrystalsAlive() + 3) == 0)
                {
                    setSynchedAction(ACTION_LANDING_APPROACH, false);
                }
                else if (player != NULL
                         && (mRandom->nextInt(static_cast<int>(std::fabs(distance)) + 2) == 0
                             || mRandom->nextInt(mDragonFight->getCrystalsAlive() + 2) == 0))
                {
                    setSynchedAction(ACTION_STRAFE_PLAYER, false);
                }
            }
        }
        mNewTarget = false;
        break;
    case ACTION_STRAFE_PLAYER:
        if (mPath == NULL || (mPath->isDone() && mNewTarget))
            setSynchedAction(ACTION_HOLDING_PATTERN, false);
        mNewTarget = false;
        break;
    case ACTION_LANDING:
        mFlameCount = 0;
        setSynchedAction(ACTION_SITTING_SCANNING, false);
        mAttackTarget = mLevel->getNearestPlayer(shared_from_this(), SITTING_TARGET_RANGE, 10.0);
        mActionTicks = 0;
        mNewTarget = false;
        break;
    default:
        mNewTarget = false;
        break;
    }

    if (getSynchedAction() == ACTION_STRAFE_PLAYER && player != NULL)
    {
        mAttackTarget = player;
        strafeAttackTarget();
    }
    else if (getSynchedAction() == ACTION_LANDING_APPROACH)
    {
        if (mPath == NULL || mPath->isDone())
        {
            const unsigned int start = findClosestNode();
            int y = mLevel->mSeaLevel + 5;
            const BlockPos top = mLevel->getTopSolidBlockPos(BlockPos(0, 0, 0));
            if (top.y > y)
                y = top.y;
            player = mLevel->getNearestPlayer(0.0, static_cast<double>(y), 0.0, 128.0, false, 128.0);

            unsigned int end;
            if (player != NULL)
            {
                Vec3* direction = Vec3::newTemp(player->mX, 0.0, player->mZ)->normalize();
                end = findClosestNode(-(direction->x * 40.0), 105.0, -(direction->z * 40.0));
            }
            else
            {
                end = findClosestNode(40.0, static_cast<double>(y), 0.0);
            }

            Node target(0, y, 0);
            if (mPath != NULL)
                delete mPath;
            mPath = findPath(start, end, &target);
            if (mPath != NULL)
                mPath->next();
        }

        mActionTicks = 0;
        navigateToNextPathNode();
        if (mPath != NULL && mPath->isDone())
            setSynchedAction(ACTION_LANDING, false);
    }
    else if (getSynchedAction() != ACTION_SITTING_FLAMING && getSynchedAction() != ACTION_SITTING_ATTACKING
             && getSynchedAction() != ACTION_SITTING_SCANNING)
    {
        if (mPath == NULL || mPath->isDone())
        {
            const unsigned int start = findClosestNode();
            int end = static_cast<int>(start);
            if (getSynchedAction() == ACTION_TAKEOFF)
            {
                Vec3* look = getHeadLookVector(1.0f);
                end = static_cast<int>(findClosestNode(-(look->x * 40.0), 105.0, -(look->z * 40.0)));
            }
            else
            {
                if (mRandom->nextInt(8) == 0)
                {
                    mClockwise = !mClockwise;
                    end += 6;
                }
                if (mClockwise)
                    ++end;
                else
                    --end;
            }

            if (mDragonFight != NULL && mDragonFight->getCrystalsAlive() == 0)
            {
                end = ((end - OUTER_RING_NODE_COUNT) & 7) + OUTER_RING_NODE_COUNT;
            }
            else
            {
                end %= OUTER_RING_NODE_COUNT;
                if (end < 0)
                    end += OUTER_RING_NODE_COUNT;
            }

            if (mPath != NULL)
                delete mPath;
            mPath = findPath(start, end, NULL);
            if (mPath != NULL)
                mPath->next();
        }

        navigateToNextPathNode();
        if (getSynchedAction() != ACTION_STRAFE_PLAYER)
            mAttackTarget.reset();
    }
}

bool EnderDragon::hurt(boost::shared_ptr<MultiEntityMobPart> part, const DamageSource* source, float amount)
{
    if (part.get() != mHead.get())
    {
        amount = amount / 4.0f + std::min(amount, 1.0f);
    }
    if (amount < 0.01f)
        return false;

    if ((source->getEntity() != NULL && eTYPE_DERIVED_FROM(EntityType::PLAYER, source->getEntity()->GetType()))
        || source->isExplosion())
    {
        const int healthBefore = static_cast<int>(getHealth());
        reallyHurt(source, amount);
        if (getHealth() <= 0.0f && getSynchedAction() != ACTION_SITTING_FLAMING
            && getSynchedAction() != ACTION_SITTING_SCANNING && getSynchedAction() != ACTION_SITTING_ATTACKING)
        {
            setHealth(1.0f);
            if (setSynchedAction(ACTION_LANDING_APPROACH, false))
            {
                if (mPath != NULL)
                {
                    while (!mPath->isDone())
                        mPath->next();
                }
                findNewTarget();
            }
        }

        if (getSynchedAction() == ACTION_SITTING_FLAMING || getSynchedAction() == ACTION_SITTING_SCANNING
            || getSynchedAction() == ACTION_SITTING_ATTACKING)
        {
            mSittingDamageReceived += healthBefore - static_cast<int>(getHealth());
            if (mSittingDamageReceived > 0.25f * getMaxHealth())
            {
                mSittingDamageReceived = 0;
                setSynchedAction(ACTION_TAKEOFF, false);
                mNewTarget = true;
            }
        }
    }
    return true;
}

void EnderDragon::GetInteractTooltip(boost::shared_ptr<MultiEntityMobPart>, const EntityTooltipDataHolder& holder)
{
    GetInteractTooltip(holder);
}

void EnderDragon::AddParts()
{
    mHead = boost::shared_ptr<MultiEntityMobPart>(
        new MultiEntityMobPart(boost::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"head", 6.0f, 6.0f));
    mNeck = boost::shared_ptr<MultiEntityMobPart>(
        new MultiEntityMobPart(boost::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"neck", 6.0f, 6.0f));
    mBody = boost::shared_ptr<MultiEntityMobPart>(
        new MultiEntityMobPart(boost::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"body", 8.0f, 8.0f));
    mTail1 = boost::shared_ptr<MultiEntityMobPart>(
        new MultiEntityMobPart(boost::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"tail", 4.0f, 4.0f));
    mTail2 = boost::shared_ptr<MultiEntityMobPart>(
        new MultiEntityMobPart(boost::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"tail", 4.0f, 4.0f));
    mTail3 = boost::shared_ptr<MultiEntityMobPart>(
        new MultiEntityMobPart(boost::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"tail", 4.0f, 4.0f));
    mWing1 = boost::shared_ptr<MultiEntityMobPart>(
        new MultiEntityMobPart(boost::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"wing", 4.0f, 4.0f));
    mWing2 = boost::shared_ptr<MultiEntityMobPart>(
        new MultiEntityMobPart(boost::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"wing", 4.0f, 4.0f));

    mSubEntities.push_back(mHead);
    mSubEntities.push_back(mNeck);
    mSubEntities.push_back(mBody);
    mSubEntities.push_back(mTail1);
    mSubEntities.push_back(mTail2);
    mSubEntities.push_back(mTail3);
    mSubEntities.push_back(mWing1);
    mSubEntities.push_back(mWing2);
}

void EnderDragon::onCrystalDestroyed(boost::shared_ptr<EnderCrystal> crystal, const BlockPos& pos,
                                     const DamageSource* source)
{
    boost::shared_ptr<Player> player;
    if (Entity::instanceof(source->getEntity(), EntityType::PLAYER))
        player = boost::static_pointer_cast<Player>(source->getEntity());
    else
        player = mLevel->getNearestAttackablePlayer(pos, 64.0, 64.0);

    if (crystal.get() == mNearestCrystal.get())
    {
        DamageSource* explosion = DamageSource::CreateExplosion(player);
        hurt(mHead, explosion, 10.0f);
        delete explosion;
    }

    if (mDragonFight != NULL && (mDragonFight->getCrystalsAlive() & 1) == 0)
    {
        if (setSynchedAction(ACTION_LANDING_APPROACH, false))
        {
            if (mPath != NULL)
            {
                while (!mPath->isDone())
                    mPath->next();
            }
            mActionTicks = 1;
        }
        else if (source->getEntity() != NULL && eTYPE_DERIVED_FROM(EntityType::PLAYER, source->getEntity()->GetType()))
        {
            if (setSynchedAction(ACTION_STRAFE_PLAYER, false))
            {
                mAttackTarget = boost::dynamic_pointer_cast<Player>(source->getEntity());
                strafeAttackTarget();
            }
        }
    }
}

void EnderDragon::_init()
{
    fjDerivedCtorCalls();
    mXTarget = mYTarget = mZTarget = 0.0;
    mPosPointer = -1;
    mOldFlapTime = 0.0f;
    mFlapTime = 0.0f;
    mNewTarget = false;
    mInWall = false;
    mAttackTarget = boost::shared_ptr<Entity>();
    mDeathTime = 0;
    mNearestCrystal = boost::shared_ptr<EnderCrystal>();
    mFireballCharge = 0;
    mUnusedFloat = 0.0f;
    mClockwise = true;
    setSynchedAction(ACTION_HOLDING_PATTERN, false);
    mActionTicks = 0;
    mSittingDamageReceived = 0;
    mClientLerpReset = 0.0;
    mFlameCount = 0;
    for (int i = 0; i < POSITION_HISTORY_SIZE; ++i)
    {
        mPositions[i][0] = 0.0;
        mPositions[i][1] = 0.0;
        mPositions[i][2] = 0.0;
    }
    mNodes = new arrayWithLength<Node*>(PATH_NODE_COUNT, true);
    mOpenSet = new BinaryHeap();
    mPath = NULL;
}

void EnderDragon::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::ENDER_DRAGON);
}

void EnderDragon::staticCtor()
{
    DATA_DRAGON_PHASE = SynchedEntityData::defineId<int>(EntityType::ENDER_DRAGON, EntityType::MOB,
                                                         EntityDataSerializers::INT);
}

void EnderDragon::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(200.0);
}

void EnderDragon::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_DRAGON_PHASE, 0);
}

bool EnderDragon::canRide(boost::shared_ptr<Entity>)
{
    return false;
}

void EnderDragon::getLatencyPos(arrayWithLength<double> positions, int index, float partialTicks)
{
    if (getHealth() <= 0.0f)
        partialTicks = 0.0f;

    partialTicks = 1.0f - partialTicks;
    int current = (mPosPointer - index) & (POSITION_HISTORY_SIZE - 1);
    int previous = (mPosPointer - index - 1) & (POSITION_HISTORY_SIZE - 1);
    double base = mPositions[current][0];
    double delta = Mth::wrapDegrees(mPositions[previous][0] - base);
    positions.data[0] = base + delta * partialTicks;
    base = mPositions[current][1];
    delta = mPositions[previous][1] - base;
    positions.data[1] = base + delta * partialTicks;
    positions.data[2] = mPositions[current][2] + (mPositions[previous][2] - mPositions[current][2]) * partialTicks;
}

float EnderDragon::getTilt(float partialTicks)
{
    double currentValues[3];
    double pastValues[3];
    arrayWithLength<double> current(currentValues, 3);
    arrayWithLength<double> past(pastValues, 3);
    getLatencyPos(current, 5, partialTicks);
    getLatencyPos(past, 10, partialTicks);
    double tilt = (current[1] - past[1]) * 10.0;
    return static_cast<float>(tilt);
}

double EnderDragon::getHeadYOffset(float)
{
    double offset;
    if (getSynchedAction() == ACTION_SITTING_FLAMING || getSynchedAction() == ACTION_SITTING_SCANNING
        || getSynchedAction() == ACTION_SITTING_ATTACKING)
    {
        offset = -1.0;
    }
    else
    {
        double bodyValues[3];
        arrayWithLength<double> body(bodyValues, 3);
        getLatencyPos(body, 5, 1.0f);
        double headValues[3];
        arrayWithLength<double> head(headValues, 3);
        getLatencyPos(head, 0, 1.0f);
        offset = head[1] - body[1];
    }
    return offset;
}

double EnderDragon::rotWrap(double angle)
{
    while (angle >= 180.0)
        angle -= 360.0;
    while (angle < -180.0)
        angle += 360.0;
    return static_cast<float>(angle);
}

void EnderDragon::checkCrystals()
{
    if (mNearestCrystal != NULL)
    {
        if (mNearestCrystal->mRemoved)
        {
            mNearestCrystal = boost::shared_ptr<EnderCrystal>();
        }
        else if (mTickCount % 10 == 0 && getHealth() < getMaxHealth())
        {
            setHealth(getHealth() + 1.0f);
        }
    }

    if (mRandom->nextInt(10) != 0)
        return;

    std::vector<boost::shared_ptr<Entity> >* crystals = mLevel->getEntitiesOfClass(
        typeid(EnderCrystal), mBoundingBox->grow(32.0, 32.0, 32.0));
    boost::shared_ptr<EnderCrystal> closest;
    double closestDistance = std::numeric_limits<double>::max();
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = crystals->begin(); it != crystals->end(); ++it)
    {
        boost::shared_ptr<EnderCrystal> crystal = boost::dynamic_pointer_cast<EnderCrystal>(*it);
        const double distance = crystal->distanceToSqr(shared_from_this());
        if (distance < closestDistance)
        {
            closestDistance = distance;
            closest = crystal;
        }
    }
    delete crystals;
    mNearestCrystal = closest;
}

void EnderDragon::knockBack(std::vector<boost::shared_ptr<Entity> >* entities)
{
    const double centreX = (mBody->mBoundingBox->mMinX + mBody->mBoundingBox->mMaxX) * 0.5;
    const double centreZ = (mBody->mBoundingBox->mMinZ + mBody->mBoundingBox->mMaxZ) * 0.5;
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        if (((*it)->GetType() & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
            continue;

        boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(*it);
        const double dx = living->mX - centreX;
        const double dz = living->mZ - centreZ;
        const double distanceSqr = dx * dx + dz * dz;
        living->push(dx / distanceSqr * 4.0, 0.2f, dz / distanceSqr * 4.0);
    }
}

void EnderDragon::hurt(std::vector<boost::shared_ptr<Entity> >* entities)
{
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        if (((*it)->GetType() & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
            continue;

        boost::shared_ptr<LivingEntity> target = boost::dynamic_pointer_cast<LivingEntity>(*it);
        EntityDamageSource* source = DamageSource::CreateMobAttack(
            boost::dynamic_pointer_cast<LivingEntity>(shared_from_this()));
        target->hurt(source, 10.0f);
        delete source;
    }
}

bool EnderDragon::checkWalls(AABB* box)
{
    const int minX = Mth::floor(box->mMinX);
    const int minY = Mth::floor(box->mMinY);
    const int minZ = Mth::floor(box->mMinZ);
    const int maxX = Mth::floor(box->mMaxX);
    const int maxY = Mth::floor(box->mMaxY);
    const int maxZ = Mth::floor(box->mMaxZ);

    bool hitWall = false;
    bool destroyedBlock = false;
    for (int x = minX; x <= maxX; ++x)
    {
        for (int y = minY; y <= maxY; ++y)
        {
            for (int z = minZ; z <= maxZ; ++z)
            {
                const BlockPos pos(x, y, z);
                const BlockState* state = mLevel->getBlockState(pos);
                Block* block = state->getBlock();
                if (state->getMaterial() == Material::AIR || state->getMaterial() == Material::FIRE)
                    continue;

                if (!mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING) || block == Blocks::barrier
                    || block == Blocks::obsidian || block == Blocks::end_stone || block == Blocks::bedrock
                    || block == Blocks::end_portal || block == Blocks::end_portal_frame
                    || block == Blocks::command_block || block == Blocks::repeating_command_block
                    || block == Blocks::chain_command_block || block == Blocks::iron_bars
                    || block == Blocks::end_gateway)
                    hitWall = true;
                else
                    destroyedBlock = mLevel->removeBlock(pos) || destroyedBlock;
            }
        }
    }

    if (destroyedBlock)
    {
        const double px = box->mMinX + (box->mMaxX - box->mMinX) * mRandom->nextFloat();
        const double py = box->mMinY + (box->mMaxY - box->mMinY) * mRandom->nextFloat();
        const double pz = box->mMinZ + (box->mMaxZ - box->mMinZ) * mRandom->nextFloat();
        mLevel->addParticle(PARTICLE_LARGE_EXPLODE, px, py, pz, 0.0, 0.0, 0.0, arrayWithLength<int>());
    }
    return hitWall;
}

void EnderDragon::aiStep()
{
    if (mLevel->mIsClientSide)
    {
        setHealth(getHealth());
        const float flap = Mth::cos(mFlapTime * 3.1415927f * 2.0f);
        const float oldFlap = Mth::cos(mOldFlapTime * 3.1415927f * 2.0f);
        if (oldFlap <= -0.3f && flap >= -0.3f && !isSilent())
        {
            mLevel->playLocalSound(mX, mY, mZ, SoundEvents::ENDERDRAGON_FLAP, getSoundSource(), 1.0f,
                                   0.8f + mRandom->nextFloat() * 0.3f, false, 100.0f);
        }

        if (getSynchedAction() != ACTION_SITTING_FLAMING && getSynchedAction() != ACTION_SITTING_SCANNING
            && getSynchedAction() != ACTION_SITTING_ATTACKING && --mGrowlTime < 0)
        {
            mLevel->playLocalSound(mX, mY, mZ, SoundEvents::ENDERDRAGON_GROWL, getSoundSource(), 0.5f,
                                   0.8f + mRandom->nextFloat() * 0.3f, false, 100.0f);
            mGrowlTime = 200 + mRandom->nextInt(200);
        }
    }

    mOldFlapTime = mFlapTime;
    if (getHealth() <= 0.0f)
    {
        const float xOffset = (mRandom->nextFloat() - 0.5f) * 8.0f;
        const float yOffset = (mRandom->nextFloat() - 0.5f) * 4.0f;
        const float zOffset = (mRandom->nextFloat() - 0.5f) * 8.0f;
        mLevel->addParticle(PARTICLE_LARGE_EXPLODE, mX + xOffset, mY + 2.0 + yOffset, mZ + zOffset, 0.0, 0.0, 0.0,
                            arrayWithLength<int>());
        return;
    }

    checkCrystals();
    float flapSpeed = static_cast<float>(0.2f / (std::sqrt(mXd * mXd + mZd * mZd) * 10.0 + 1.0));
    flapSpeed *= static_cast<float>(std::pow(2.0, mYd));
    if (getSynchedAction() == ACTION_SITTING_FLAMING || getSynchedAction() == ACTION_SITTING_SCANNING
        || getSynchedAction() == ACTION_SITTING_ATTACKING)
        mFlapTime += 0.1f;
    else if (mInWall)
        mFlapTime += flapSpeed * 0.5f;
    else
        mFlapTime += flapSpeed;
    mYRot = Mth::wrapDegrees(mYRot);

    if (isNoAi())
    {
        mFlapTime = 0.5f;
        return;
    }

    if (mPosPointer < 0)
    {
        for (int i = 0; i < POSITION_HISTORY_SIZE; ++i)
        {
            mPositions[i][0] = mYRot;
            mPositions[i][1] = mY;
        }
    }
    if (++mPosPointer == POSITION_HISTORY_SIZE)
        mPosPointer = 0;
    mPositions[mPosPointer][0] = mYRot;
    mPositions[mPosPointer][1] = mY;

    if (mLevel->mIsClientSide)
    {
        if (mLerpSteps > 0)
        {
            const double x = mX + (mLerpX - mX) / mLerpSteps;
            const double y = mY + (mLerpY - mY) / mLerpSteps;
            const double z = mZ + (mLerpZ - mZ) / mLerpSteps;
            const double yRotDelta = Mth::wrapDegrees(mLerpYRot - mYRot);
            mYRot += static_cast<float>(yRotDelta) / static_cast<float>(mLerpSteps);
            mXRot += static_cast<float>((mLerpXRot - mXRot) / mLerpSteps);
            --mLerpSteps;
            mClientLerpReset = 0.0;
            setPos(x, y, z);
            setRot(mYRot, mXRot);
        }

        if (getSynchedAction() == ACTION_LANDING
            || (getSynchedAction() == ACTION_SITTING_FLAMING && (mTickCount & 1) == 0))
        {
            Vec3* look = getHeadLookVector(1.0f)->normalize()->yRot(-3.1415927f / 4.0f);
            for (int i = 0; i < 8; ++i)
            {
                if (getSynchedAction() == ACTION_LANDING)
                {
                    double x = mHead->mX;
                    double y = mHead->mBoundingBox->mMinY + mHead->mBbHeight * 0.5f;
                    double z = mHead->mZ;
                    x += (mLevel->mRandom->nextBoolean() ? 1 : -1) * mLevel->mRandom->nextFloat() * 0.5f;
                    y += (mLevel->mRandom->nextBoolean() ? 1 : -1) * mLevel->mRandom->nextFloat() * 0.5f;
                    z += (mLevel->mRandom->nextBoolean() ? 1 : -1) * mLevel->mRandom->nextFloat() * 0.5f;
                    mLevel->addParticle(PARTICLE_DRAGON_BREATH, x, y, z, mXd - look->x * 0.08, mYd - look->y * 0.3,
                                        mZd - look->z * 0.08, arrayWithLength<int>());
                }
                else
                {
                    for (unsigned int j = 0; j < 2; ++j)
                    {
                        double x = mHead->mX;
                        double y = mHead->mBoundingBox->mMinY + mHead->mBbHeight * 0.5f;
                        double z = mHead->mZ;
                        x += (mLevel->mRandom->nextBoolean() ? 1 : -1) * mLevel->mRandom->nextFloat() * 0.5f;
                        y += (mLevel->mRandom->nextBoolean() ? 1 : -1) * mLevel->mRandom->nextFloat() * 0.5f;
                        z += (mLevel->mRandom->nextBoolean() ? 1 : -1) * mLevel->mRandom->nextFloat() * 0.5f;
                        mLevel->addParticle(PARTICLE_DRAGON_BREATH, x, y, z, -(look->x * 0.08 * j), -(look->y * 0.6),
                                            -(look->z * 0.08 * j), arrayWithLength<int>());
                    }
                }
                look = look->yRot(3.1415927f / 16.0f);
            }
        }
        else if (getSynchedAction() == ACTION_SITTING_ATTACKING)
        {
            mLevel->playLocalSound(mX, mY, mZ, SoundEvents::ENDERDRAGON_GROWL, getSoundSource(), 0.5f,
                                   0.8f + mRandom->nextFloat() * 0.3f, false, 100.0f);
        }
        mYBodyRot = mYRot;
    }
    else
    {
        const double dx = mXTarget - mX;
        const double dy = mYTarget - mY;
        const double dz = mZTarget - mZ;
        const double distanceSqr = dx * dx + dy * dy + dz * dz;

        if (getSynchedAction() == ACTION_SITTING_FLAMING)
        {
            if (--mActionTicks <= 0)
            {
                if (mFlameCount >= 4)
                {
                    setSynchedAction(ACTION_TAKEOFF, false);
                    mNewTarget = true;
                }
                else
                {
                    setSynchedAction(ACTION_SITTING_SCANNING, false);
                    mAttackTarget = mLevel->getNearestPlayer(shared_from_this(), SITTING_TARGET_RANGE, 10.0);
                }
            }
        }
        else if (getSynchedAction() == ACTION_SITTING_SCANNING)
        {
            mAttackTarget = mLevel->getNearestPlayer(shared_from_this(), SITTING_TARGET_RANGE, 10.0);
            ++mActionTicks;
            if (mAttackTarget != NULL)
            {
                if (mActionTicks > 25)
                {
                    setSynchedAction(ACTION_SITTING_ATTACKING, false);
                    mActionTicks = 40;
                }
            }
            else if (mActionTicks >= 100)
            {
                setSynchedAction(ACTION_TAKEOFF, false);
                mNewTarget = true;
            }
        }
        else if (getSynchedAction() == ACTION_SITTING_ATTACKING)
        {
            if (--mActionTicks <= 0)
            {
                ++mFlameCount;
                setSynchedAction(ACTION_SITTING_FLAMING, false);
                mAttackTarget = mLevel->getNearestPlayer(shared_from_this(), SITTING_TARGET_RANGE, 10.0);
                mActionTicks = 60;
            }
        }
        else if (!mNewTarget && getSynchedAction() == ACTION_TAKEOFF)
        {
            const BlockPos top = mLevel->getTopSolidBlockPos(BlockPos(0, 0, 0));
            if (static_cast<float>(distanceToSqr(0.0, static_cast<double>(top.y), 0.0)) > 100.0f)
                setSynchedAction(ACTION_HOLDING_PATTERN, false);
        }
        else if (mNewTarget || (getSynchedAction() != ACTION_LANDING && distanceSqr < 100.0) || distanceSqr < 1.0
                 || distanceSqr > 22500.0 || mHorizontalCollision || mVerticalCollision)
        {
            findNewTarget();
        }

        if (getSynchedAction() == ACTION_SITTING_FLAMING && mActionTicks == 50)
        {
            Vec3* look = getHeadLookVector(1.0f)->normalize();
            const double x = mHead->mX - look->x * 5.0 * 0.5;
            const double z = mHead->mZ - look->z * 5.0 * 0.5;
            double y = mHead->mY + mHead->mBbHeight * 0.5f;
            MutableBlockPos pos(Mth::floor(x), Mth::floor(y), Mth::floor(z));
            while (mLevel->isEmptyBlock(pos))
            {
                y -= 1.0;
                pos.set(Mth::floor(x), Mth::floor(y), Mth::floor(z));
                if (y < 0.0)
                    break;
            }

            if (y > 0.0)
            {
                y = static_cast<double>(Mth::floor(y) + 1);
                if (mFlameCloud != NULL)
                {
                    mFlameCloud->remove();
                    mFlameCloud = boost::shared_ptr<AreaEffectCloud>();
                }
                mFlameCloud = boost::shared_ptr<AreaEffectCloud>(new AreaEffectCloud(mLevel, x, y, z));
                mFlameCloud->setOwner(boost::static_pointer_cast<LivingEntity>(shared_from_this()));
                mFlameCloud->setRadius(5.0f);
                mFlameCloud->setDuration(60);
                mFlameCloud->setParticle(PARTICLE_DRAGON_BREATH);
                mFlameCloud->addEffect(new MobEffectInstance(MobEffects::HARM));
                mFlameCloud->setParticleSpawnRateOnTick(10);
                mLevel->addEntity(mFlameCloud);
            }
        }

        if (getSynchedAction() != ACTION_SITTING_FLAMING)
        {
            if (getSynchedAction() == ACTION_SITTING_SCANNING)
            {
                if (mAttackTarget != NULL)
                {
                    Vec3* toTarget = Vec3::newTemp(mAttackTarget->mX - mX, 0.0, mAttackTarget->mZ - mZ)->normalize();
                    Vec3* facing = Vec3::newTemp(std::sin(mYRot * 3.1415927f / 180.0f), 0.0,
                                                 -std::cos(mYRot * 3.1415927f / 180.0f))
                                       ->normalize();
                    const float angle = std::acos(static_cast<float>(facing->dot(toTarget))) * 180.0f / 3.1415927f
                                        + 0.5f;
                    if (angle < 0.0f || angle > 10.0f)
                    {
                        double yRotDelta = Mth::wrapDegrees(
                            180.0
                            - std::atan2(mAttackTarget->mX - mHead->mX, mAttackTarget->mZ - mHead->mZ) * 180.0
                                  / 3.1415927f
                            - mYRot);
                        if (yRotDelta > 50.0)
                            yRotDelta = 50.0;
                        else if (yRotDelta < -50.0)
                            yRotDelta = -50.0;

                        const double targetDx = mXTarget - mX;
                        const double targetDz = mZTarget - mZ;
                        const double targetDistanceSqr = targetDx * targetDx + targetDz * targetDz;
                        mYRota *= 0.8f;
                        const float speed = static_cast<float>(std::sqrt(targetDistanceSqr) * 1.0 + 1.0);
                        float maxSpeed = static_cast<float>(std::sqrt(targetDistanceSqr) * 1.0 + 1.0);
                        if (maxSpeed > 40.0f)
                            maxSpeed = 40.0f;
                        mYRota += static_cast<float>(yRotDelta * ((0.7f / maxSpeed) / speed));
                        mYRot += mYRota;
                    }
                }
            }
            else if (getSynchedAction() != ACTION_SITTING_ATTACKING)
            {
                if (getSynchedAction() == ACTION_STRAFE_PLAYER && mAttackTarget != NULL && mPath != NULL
                    && mPath->isDone())
                {
                    mXTarget = mAttackTarget->mX;
                    mZTarget = mAttackTarget->mZ;
                    const double targetDx = mXTarget - mX;
                    const double targetDz = mZTarget - mZ;
                    double height = 0.4f + std::sqrt(targetDx * targetDx + targetDz * targetDz) / 80.0 - 1.0;
                    if (height > 10.0)
                        height = 10.0;
                    mYTarget = mAttackTarget->mBoundingBox->mMinY + height;
                }

                const double horizontalDistance = std::sqrt(dx * dx + dz * dz);
                float maxClimb = 0.6f;
                if (getSynchedAction() == ACTION_LANDING)
                    maxClimb = 1.5f;
                double climb = dy / horizontalDistance;
                if (climb < -maxClimb)
                    climb = -maxClimb;
                if (climb > maxClimb)
                    climb = maxClimb;
                mYd += climb * 0.1f;

                while (mYRot < -180.0f)
                    mYRot += 360.0f;
                while (mYRot >= 180.0f)
                    mYRot -= 360.0f;

                double yRotDelta = 180.0 - std::atan2(dx, dz) * 180.0 / 3.1415927f - mYRot;
                while (yRotDelta < -180.0)
                    yRotDelta += 360.0;
                while (yRotDelta >= 180.0)
                    yRotDelta -= 360.0;
                if (yRotDelta > 50.0)
                    yRotDelta = 50.0;
                else if (yRotDelta < -50.0)
                    yRotDelta = -50.0;

                Vec3* direction = Vec3::newTemp(mXTarget - mX, mYTarget - mY, mZTarget - mZ)->normalize();
                Vec3* facing = Vec3::newTemp(std::sin(mYRot * 3.1415927f / 180.0f), mYd,
                                             -std::cos(mYRot * 3.1415927f / 180.0f))
                                   ->normalize();
                float alignment = static_cast<float>(facing->dot(direction) + 0.5) / 1.5f;
                if (alignment < 0.0f)
                    alignment = 0.0f;

                mYRota *= 0.8f;
                const float speed = static_cast<float>(std::sqrt(mXd * mXd + mZd * mZd) * 1.0 + 1.0);
                float maxSpeed = static_cast<float>(std::sqrt(mXd * mXd + mZd * mZd) * 1.0 + 1.0);
                if (maxSpeed > 40.0f)
                    maxSpeed = 40.0f;

                if (getSynchedAction() == ACTION_LANDING)
                    mYRota += static_cast<float>(yRotDelta * (maxSpeed / speed));
                else
                    mYRota += static_cast<float>(yRotDelta * ((0.7f / maxSpeed) / speed));
                mYRot += mYRota * 0.1f;

                const float turnInertia = 2.0f / (maxSpeed + 1.0f);
                moveRelative(0.0f, 0.0f, -1.0f, 0.06f * (alignment * turnInertia + (1.0f - turnInertia)));
                if (mInWall)
                    move(eMoverType_Self, mXd * 0.8f, mYd * 0.8f, mZd * 0.8f, false);
                else
                    move(eMoverType_Self, mXd, mYd, mZd, false);

                Vec3* motion = Vec3::newTemp(mXd, mYd, mZd)->normalize();
                const float drag = static_cast<float>(motion->dot(facing) + 1.0) * 0.5f * 0.15f + 0.8f;
                mXd *= drag;
                mZd *= drag;
                mYd *= 0.91f;
            }
        }
        mYBodyRot = mYRot;
    }

    mHead->mBbHeight = 1.0f;
    mHead->mBbWidth = 1.0f;
    mNeck->mBbHeight = 3.0f;
    mNeck->mBbWidth = 3.0f;
    mTail1->mBbHeight = 2.0f;
    mTail1->mBbWidth = 2.0f;
    mTail2->mBbHeight = 2.0f;
    mTail2->mBbWidth = 2.0f;
    mTail3->mBbHeight = 2.0f;
    mTail3->mBbWidth = 2.0f;
    mBody->mBbHeight = 3.0f;
    mBody->mBbWidth = 5.0f;
    mWing1->mBbHeight = 2.0f;
    mWing1->mBbWidth = 4.0f;
    mWing2->mBbHeight = 2.0f;
    mWing2->mBbWidth = 4.0f;

    const unsigned int partCount = mSubEntities.size();
    std::vector<Vec3*> oldPositions;
    oldPositions.reserve(partCount);
    for (unsigned int i = 0; i < partCount; ++i)
        oldPositions.push_back(Vec3::newTemp(mSubEntities[i]->mX, mSubEntities[i]->mY, mSubEntities[i]->mZ));

    const float tilt = getTilt(1.0f) / 180.0f * 3.1415927f;
    const float cosTilt = std::cos(tilt);
    const float sinTilt = std::sin(tilt);
    const float yRotRadians = mYRot * 3.1415927f / 180.0f;
    const float sinYRot = std::sin(yRotRadians);
    const float cosYRot = std::cos(yRotRadians);

    mBody->tick();
    mBody->moveTo(mX + sinYRot * 0.5f, mY, mZ - cosYRot * 0.5f, 0.0f, 0.0f);
    mWing1->tick();
    mWing1->moveTo(mX + cosYRot * 4.5f, mY + 2.0, mZ + sinYRot * 4.5f, 0.0f, 0.0f);
    mWing2->tick();
    mWing2->moveTo(mX - cosYRot * 4.5f, mY + 2.0, mZ - sinYRot * 4.5f, 0.0f, 0.0f);

    if (!mLevel->mIsClientSide && mHurtDuration == 0)
    {
        std::vector<boost::shared_ptr<Entity> > nearbyEntities;
        knockBack(mLevel->getEntities(shared_from_this(),
                                      mWing1->mBoundingBox->grow(4.0, 2.0, 4.0)->move(0.0, -2.0, 0.0), NULL));
        knockBack(mLevel->getEntities(shared_from_this(),
                                      mWing2->mBoundingBox->grow(4.0, 2.0, 4.0)->move(0.0, -2.0, 0.0), NULL));
        hurt(mLevel->getEntities(shared_from_this(), mNeck->mBoundingBox->grow(1.0, 1.0, 1.0), &nearbyEntities));
        hurt(mLevel->getEntities(shared_from_this(), mHead->mBoundingBox->grow(1.0, 1.0, 1.0), &nearbyEntities));
    }

    double bodyLatency[3];
    getLatencyPos(arrayWithLength<double>(bodyLatency, 3), 5, 1.0f);
    const double headYRotDiff = getHeadYRotDiff(1.0f);
    const float sinHead = static_cast<float>(std::sin((mYRot + headYRotDiff) * 3.1415927f / 180.0 - mYRota * 0.01f));
    const float cosHead = static_cast<float>(std::cos((mYRot + headYRotDiff) * 3.1415927f / 180.0 - mYRota * 0.01f));
    mHead->tick();
    mNeck->tick();
    const double headYOffset = getHeadYOffset(1.0f);
    mHead->moveTo(mX + sinHead * 6.5f * cosTilt, mY + headYOffset + sinTilt * 6.5f, mZ - cosHead * 6.5f * cosTilt, 0.0f,
                  0.0f);
    mNeck->moveTo(mX + sinHead * 5.5f * cosTilt, mY + headYOffset + sinTilt * 5.5f, mZ - cosHead * 5.5f * cosTilt, 0.0f,
                  0.0f);

    for (int i = 0; i < 3; ++i)
    {
        boost::shared_ptr<MultiEntityMobPart> part;
        if (i == 0)
            part = mTail1;
        if (i == 1)
            part = mTail2;
        if (i == 2)
            part = mTail3;

        double partLatency[3];
        getLatencyPos(arrayWithLength<double>(partLatency, 3), 12 + i * 2, 1.0f);
        const float partYRot = mYRot * 3.1415927f / 180.0f
                               + static_cast<float>(rotWrap(partLatency[0] - bodyLatency[0])) * 3.1415927f / 180.0f
                                     * 1.0f;
        const float sinPart = std::sin(partYRot);
        const float cosPart = std::cos(partYRot);
        const float spacing = static_cast<float>(i + 1) * 2.0f;
        part->tick();
        part->moveTo(mX - (sinYRot * 1.5f + sinPart * spacing) * cosTilt,
                     mY + (partLatency[1] - bodyLatency[1]) - (spacing + 1.5f) * sinTilt + 1.5,
                     mZ + (cosYRot * 1.5f + cosPart * spacing) * cosTilt, 0.0f, 0.0f);
    }

    if (!mLevel->mIsClientSide)
    {
        double fireballRange = 64.0;
        bool targetInRange = false;
        if (getSynchedAction() == ACTION_STRAFE_PLAYER && mAttackTarget != NULL
            && mAttackTarget->distanceToSqr(shared_from_this()) < fireballRange * fireballRange)
            targetInRange = true;

        if (targetInRange && canSee(mAttackTarget))
        {
            ++mFireballCharge;
            Vec3* toTarget = Vec3::newTemp(mAttackTarget->mX - mX, 0.0, mAttackTarget->mZ - mZ)->normalize();
            Vec3* facing = Vec3::newTemp(std::sin(mYRot * 3.1415927f / 180.0f), 0.0,
                                         -std::cos(mYRot * 3.1415927f / 180.0f))
                               ->normalize();
            const float angle = std::acos(static_cast<float>(facing->dot(toTarget))) * 180.0f / 3.1415927f + 0.5f;
            if (mFireballCharge >= 20 && angle >= 0.0f && angle < 10.0f)
            {
                Vec3* look = getViewVector(1.0f);
                const double headX = mHead->mX - look->x;
                const double headY = mHead->mY + mHead->mBbHeight * 0.5f + 0.5;
                const double headZ = mHead->mZ - look->z;
                const double xa = mAttackTarget->mX - headX;
                const double ya = mAttackTarget->mBoundingBox->mMinY + mAttackTarget->mBbHeight * 0.5f
                                  - (headY + mHead->mBbHeight * 0.5f);
                const double za = mAttackTarget->mZ - headZ;
                mLevel->levelEvent(boost::shared_ptr<Player>(), LevelEvent::SOUND_GHAST_FIREBALL,
                                   BlockPos(static_cast<int>(mX), static_cast<int>(mY), static_cast<int>(mZ)), 0);
                boost::shared_ptr<DragonFireball> fireball(
                    new DragonFireball(mLevel, boost::dynamic_pointer_cast<Mob>(shared_from_this()), xa, ya, za));
                fireball->mX = headX;
                fireball->mY = headY;
                fireball->mZ = headZ;
                mLevel->addEntity(fireball);
                mFireballCharge = 0;
                if (mPath != NULL)
                {
                    while (!mPath->isDone())
                        mPath->next();
                }
                mNewTarget = true;
                findNewTarget();
            }
        }
        else if (mFireballCharge > 0)
        {
            --mFireballCharge;
        }
    }

    if (!mLevel->mIsClientSide)
    {
        mInWall = checkWalls(mHead->mBoundingBox) | checkWalls(mNeck->mBoundingBox) | checkWalls(mBody->mBoundingBox);
        if (mDragonFight != NULL)
            mDragonFight->updateDragon(boost::static_pointer_cast<EnderDragon>(shared_from_this()));
    }

    for (unsigned int i = 0; i < partCount; ++i)
    {
        mSubEntities[i]->mXo = oldPositions[i]->x;
        mSubEntities[i]->mYo = oldPositions[i]->y;
        mSubEntities[i]->mZo = oldPositions[i]->z;
    }
}

ResourceLocation* EnderDragon::getDefaultLootTable()
{
    return &BuiltInLootTables::ENDER_DRAGON;
}

bool EnderDragon::hurt(const DamageSource* source, float amount)
{
    if (source != NULL)
    {
        const EntityDamageSource* entitySource = dynamic_cast<const EntityDamageSource*>(source);
        if (entitySource != NULL && entitySource->isThorns())
            reallyHurt(source, amount);
    }
    return false;
}

void EnderDragon::reallyHurt(const DamageSource* source, float amount)
{
    LivingEntity::hurt(source, amount);
}

void EnderDragon::kill()
{
    remove();
    if (mDragonFight != NULL)
    {
        boost::shared_ptr<EnderDragon> self = boost::static_pointer_cast<EnderDragon>(shared_from_this());
        mDragonFight->updateDragon(self);
        mDragonFight->setDragonKilled(self);
    }
}

void EnderDragon::dropExperience(int experience)
{
    while (experience > 0)
    {
        const int orbValue = ExperienceOrb::getExperienceValue(experience);
        experience -= orbValue;
        mLevel->addEntity(boost::shared_ptr<ExperienceOrb>(new ExperienceOrb(mLevel, mX, mY, mZ, orbValue)));
    }
}

void EnderDragon::tickDeath()
{
    if (getSynchedAction() != ACTION_SITTING_FLAMING && getSynchedAction() != ACTION_SITTING_SCANNING
        && getSynchedAction() != ACTION_SITTING_ATTACKING)
    {
        if (!mLevel->mIsClientSide)
            setHealth(1.0f);
        return;
    }

    if (mDragonFight != NULL)
        mDragonFight->updateDragon(boost::static_pointer_cast<EnderDragon>(shared_from_this()));

    ++mDeathTime;

    if (mDeathTime >= 180 && mDeathTime <= 200)
    {
        const float xOffset = (mRandom->nextFloat() - 0.5f) * 8.0f;
        const float yOffset = (mRandom->nextFloat() - 0.5f) * 4.0f;
        const float zOffset = (mRandom->nextFloat() - 0.5f) * 8.0f;
        mLevel->addParticle(PARTICLE_HUGE_EXPLOSION, mX + xOffset, mY + 2.0 + yOffset, mZ + zOffset, 0.0, 0.0, 0.0,
                            arrayWithLength<int>());
    }

    const bool doMobLoot = mLevel->getGameRules()->getBoolean(GameRules::RULE_DOMOBLOOT);
    int experience = 500;
    if (mDragonFight != NULL && !mDragonFight->hasPreviouslyKilledDragon())
        experience = 12000;

    if (!mLevel->mIsClientSide)
    {
        if (mDeathTime > 150 && mDeathTime % 5 == 0 && doMobLoot)
            dropExperience(Mth::floor(static_cast<float>(experience) * 0.08f));

        if (mDeathTime == 1)
            mLevel->globalLevelEvent(LevelEvent::SOUND_DRAGON_DEATH, BlockPos(shared_from_this()), 0);
    }

    move(eMoverType_Self, 0.0, 0.1f, 0.0, false);
    mYRot += 20.0f;
    mYBodyRot = mYRot;

    if (mDeathTime == 200 && !mLevel->mIsClientSide)
    {
        if (doMobLoot)
            dropExperience(Mth::floor(static_cast<float>(experience) * 0.2f));

        if (mDragonFight != NULL)
            mDragonFight->setDragonKilled(boost::static_pointer_cast<EnderDragon>(shared_from_this()));

        remove();
    }
}

void EnderDragon::checkDespawn()
{
}
std::vector<boost::shared_ptr<Entity> >* EnderDragon::getSubEntities()
{
    return &mSubEntities;
}
bool EnderDragon::isPickable()
{
    return false;
}
const SoundEvent* EnderDragon::getAmbientSound()
{
    return SoundEvents::ENDERDRAGON_GROWL;
}
const SoundEvent* EnderDragon::getHurtSound(const DamageSource*)
{
    return SoundEvents::ENDERDRAGON_HURT;
}
float EnderDragon::getSoundVolume()
{
    return 5.0f;
}

void EnderDragon::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"DragonState", getSynchedAction());
    Mob::addAdditonalSaveData(tag);
}

void EnderDragon::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->contains(L"DragonState"))
        setSynchedAction(static_cast<EEnderdragonAction>(tag->getInt(L"DragonState")), true);
    Mob::readAdditionalSaveData(tag);
}

double EnderDragon::getHeadPartYOffset(int part, arrayWithLength<double> first, arrayWithLength<double> second)
{
    double result;
    if (getSynchedAction() == ACTION_LANDING || getSynchedAction() == ACTION_TAKEOFF)
    {
        const BlockPos top = mLevel->getTopSolidBlockPos(BlockPos(0, 0, 0));
        double scale = std::sqrt(distanceToSqr(0.0, static_cast<double>(top.y), 0.0)) * 0.25;
        if (scale < 1.0)
            scale = 1.0;
        result = static_cast<double>(part) / scale;
    }
    else if (getSynchedAction() == ACTION_SITTING_FLAMING || getSynchedAction() == ACTION_SITTING_SCANNING
             || getSynchedAction() == ACTION_SITTING_ATTACKING)
    {
        result = static_cast<double>(part);
    }
    else if (part == 6)
    {
        result = 0.0;
    }
    else
    {
        result = second.data[1] - first.data[1];
    }
    return result;
}

double EnderDragon::getHeadPartYRotDiff(int, arrayWithLength<double> first, arrayWithLength<double> second)
{
    return second.data[0] - first.data[0];
}

unsigned int EnderDragon::GetInteractTooltip(const EntityTooltipDataHolder&)
{
    return 0xffffffffU;
}
unsigned long long EnderDragon::GetType()
{
    return EntityType::ENDER_DRAGON;
}

Entity* EnderDragon::create(Level* level)
{
    return new EnderDragon(level);
}
