#include "game/core/SchematicTheme.hpp"

#include "game/core/ConsoleSchematicFile.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockTransaction.hpp"

SchematicTheme::SchematicTheme()
{
}

void SchematicTheme::Apply(Random&, BlockTransaction* transaction, int& minY, int& maxY) const
{
    int depth = transaction->mSize.z;
    int width = transaction->mSize.x;
    int size = depth > width ? depth : width;
    ConsoleSchematicFile* schematic = getAssetForSize(size);
    schematic->applySchematicFile(transaction);
    Vec3i bound = transaction->getMin();
    minY = bound.y;
    bound = transaction->getMax();
    maxY = bound.y;
}
