#include "game/client/render/ModelDefinition.hpp"

#include "game/client/render/ModelPartDefinition.hpp"
#include "game/core/DataInputStream.hpp"

static const ModelPartDefinition sMissingModelPart;

ModelDefinition::ModelDefinition()
{
    mOverridesModel = false;
    mTextureWidth = 0;
    mTextureHeight = 0;
}

bool ModelDefinition::hasModelPart(const std::wstring& partName) const
{
    return mModelParts.find(partName) != mModelParts.end();
}

const ModelPartDefinition& ModelDefinition::getModelPart(const std::wstring& partName) const
{
    ModelPartMap::const_iterator part = mModelParts.find(partName);

    if (part != mModelParts.end())
    {
        return part->second;
    }

    return sMissingModelPart;
}

const ModelPartDefinition& ModelDefinition::operator[](const std::wstring& partName) const
{
    ModelPartMap::const_iterator part = mModelParts.find(partName);

    if (part != mModelParts.end())
    {
        return part->second;
    }

    return sMissingModelPart;
}

bool ModelDefinition::DoesOverrideModel() const
{
    return mOverridesModel;
}

int ModelDefinition::getTextureWidth() const
{
    return mTextureWidth;
}

int ModelDefinition::getTextureHeight() const
{
    return mTextureHeight;
}

void ModelDefinition::readModel(DataInputStream* input, int version)
{
    mOverridesModel = true;
    mTextureWidth = input->readInt();
    mTextureHeight = input->readInt();

    int numParts = input->readInt();

    for (int i = 0; i < numParts; ++i)
    {
        mModelParts[input->readUTF()].readModelPart(input, version);
    }
}
