#ifndef GAME_CLIENT_RENDER_MODELDEFINITION_HPP
#define GAME_CLIENT_RENDER_MODELDEFINITION_HPP

#include <string>

#include <boost/unordered_map.hpp>

#include "game/client/render/ModelPartDefinition.hpp"

class DataInputStream;

class ModelDefinition
{
public:
    typedef boost::unordered_map<std::wstring, ModelPartDefinition> ModelPartMap;

    ModelDefinition();
    ~ModelDefinition() {}

    bool hasModelPart(const std::wstring& partName) const;
    const ModelPartDefinition& getModelPart(const std::wstring& partName) const;
    const ModelPartDefinition& operator[](const std::wstring& partName) const;

    bool DoesOverrideModel() const;
    int getTextureWidth() const;
    int getTextureHeight() const;

    void readModel(DataInputStream* input, int version);

private:
    bool mOverridesModel;
    int mTextureWidth;
    int mTextureHeight;
    ModelPartMap mModelParts;
};

#endif
