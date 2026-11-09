#ifndef GAME_CLIENT_RENDER_MODELPARTDEFINITION_HPP
#define GAME_CLIENT_RENDER_MODELPARTDEFINITION_HPP

class DataInputStream;
class ModelPart;
class ModelPartCubeDefinition;

class ModelPartDefinition
{
public:
    ModelPartDefinition();
    ~ModelPartDefinition();
    void SetPartPosition(ModelPart* part, ModelPart* parent) const;
    void SetPartRotation(ModelPart* part) const;
    void AddCubes(ModelPart* part, float scale) const;
    void SetupModelPart(ModelPart* part, ModelPart* parent, float scale) const;
    void readModelPart(DataInputStream* input, int hasCubeRotation);

private:
    float mX;
    float mY;
    float mZ;
    float mXRot;
    float mYRot;
    float mZRot;
    float mCubeXRot;
    float mCubeYRot;
    float mCubeZRot;
    int mCubeCount;
    ModelPartCubeDefinition* mCubes;
};

#endif
