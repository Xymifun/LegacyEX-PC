#include "game/client/render/NoteParticle.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"

NoteParticle::NoteParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    init(level, x, y, z, xa, ya, za, 2.0f);
}

void NoteParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                          float ya, float za, float xa2, float za2)
{
    float progress = ((age + partialTicks) / lifetime) * 32.0f;
    progress = Mth::clamp(progress, 0.0f, 1.0f);
    size = scale * progress;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void NoteParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime)
    {
        remove();
    }

    move(xd, yd, zd);

    if (y == yo)
    {
        xd *= 1.1;
        zd *= 1.1;
    }

    xd *= 0.6600000262260437;
    yd *= 0.6600000262260437;
    zd *= 0.6600000262260437;

    if (onGround)
    {
        xd *= 0.699999988079071;
        zd *= 0.699999988079071;
    }
}

void NoteParticle::init(Level* level, double x, double y, double z, double xa, double ya, double za, float factor)
{
    xd *= 0.01;
    yd *= 0.01;
    zd *= 0.01;
    yd += 0.2;

    int note = eMinecraftColour_Particle_Note_00 + (int)std::floor(xa * 24.0 + 0.5);
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour((eMinecraftColour)note);

    rCol = (float)((colour >> 16) & 255) / 255.0f;
    gCol = (float)((colour >> 8) & 255) / 255.0f;
    bCol = (float)(colour & 255) / 255.0f;

    size *= 0.75f;
    size *= factor;
    scale = size;
    lifetime = 6;
    setMiscTex(64);
}

uint64_t NoteParticle::GetType()
{
    return ParticleKind::NOTE;
}

Particle* NoteParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                 double xa, double ya, double za, arrayWithLength<int> params)
{
    return new NoteParticle(level, x, y, z, xa, ya, za);
}
