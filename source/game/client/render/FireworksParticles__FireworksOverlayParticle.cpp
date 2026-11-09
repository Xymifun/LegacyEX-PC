#include "game/client/render/FireworksParticles__FireworksOverlayParticle.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/core/BufferBuilder.hpp"

FireworksParticles::FireworksOverlayParticle::FireworksOverlayParticle(Level* level, double x, double y, double z)
    : Particle(level, x, y, z)
{
    lifetime = 4;
    alpha = 0.99f;
}

void FireworksParticles::FireworksOverlayParticle::render(BufferBuilder* buffer,
                                                          const boost::shared_ptr<Entity>& camera, float partialTicks,
                                                          float xa, float ya, float za, float xa2, float za2)
{
    (void)camera;
    float scale = 7.1f * (float)std::sin((age + partialTicks - 1.0f) * 0.25f * 3.1415927f);
    setAlpha(0.6f - (age + partialTicks - 1.0f) * 0.25f * 0.5f);

    float px = (float)((x - xo) * partialTicks + xo - xOff);
    float py = (float)((y - yo) * partialTicks + yo - yOff);
    float pz = (float)((z - zo) * partialTicks + zo - zOff);

    if (px * px + py * py + pz * pz >= 3.0f)
    {
        int texture = getLightColor(partialTicks);
        buffer->color(rCol, gCol, bCol, getAlpha());
        buffer->tex2(texture);

        float x0 = px - xa * scale;
        float y0 = py - ya * scale;
        float z0 = pz - za * scale;
        float x1 = xa2 * scale;
        float z1 = za2 * scale;

        buffer->vertexUV(x0 - x1, y0, z0 - z1, 0.5f, 0.375f);
        buffer->vertexUV(x0 + x1, py + ya * scale, z0 + z1, 0.5f, 0.125f);
        x0 = px + xa * scale;
        z0 = pz + za * scale;
        buffer->vertexUV(x0 + x1, py + ya * scale, z0 + z1, 0.25f, 0.125f);
        buffer->vertexUV(x0 - x1, y0, z0 - z1, 0.25f, 0.375f);
    }
}

uint64_t FireworksParticles::FireworksOverlayParticle::GetType()
{
    return ParticleKind::FIREWORKS_OVERLAY;
}
