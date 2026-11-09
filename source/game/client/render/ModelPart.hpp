#ifndef GAME_CLIENT_RENDER_MODELPART_HPP
#define GAME_CLIENT_RENDER_MODELPART_HPP

#include <string>
#include <vector>

class Cube;
class Model;
class ModelDefinition;
struct SKIN_BOX;

class ModelPart
{
public:
    static const float RAD;

    ModelPart();
    ModelPart(Model* model);
    ModelPart(Model* model, int xTexOffs, int yTexOffs);
    ModelPart(Model* model, int xTexOffs, int yTexOffs, int texWidth, int texHeight);
    ModelPart(Model* model, const std::wstring& name);
    ModelPart(Model* model, const std::wstring& name, const ModelDefinition& definition, ModelPart* parent,
              float scale);

    virtual ~ModelPart() {}

    virtual void render(float scale, bool useDisplayList, bool childrenOnly);
    virtual void renderRollable(float scale, bool useDisplayList);
    virtual void compile(float scale);

    void _init();
    void construct(Model* model);
    void construct(Model* model, const std::wstring& name);
    void construct(Model* model, int xTexOffs, int yTexOffs);
    void construct(Model* model, int xTexOffs, int yTexOffs, int texWidth, int texHeight);

    void PrintCtorValues(const std::string& tag);

    ModelPart* setTexSize(int texWidth, int texHeight);
    ModelPart* texOffs(int xTexOffs, int yTexOffs);
    int getfU();
    int getfV();

    void setPos(float x, float y, float z);
    void setWorldPos(float x, float y, float z);
    void mirror();

    void setYOffset(float offset);
    bool hasYBeenOffset();
    void clearAllOffsets();

    void addChild(ModelPart* child);
    void addChildAndAdjustPivots(ModelPart* child);
    ModelPart* retrieveChild(SKIN_BOX* skinBox);
    ModelPart* retrieveChild(unsigned int skinId, SKIN_BOX* skinBox, float delta);
    ModelPart* retrieveChildWithSkinIdAndArmourFlags(unsigned int skinId, unsigned int armourFlags);

    ModelPart* addBox(float x, float y, float z, int width, int height, int depth);
    ModelPart* addBox(float x, float y, float z, int width, int height, int depth, bool mirror);
    void addBox(float x, float y, float z, int width, int height, int depth, float delta);
    ModelPart* addBox(std::wstring id, float x, float y, float z, int width, int height, int depth);
    ModelPart* addBoxWithMask(float x, float y, float z, int width, int height, int depth, int faceMask);
    void addBoxMapTex(float x, float y, float z, int width, int height, int depth, int xTexOffs, int yTexOffs,
                      float delta);
    void addBoxMapTex(float x, float y, float z, int width, int height, int depth, int xTexOffs, int yTexOffs,
                      bool mirror, float delta);
    void addHumanoidBox(float x, float y, float z, int width, int height, int depth, float delta);

    void translateTo(float scale);

    float mTexWidth;
    float mTexHeight;
    float mX;
    float mY;
    float mZ;
    float mWorldX;
    float mWorldY;
    float mWorldZ;
    float mXRot;
    float mYRot;
    float mZRot;
    float mCubeXRot;
    float mCubeYRot;
    float mCubeZRot;
    bool mMirror;
    bool mShow;
    bool mHidden;
    std::vector<Cube*> mCubes;
    std::vector<ModelPart*> mChildren;
    float mXOffset;
    float mYOffset;
    float mZOffset;
    unsigned int mArmourFlags;
    unsigned int mSkinId;
    float mXRotOffset;
    float mYRotOffset;
    float mZRotOffset;
    unsigned int mSkinBoxType;
    std::wstring mName;
    int mXTexOffs;
    int mYTexOffs;
    bool mCompiled;
    int mDisplayList;
    Model* mModel;
    float mOriginalY;
    bool mYBeenOffset;
};

#endif
