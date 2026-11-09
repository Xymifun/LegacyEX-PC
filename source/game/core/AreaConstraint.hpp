#ifndef GAME_CORE_AREACONSTRAINT_HPP
#define GAME_CORE_AREACONSTRAINT_HPP

#include "game/minigame/TutorialConstraint.hpp"

class AABB;
class Vec3;

class AreaConstraint : public TutorialConstraint
{
public:
    AreaConstraint(int messageId, double x0, double y0, double z0, double x1, double y1, double z1, bool insideArea,
                   bool constrainHorizontally, bool constrainVertically, int border);

    virtual ~AreaConstraint();

    virtual int getType();
    virtual bool isConstraintSatisfied(int playerId);
    virtual bool isConstraintRestrictive(int playerId);
    virtual int canMoveToPosition(double oldX, double oldY, double oldZ, double newX, double newY, double newZ,
                                  Vec3* clampedPosition, bool positionOnly);

    AABB* area;
    AABB* innerArea;
    bool insideArea;
    bool constrainHorizontally;
    bool constrainVertically;
};

#endif
