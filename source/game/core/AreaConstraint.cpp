#include "game/core/AreaConstraint.hpp"

#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

AreaConstraint::AreaConstraint(int messageId, double x0, double y0, double z0, double x1, double y1, double z1,
                               bool insideArea, bool constrainHorizontally, bool constrainVertically, int border)
    : TutorialConstraint(messageId)
{
    innerArea = AABB::newPermanent(x0 + border, y0 + border, z0 + border, x1 - border, y1 - border, z1 - border);
    area = AABB::newPermanent(x0, y0, z0, x1, y1, z1);
    this->insideArea = insideArea;
    this->constrainHorizontally = constrainHorizontally;
    this->constrainVertically = constrainVertically;
}

AreaConstraint::~AreaConstraint()
{
    delete innerArea;
    delete area;
}

int AreaConstraint::getType()
{
    return 1;
}

bool AreaConstraint::isConstraintSatisfied(int playerId)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    bool withinHorizontally = true;
    bool withinVertically = true;

    Vec3* pos = minecraft->mLocalPlayers[playerId]->getPos(1.0f);

    if (pos->x <= innerArea->mMinX || pos->x >= innerArea->mMaxX)
    {
        withinHorizontally = false;
    }

    if (pos->z <= innerArea->mMinZ || pos->z >= innerArea->mMaxZ)
    {
        withinHorizontally = false;
    }

    if (pos->y <= innerArea->mMinY || pos->y >= innerArea->mMaxY)
    {
        withinVertically = false;
    }

    bool satisfied = false;

    if (constrainHorizontally)
    {
        if (withinHorizontally == insideArea && (!constrainVertically || withinVertically == insideArea))
        {
            satisfied = true;
        }
    }
    else if (constrainVertically && withinVertically == insideArea)
    {
        satisfied = true;
    }

    return satisfied;
}

bool AreaConstraint::isConstraintRestrictive(int playerId)
{
    return constrainHorizontally || constrainVertically;
}

int AreaConstraint::canMoveToPosition(double oldX, double oldY, double oldZ, double newX, double newY, double newZ,
                                      Vec3* clampedPosition, bool positionOnly)
{
    if (!constrainHorizontally && !constrainVertically)
    {
        return 0;
    }

    Vec3* oldPos = Vec3::newTemp(oldX, oldY, oldZ);
    Vec3* newPos = Vec3::newTemp(newX, newY, newZ);

    bool withinX = true;
    bool withinY = true;
    bool withinZ = true;

    if (positionOnly)
    {
        if (newPos->x < area->mMinX || newPos->x > area->mMaxX)
        {
            withinX = false;
        }

        if (newPos->y < area->mMinY || newPos->y > area->mMaxY)
        {
            withinY = false;
        }

        if (newPos->z < area->mMinZ || newPos->z > area->mMaxZ)
        {
            withinZ = false;
        }
    }
    else
    {
        if ((newPos->x <= area->mMinX && newPos->x < oldPos->x) || (newPos->x >= area->mMaxX && newPos->x > oldPos->x))
        {
            withinX = false;
        }

        if ((newPos->y <= area->mMinY && newPos->y < oldPos->y) || (newPos->y >= area->mMaxY && newPos->y > oldPos->y))
        {
            withinY = false;
        }

        if ((newPos->z <= area->mMinZ && newPos->z < oldPos->z) || (newPos->z >= area->mMaxZ && newPos->z > oldPos->z))
        {
            withinZ = false;
        }
    }

    bool blockedX = constrainHorizontally && withinX != insideArea;
    bool blockedY = constrainVertically && withinY != insideArea;
    bool blockedZ = constrainHorizontally && withinZ != insideArea;

    if (clampedPosition != NULL)
    {
        if (blockedX && !withinX)
        {
            clampedPosition->x = newPos->x < area->mMaxX ? area->mMinX : area->mMaxX;
        }

        if (blockedY && !withinY)
        {
            if (newPos->y < area->mMaxY)
            {
                clampedPosition->y = area->mMinY;
            }
            else
            {
                clampedPosition->y = area->mMaxY;
            }
        }

        if (blockedZ && !withinZ)
        {
            if (newPos->z < area->mMaxZ)
            {
                clampedPosition->z = area->mMinZ;
            }
            else
            {
                clampedPosition->z = area->mMaxZ;
            }
        }
    }

    int blockedAxes;

    if (blockedX)
    {
        if (blockedY)
        {
            blockedAxes = 3;

            if (blockedZ)
            {
                blockedAxes = 7;
            }
        }
        else
        {
            blockedAxes = 1;

            if (blockedZ)
            {
                blockedAxes = 5;
            }
        }
    }
    else if (blockedY)
    {
        blockedAxes = 2;

        if (blockedZ)
        {
            blockedAxes = 6;
        }
    }
    else
    {
        blockedAxes = 0;

        if (blockedZ)
        {
            blockedAxes = 4;
        }
    }

    return blockedAxes;
}
