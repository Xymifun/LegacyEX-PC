#ifndef GAME_CLIENT_RENDER_MODELPARTSSKINNED_HPP
#define GAME_CLIENT_RENDER_MODELPARTSSKINNED_HPP

#include <vector>

class ModelPartSkin;

class ModelPartsSkinned
{
public:
    ModelPartsSkinned();
    ~ModelPartsSkinned();

    void addModelPart(ModelPartSkin* part);
    void removeModelPart(ModelPartSkin* part);
    void removeAllModels();

    void compile(float scale);
    void render();

    int mDisplayList;
    bool mCompiled;
    std::vector<ModelPartSkin*> mParts;
};

#endif
