#ifndef GAME_CLIENT_RENDER_MODELPARTSKIN_HPP
#define GAME_CLIENT_RENDER_MODELPARTSKIN_HPP

#include <string>

#include "game/client/render/ModelPart.hpp"

class Model;
class ModelDefinition;
class ModelPartsSkinned;

class ModelPartSkin : public ModelPart
{
public:
    ModelPartSkin();
    ModelPartSkin(Model* model);
    ModelPartSkin(Model* model, const std::wstring& name);
    ModelPartSkin(Model* model, int xTexOffs, int yTexOffs, ModelPartsSkinned* skinnedParts);
    ModelPartSkin(Model* model, int xTexOffs, int yTexOffs, int texWidth, int texHeight);
    ModelPartSkin(Model* model, const std::wstring& name, const ModelDefinition& definition, ModelPart* parent,
                  float scale, ModelPartsSkinned* skinnedParts);

    virtual ~ModelPartSkin();

    virtual void render(float scale, bool useDisplayList, bool childrenOnly);
    virtual void renderRollable(float scale, bool useDisplayList);
    virtual void compile(float scale);

    void setNullBoneMatricesRecursive();

    int mBoneIndex;
    ModelPartsSkinned* mSkinnedParts;
};

#endif
