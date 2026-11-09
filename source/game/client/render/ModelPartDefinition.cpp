#include "game/client/render/ModelPartDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartCubeDefinition.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/Mth.hpp"

ModelPartDefinition::ModelPartDefinition()
    : mX(0.0f),
      mY(0.0f),
      mZ(0.0f),
      mXRot(0.0f),
      mYRot(0.0f),
      mZRot(0.0f),
      mCubeXRot(0.0f),
      mCubeYRot(0.0f),
      mCubeZRot(0.0f),
      mCubeCount(0),
      mCubes(0)
{
}

ModelPartDefinition::~ModelPartDefinition()
{
    delete[] mCubes;
}

void ModelPartDefinition::SetPartPosition(ModelPart* part, ModelPart* parent) const
{
    part->setWorldPos(mX, mY, mZ);
    if (parent != 0)
        part->setPos(mX - parent->mWorldX, mY - parent->mWorldY, mZ - parent->mWorldZ);
    else
        part->setPos(mX, mY, mZ);
}

void ModelPartDefinition::SetPartRotation(ModelPart* part) const
{
    part->mXRot = mXRot;
    part->mYRot = mYRot;
    part->mZRot = mZRot;
    part->mCubeXRot = mCubeXRot;
    part->mCubeYRot = mCubeYRot;
    part->mCubeZRot = mCubeZRot;
}

void ModelPartDefinition::AddCubes(ModelPart* part, float scale) const
{
    for (int i = 0; i < mCubeCount; ++i)
    {
        const ModelPartCubeDefinition& cube = mCubes[i];
        float delta = cube.inflate + scale;
        part->addBoxMapTex(cube.x, cube.y, cube.z, cube.width, cube.height, cube.depth, static_cast<int>(cube.texU),
                           static_cast<int>(cube.texV), cube.mirror, delta);
    }
}

void ModelPartDefinition::SetupModelPart(ModelPart* part, ModelPart* parent, float scale) const
{
    SetPartPosition(part, parent);
    SetPartRotation(part);
    AddCubes(part, scale);
    if (parent != 0)
        parent->addChild(part);
}

void ModelPartDefinition::readModelPart(DataInputStream* input, int hasCubeRotation)
{
#ifdef MINECRAFT_PC
    if (hasCubeRotation >= 2)
        input->readUTF();
#endif
    mX = input->readFloat();
    mY = input->readFloat();
    mZ = input->readFloat();
    mXRot = input->readFloat() * Mth::DEG_TO_RAD;
    mYRot = input->readFloat() * Mth::DEG_TO_RAD;
    mZRot = input->readFloat() * Mth::DEG_TO_RAD;
    if (hasCubeRotation > 0)
    {
        mCubeXRot = input->readFloat() * Mth::DEG_TO_RAD;
        mCubeYRot = input->readFloat() * Mth::DEG_TO_RAD;
        mCubeZRot = input->readFloat() * Mth::DEG_TO_RAD;
    }
    mCubeCount = input->readInt();
    mCubes = new ModelPartCubeDefinition[mCubeCount];
    int i;
    for (i = 0; i < mCubeCount; ++i)
        mCubes[i].readModelPartCube(input);
}
