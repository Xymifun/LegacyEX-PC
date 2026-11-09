#ifndef GAME_MINIGAME_TUTORIALCONSTRAINT_HPP
#define GAME_MINIGAME_TUTORIALCONSTRAINT_HPP

class Vec3;

class TutorialConstraint
{
public:
    TutorialConstraint(int messageId) : messageId(messageId), deleteWhenRemoved(false), removalPending(false) {}

    virtual ~TutorialConstraint() {}

    virtual int getType() = 0;
    virtual void tick(int playerId);
    virtual bool isConstraintSatisfied(int playerId);
    virtual bool isConstraintRestrictive(int playerId);
    virtual bool isMappingConstrained(int playerId, int mapping);
    virtual bool isXuiInputConstrained(int xuiInput);
    virtual int canMoveToPosition(double oldX, double oldY, double oldZ, double newX, double newY, double newZ,
                                  Vec3* clampedPosition, bool positionOnly);

    int messageId;
    bool deleteWhenRemoved;
    bool removalPending;
};

#endif
