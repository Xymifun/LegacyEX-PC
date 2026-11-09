#include "game/world/generation/ArrowLayer.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/Cube.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/util/Random.hpp"

ArrowLayer::ArrowLayer(LivingEntityRenderer* renderer) : mRenderer(renderer)
{
}

ArrowLayer::~ArrowLayer()
{
}

void ArrowLayer::render(boost::shared_ptr<LivingEntity>& entity, float, float, float partialTicks, float, float, float,
                        float scale, bool)
{
    int arrowCount = entity.get()->getArrowCount();
    if (arrowCount < 1)
        return;

    boost::shared_ptr<Entity> arrow = boost::shared_ptr<TippableArrow>(
        new TippableArrow(entity.get()->mLevel, entity.get()->mX, entity.get()->mY, entity.get()->mZ));
    Random random(entity.get()->getId());
    Lighting::turnOff();

    for (int i = 0; i < arrowCount; ++i)
    {
        Model* model = mRenderer->getModel();
        ModelPart* modelPart = model->getRandomModelPart(random);
        if (!modelPart->mCubes.empty())
        {
            GlStateManager::pushMatrix();
            Cube* cube = modelPart->mCubes.at(random.nextInt(static_cast<int>(modelPart->mCubes.size())));
            modelPart->translateTo(0.0625f);

            float x = random.nextFloat();
            float y = random.nextFloat();
            float z = random.nextFloat();
            float xOffset = cube->mX0 + (cube->mX1 - cube->mX0) * x;
            float yOffset = cube->mY0 + (cube->mY1 - cube->mY0) * y;
            float zOffset = cube->mZ0 + (cube->mZ1 - cube->mZ0) * z;
            GlStateManager::translatef(xOffset / 16.0f, yOffset / 16.0f, zOffset / 16.0f);

            x = x * 2.0f - 1.0f;
            y = y * 2.0f - 1.0f;
            z = z * 2.0f - 1.0f;
            x = x * -1.0f;
            y = y * -1.0f;
            z = z * -1.0f;
            float horizontal = Mth::sqrt(x * x + z * z);
            arrow.get()->mYRot = std::atan2(x, z) * Mth::RAD_TO_DEG;
            arrow.get()->mXRot = std::atan2(y, horizontal) * Mth::RAD_TO_DEG;
            arrow.get()->mYRotO = arrow.get()->mYRot;
            arrow.get()->mXRotO = arrow.get()->mXRot;

            mRenderer->getDispatcher()->render(arrow, 0.0, 0.0, 0.0, 0.0f, partialTicks, false, false, true, false);
            GlStateManager::popMatrix();
        }
    }

    Lighting::turnOn();
}

bool ArrowLayer::colorsOnDamage()
{
    return 0;
}
