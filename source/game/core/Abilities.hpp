#ifndef GAME_CORE_ABILITIES_HPP
#define GAME_CORE_ABILITIES_HPP

class CompoundTag;
class DataInputStream;
class DataOutputStream;

class Abilities
{
public:
    Abilities();

    bool operator==(const Abilities& other) const;

    void addSaveData(CompoundTag* tag);
    void loadSaveData(CompoundTag* tag);

    float getFlyingSpeed();
    void setFlyingSpeed(float speed);
    float getWalkingSpeed();
    void setWalkingSpeed(float speed);

    void write(DataOutputStream* output);
    void read(DataInputStream* input);

    bool mInvulnerable;
    bool mFlying;
    bool mMayfly;
    bool mInstabuild;
    bool mMayBuild;

private:
    float mFlyingSpeed;
    float mWalkingSpeed;
};

#endif
