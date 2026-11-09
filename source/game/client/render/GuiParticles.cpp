#include "game/client/render/GuiParticles.hpp"

GuiParticles::GuiParticles(Minecraft* minecraft) : mMinecraft(minecraft)
{
}

void GuiParticles::tick()
{
    for (std::vector<GuiParticle*>::size_type i = 0; i < mParticles.size(); ++i)
    {
        GuiParticle* particle = mParticles[i];
        particle->preTick();
        particle->tick(this);
        if (particle->removed)
        {
            mParticles.erase(mParticles.begin() + i);
            --i;
        }
    }
}
