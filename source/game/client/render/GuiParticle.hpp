#ifndef GAME_CLIENT_RENDER_GUIPARTICLE_HPP
#define GAME_CLIENT_RENDER_GUIPARTICLE_HPP

#include <stdint.h>

class GuiParticles;

class GuiParticle
{
public:
    GuiParticle(double x, double y, double z, double alpha);

    void preTick();
    void remove();
    void tick(GuiParticles* particles);

    double x;
    double y;
    double xo;
    double yo;
    double xd;
    double yd;
    double gravity;
    bool removed;
    int age;
    int lifetime;
    double z;
    double extra0;
    double extra1;
    double alpha;
    double zo;
    double extra0o;
    double extra1o;
    double alphao;
};

#endif
