#include "game/client/render/GuiParticle.hpp"

void GuiParticle::preTick()
{
    zo = z;
    extra0o = extra0;
    extra1o = extra1;
    alphao = alpha;
    xo = x;
    yo = y;
}

void GuiParticle::remove()
{
    removed = true;
}

void GuiParticle::tick(GuiParticles*)
{
    ++age;
    x += xd;
    y += yd;
    xd *= gravity;
    yd = yd * gravity + 0.1;

    if (age > lifetime)
    {
        remove();
    }

    double progress = (double)age / (double)lifetime;
    progress = 2.0 - progress * 2.0;
    if (progress > 1.0)
    {
        progress = 1.0;
    }
    alpha = progress * progress * 0.5;
}
