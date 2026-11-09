#include "game/core/Camera.hpp"

#include <cmath>
#include <cstring>

#include <cafe/mtx.h>

#include "game/core/CameraController.hpp"
#include "game/core/FloatBuffer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModViews.hpp"
#endif

FloatBuffer* gCameraModelViewMatrix = MemoryTracker::createFloatBuffer(16);
FloatBuffer* gCameraProjectionMatrix = MemoryTracker::createFloatBuffer(16);
float gCameraPositionX = 0.0f;
float gCameraPositionY = 0.0f;
float gCameraPositionZ = 0.0f;
float gCameraXOff = 0.0f;
float gCameraYOff = 0.0f;
float gCameraZOff = 0.0f;
float gCameraYOff2 = 0.0f;
float gCameraXOff2 = 0.0f;
float gCameraOffsetsX[4] = {0.0f, 0.0f, 0.0f, 0.0f};
float gCameraOffsetsY[4] = {0.0f, 0.0f, 0.0f, 0.0f};
float gCameraOffsetsZ[4] = {0.0f, 0.0f, 0.0f, 0.0f};

void Camera::prepare(boost::shared_ptr<Player> player, bool secondPass)
{
    GlStateManager::getMatrix(0, gCameraModelViewMatrix);
    GlStateManager::getMatrix(1, gCameraProjectionMatrix);

    Mtx44 modelView;
    Mtx44 projection;
    Mtx44 combined;
    Mtx44 inverse;
    std::memcpy(modelView, gCameraModelViewMatrix->mBuffer, sizeof(modelView));
    std::memcpy(projection, gCameraProjectionMatrix->mBuffer, sizeof(projection));
    ASM_MTX44Concat(modelView, projection, combined);
    C_MTX44Inverse(combined, inverse);
#ifdef MINECRAFT_PC
    if (PcModViewsInView())
        C_MTX44Inverse(modelView, inverse);
#endif
    gCameraPositionX = inverse[3][0] / inverse[3][3];
    gCameraPositionY = inverse[3][1] / inverse[3][3];
    gCameraPositionZ = inverse[3][2] / inverse[3][3];

    CameraController* controller = player->GetCameraController();
    float xRotation = player->mXRot;
    float yRotation = player->mYRot;
    if (controller != NULL && player->GetThirdPersonView(false) == 3)
    {
        xRotation += (float)controller->getCurrentRot()->x;
        yRotation += (float)controller->getCurrentRot()->y;
    }

    float cosYaw = cosf(yRotation * 0.017453292f);
    float side = (float)(1 - 2 * (int)secondPass);
    gCameraXOff = cosYaw * side;
    gCameraZOff = sinf(yRotation * 0.017453292f) * side;
    gCameraYOff2 = -gCameraZOff * sinf(xRotation * 0.017453292f) * side;
    gCameraXOff2 = gCameraXOff * sinf(xRotation * 0.017453292f) * side;
    gCameraYOff = cosf(xRotation * 0.017453292f);

    int index = dynamic_cast<MultiplayerLocalPlayer*>(player.get())->mUserIndex;
    gCameraOffsetsX[index] = gCameraPositionX;
    gCameraOffsetsY[index] = gCameraPositionY;
    gCameraOffsetsZ[index] = gCameraPositionZ;
}

Vec3* Camera::getCameraPos(boost::shared_ptr<Entity> entity, double partialTick)
{
    double x = (entity->mX - entity->mXo) * partialTick + entity->mXo;
    double y = (entity->mY - entity->mYo) * partialTick + entity->mYo;
    double z = (entity->mZ - entity->mZo) * partialTick + entity->mZo;
    return Vec3::newTemp(x + gCameraPositionX, y + gCameraPositionY, z + gCameraPositionZ);
}

BlockPos Camera::getCameraBlockPos(boost::shared_ptr<LivingEntity> entity, double partialTick)
{
    return BlockPos(getCameraPos(entity, partialTick));
}

const BlockState* Camera::getBlockAt(Level* level, boost::shared_ptr<Entity> entity, float partialTick)
{
    Vec3* position = getCameraPos(entity, partialTick);
    BlockPos blockPos(position);
    const BlockState* state = level->getBlockState(blockPos);
    unsigned short extraElement = level->getExtraElement(blockPos.x, blockPos.y, blockPos.z);
    if (extraElement != 0)
    {
        state = Block::getStateByElementId(extraElement);
    }

    if (state->getMaterial()->isLiquid())
    {
        float height = 0.0f;
        if (state->getBlock()->isLiquidBlock())
        {
            height = LiquidBlock::getHeightFromData(state->getValue(LiquidBlock::LEVEL)) - 0.11111111f;
        }

        float top = (float)(blockPos.y + 1) - height;
        if (position->y >= top)
        {
            state = level->getBlockState(blockPos.above());
        }
    }
    return state;
}

void Camera::UpdatePlayerOffsets(boost::shared_ptr<Player> player)
{
    int index = dynamic_cast<MultiplayerLocalPlayer*>(player.get())->mUserIndex;
    gCameraPositionX = gCameraOffsetsX[index];
    gCameraPositionY = gCameraOffsetsY[index];
    gCameraPositionZ = gCameraOffsetsZ[index];
}
