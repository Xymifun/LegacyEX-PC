#ifndef GAME_CLIENT_RENDER_MODELPARTCUBEDEFINITION_HPP
#define GAME_CLIENT_RENDER_MODELPARTCUBEDEFINITION_HPP

class DataInputStream;

class ModelPartCubeDefinition
{
public:
    float x;
    float y;
    float z;
    int width;
    int height;
    int depth;
    float texU;
    float texV;
    float inflate;
    bool mirror;

    ModelPartCubeDefinition();

    void readModelPartCube(DataInputStream* input);
};

#endif
