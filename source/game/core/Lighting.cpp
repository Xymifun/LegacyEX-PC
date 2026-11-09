#include "game/core/Lighting.hpp"

#include "game/core/GlStateManager.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/util/Vec3.hpp"

static FloatBuffer* BUFFER = MemoryTracker::createFloatBuffer(16);

FloatBuffer* Lighting::getBuffer(float x, float y, float z, float w)
{
    BUFFER->clear();
    BUFFER->put(x)->put(y)->put(z)->put(w);


    BUFFER->flip();

    return BUFFER;
}

FloatBuffer* Lighting::getBuffer(double x, double y, double z, double w)
{
    return getBuffer((float)x, (float)y, (float)z, (float)w);
}

void Lighting::turnOn()
{
    GlStateManager::enableLighting();
    GlStateManager::enableLight(8);
    GlStateManager::enableLight(9);
    GlStateManager::enableColorMaterial();
    GlStateManager::colorMaterial(2, 0);

    Vec3* frontLight = Vec3::newTemp(0.2f, 1.0f, -0.7f)->normalize();
    GlStateManager::light(8, 1, getBuffer(frontLight->x, frontLight->y, frontLight->z, 0.0));
    GlStateManager::light(8, 3, getBuffer(0.6f, 0.6f, 0.6f, 1.0f));
    GlStateManager::light(8, 2, getBuffer(0.0f, 0.0f, 0.0f, 1.0f));
    GlStateManager::light(8, 4, getBuffer(0.0f, 0.0f, 0.0f, 1.0f));

    Vec3* backLight = Vec3::newTemp(-0.2f, 1.0f, 0.7f)->normalize();
    GlStateManager::light(9, 1, getBuffer(backLight->x, backLight->y, backLight->z, 0.0));
    GlStateManager::light(9, 3, getBuffer(0.6f, 0.6f, 0.6f, 1.0f));
    GlStateManager::light(9, 2, getBuffer(0.0f, 0.0f, 0.0f, 1.0f));
    GlStateManager::light(9, 4, getBuffer(0.0f, 0.0f, 0.0f, 1.0f));

    GlStateManager::shadeModel(0);

    GlStateManager::lightModel(1, getBuffer(0.4f, 0.4f, 0.4f, 1.0f));
}

void Lighting::turnOff()
{
    GlStateManager::disableLighting();
    GlStateManager::disableLight(8);
    GlStateManager::disableLight(9);
    GlStateManager::disableColorMaterial();
}

void Lighting::turnOnGui()
{
    GlStateManager::pushMatrix();
    GlStateManager::rotatef(120.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(20.0f, 0.0f, 1.0f, 0.0f);
    turnOn();
    GlStateManager::popMatrix();
}
