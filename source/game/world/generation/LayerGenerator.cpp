#include "game/world/generation/LayerGenerator.hpp"

#include <vector>

#include "game/core/Shape.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/world/generation/FunctionLayerFill.hpp"
#include "game/world/generation/LayerShape.hpp"
#include "game/world/generation/LayerTheme.hpp"
#include "game/core/PIXNamedEvents.hpp"

LayerGenerator::LayerGenerator(AABB* area, const LayerShape* shape, const LayerFill* fill,
                               const std::vector<const LayerTheme*>& themes)
    : mShape(shape),
      mFill(fill),
      mThemes(themes),
      mArea(area),
      mCoreWorldMinY(1000),
      mCoreWorldMaxY(0)
{
}

LayerGenerator::~LayerGenerator()
{
    delete mArea;
}

AABB* LayerGenerator::getArea() const
{
    return mArea;
}

int LayerGenerator::getCoreWorldMinY() const
{
    return mCoreWorldMinY + static_cast<int>(mArea->mMinY);
}

BlockTransaction* LayerGenerator::Generate(Random& random, Level*)
{
    PIXBeginNamedEvent(0.0f, "LayerGenerator::Setup");
    BlockTransaction* transaction = BlockTransaction::CreateTransaction(
        BlockPos(mArea->mMinX, mArea->mMinY, mArea->mMinZ), BlockPos(mArea->mMaxX, mArea->mMaxY, mArea->mMaxZ));
    Shape shape(transaction->mSize.x, transaction->mSize.z);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "LayerGenerator::GenShape");
    mShape->GenerateShape(random, shape);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "LayerGenerator::ApplyFill");
    mFill->ExtrudeLayer(random, shape, transaction->mSize, transaction, &mCoreWorldMinY, &mCoreWorldMaxY);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "LayerGenerator::ApplyThemes");
    std::vector<const LayerTheme*>::const_iterator end = mThemes.end();
    for (std::vector<const LayerTheme*>::const_iterator it = mThemes.begin(); it != end; ++it)
    {
        (*it)->Apply(random, transaction, mCoreWorldMinY, mCoreWorldMaxY);
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "LayerGenerator::reduceMinMax");
    transaction->reduceMinMax();
    PIXEndNamedEvent();
    return transaction;
}

void LayerGenerator::updateArea(AABB* area)
{
    delete mArea;
    mArea = area;
}

int LayerGenerator::getCoreWorldMaxY() const
{
    return mCoreWorldMaxY + static_cast<int>(mArea->mMinY);
}
