#include "game/client/render/PistonHeadRenderer.hpp"
#include "game/client/render/Textures.hpp"

#include "game/client/render/BlockRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/DefaultVertexFormat.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/PistonHeadBlock.hpp"
#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/world/Level.hpp"
#include "game/core/Boxed.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/AbstractProperty.hpp"


PistonHeadRenderer::PistonHeadRenderer()
{
    mBlockRenderer = NULL;
}

PistonHeadRenderer::~PistonHeadRenderer()
{
}

void PistonHeadRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z,
                                float partialTick, int breakProgress, bool bTranslucent, float alpha,
                                bool bUseDisplayList)
{
    boost::shared_ptr<PistonMovingBlockEntity> movingBlock = boost::static_pointer_cast<PistonMovingBlockEntity>(
        blockEntity);
    if (movingBlock == NULL)
        return;

    BlockPos position = movingBlock->getBlockPos();
    const BlockState* movedState = movingBlock->getBlockState();
    const Block* movedBlock = movedState->getBlock();
    if (movedState->getMaterial() == Material::AIR || movingBlock->getProgress(partialTick) > 1.0F)
        return;

    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    bindTexture(&BLOCKS_LOCATION);
    Lighting::turnOff();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::enableBlend();
    GlStateManager::disableCull();
    Minecraft::useAmbientOcclusion();
    GlStateManager::shadeModel(0);
    builder->begin();
    builder->setVertexFormat(DefaultVertexFormat::BLOCK);

    builder->offset((float)x - (float)position.x + movingBlock->getXOff(partialTick),
                    (float)y - (float)position.y + movingBlock->getYOff(partialTick),
                    (float)z - (float)position.z + movingBlock->getZOff(partialTick));
    builder->color(1.0F, 1.0F, 1.0F);
    onNewLevel(getLevel());

    if (movedBlock == Blocks::piston_head
        && movingBlock->getProgress(partialTick) <= PistonBaseBlock::PLATFORM_THICKNESS)
    {
        const BlockState* headState = movedState->setValue(PistonHeadBlock::SHORT, true);
        mBlockRenderer->tesselatePistonArmNoCulling(headState, position, false, headState);
    }
    else if (movingBlock->isSourcePiston() && !movingBlock->isExtending())
    {
        PistonHeadBlock::PistonType* pistonType = movedBlock == Blocks::sticky_piston ? pistonTypeSticky
                                                                                      : pistonTypeNormal;
        const BlockState* headState =
            Blocks::piston_head->defaultBlockState()
                ->setValue(PistonHeadBlock::TYPE, pistonType)
                ->setValue(PistonHeadBlock::FACING, movedState->getValue(PistonBaseBlock::FACING))
                ->setValue(PistonHeadBlock::SHORT, movingBlock->getProgress(partialTick) >= 0.5F);
        mBlockRenderer->tesselatePistonArmNoCulling(headState, position, movingBlock->getProgress(partialTick) < 0.5F,
                                                    headState);

        builder->offset((float)x - (float)position.x, (float)y - (float)position.y, (float)z - (float)position.z);
        movedState->setValue(PistonBaseBlock::EXTENDED, true);
        mBlockRenderer->tesselatePistonBaseForceExtended(movedState, position, movedState);
    }
    else
    {
        mBlockRenderer->tesselateInWorldNoCulling(movedState, position, movedState, movingBlock);
    }

    builder->offset(0.0F, 0.0F, 0.0F);
    tesselator->end();
    Lighting::turnOn();
}

void PistonHeadRenderer::onNewLevel(Level* level)
{
    delete mBlockRenderer;
    mBlockRenderer = new BlockRenderer(level);
}
