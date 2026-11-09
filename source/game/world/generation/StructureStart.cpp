#include "game/world/generation/StructureStart.hpp"

#include "game/core/BoundingBox.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"

StructureStart::StructureStart()
{
    mChunkZ = 0;
    mChunkX = 0;
    mBoundingBox = NULL;
    mDirty = true;
}

StructureStart::StructureStart(int chunkX, int chunkZ)
{
    mChunkX = chunkX;
    mChunkZ = chunkZ;
    mBoundingBox = NULL;
    mDirty = true;
}

StructureStart::~StructureStart()
{
    for (std::vector<StructurePiece*>::iterator it = mPieces.begin(); it != mPieces.end(); ++it)
    {
        delete *it;
    }
    delete mBoundingBox;
}

void StructureStart::setDirty()
{
    mDirty = true;
}

void StructureStart::postProcess(Level* level, Random& random, BoundingBox* box)
{
    std::vector<StructurePiece*>::iterator it = mPieces.begin();
    while (it != mPieces.end())
    {
        StructurePiece* piece = *it;
        if (piece->getBoundingBox()->intersects(box) && !piece->postProcess(level, random, box))
        {
            it = mPieces.erase(it);
        }
        else
        {
            if (piece->isDirty())
            {
                setDirty();
                piece->clearDirty();
            }
            ++it;
        }
    }
}

void StructureStart::calculateBoundingBox()
{
    mBoundingBox = BoundingBox::getUnknownBox();
    for (std::vector<StructurePiece*>::iterator it = mPieces.begin(); it != mPieces.end(); ++it)
    {
        mBoundingBox->expand((*it)->getBoundingBox());
    }
}

arrayWithLength<unsigned char> StructureStart::createTag(int chunkX, int chunkZ)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream output(&byteStream);
    output.writeInt(4);
    output.writeUTF(StructureFeatureIO::getEncodeId(this));
    output.writeInt(chunkX);
    output.writeInt(chunkZ);
    mBoundingBox->write(&output);
    output.writeInt((int)mPieces.size());
    for (std::vector<StructurePiece*>::iterator it = mPieces.begin(); it != mPieces.end(); ++it)
    {
        (*it)->write(&output);
    }
    addAdditionalSaveData(&output);
    return byteStream.toByteArray();
}

void StructureStart::addAdditionalSaveData(DataOutputStream* output)
{
}

void StructureStart::load(Level* level, DataInputStream* input)
{
    mChunkX = input->readInt();
    mChunkZ = input->readInt();
    mBoundingBox = new BoundingBox();
    mBoundingBox->read(input);
    int count = input->readInt();
    for (int i = 0; i < count; ++i)
    {
        StructurePiece* piece = StructureFeatureIO::loadStaticPiece(input, level);
        if (piece != NULL)
        {
            mPieces.push_back(piece);
        }
    }
    readAdditionalSaveData(input);
}

void StructureStart::readAdditionalSaveData(DataInputStream* input)
{
}

void StructureStart::moveBelowSeaLevel(Level* level, Random& random, int offset)
{
    int limit = level->getSeaLevel() - offset;
    int y = mBoundingBox->getYSpan() + 1;
    if (y < limit)
    {
        y += random.nextInt(limit - y);
    }
    int delta = y - mBoundingBox->y1;
    mBoundingBox->move(0, delta, 0);
    for (std::vector<StructurePiece*>::iterator it = mPieces.begin(); it != mPieces.end(); ++it)
    {
        (*it)->move(0, delta, 0);
    }
}

void StructureStart::moveInsideHeights(Level* level, Random& random, int minHeight, int maxHeight)
{
    int spare = maxHeight - minHeight + 1 - mBoundingBox->getYSpan();
    int delta;
    if (spare > 1)
    {
        delta = minHeight + random.nextInt(spare) - mBoundingBox->y0;
    }
    else
    {
        delta = minHeight - mBoundingBox->y0;
    }
    mBoundingBox->move(0, delta, 0);
    for (std::vector<StructurePiece*>::iterator it = mPieces.begin(); it != mPieces.end(); ++it)
    {
        (*it)->move(0, delta, 0);
    }
}

bool StructureStart::isValid()
{
    return true;
}

bool StructureStart::isValidForPostProcess(ChunkPos* chunkPos)
{
    return true;
}

void StructureStart::notifyPostProcessAt(ChunkPos* chunkPos)
{
}
