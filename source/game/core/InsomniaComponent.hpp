#ifndef GAME_CORE_INSOMNIACOMPONENT_HPP
#define GAME_CORE_INSOMNIACOMPONENT_HPP

class CompoundTag;
class LivingEntity;

class InsomniaComponent
{
public:
    InsomniaComponent(LivingEntity* entity, float daysUntilInsomnia);

    void restartTimer();
    void tick();

    bool hasInsomnia() const;
    void setInsomnia(int timerTicks);

    int getInsomniaTimerTicks() const;
    int getTicksUntilInsomnia() const;

    void addAdditionalSaveData(CompoundTag* tag);
    void readAdditionalSaveData(CompoundTag* tag);

private:
    LivingEntity* mEntity;
    int mInsomniaTimerTicks;
    float mDaysUntilInsomnia;
    int mTicksUntilInsomnia;
};

#endif
