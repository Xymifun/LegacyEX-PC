#include "game/entity/ai/Goal.hpp"

Goal::Goal()
{
    Goal* obj = this;
    if (obj != NULL || (obj = static_cast<Goal*>(operator new(8)), obj != NULL))
    {
        obj->mRequiredControlFlags = 0;
    }
}

bool Goal::canContinueToUse()
{
    return canUse();
}

bool Goal::canInterrupt()
{
    return true;
}

void Goal::start()
{
}

void Goal::stop()
{
}

void Goal::tick()
{
}

void Goal::setRequiredControlFlags(int flags)
{
    mRequiredControlFlags = flags;
}

int Goal::getRequiredControlFlags()
{
    return mRequiredControlFlags;
}

void Goal::setLevel(Level* level)
{
    (void)level;
}

std::wstring Goal::toString()
{
    return getName();
}
