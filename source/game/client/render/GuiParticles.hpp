#ifndef GAME_CLIENT_RENDER_GUIPARTICLES_HPP
#define GAME_CLIENT_RENDER_GUIPARTICLES_HPP

#include <vector>

#include "game/core/GuiComponent.hpp"
#include "game/client/render/GuiParticle.hpp"

class Minecraft;

class GuiParticles : public GuiComponent
{
public:
    GuiParticles(Minecraft* minecraft);

    void tick();
    void render(float partialTicks);
    void add(GuiParticle* particle);

private:
    std::vector<GuiParticle*> mParticles;
    Minecraft* mMinecraft;
};

#endif
