#include "game/client/render/ModelPartCubeDefinition.hpp"

#include "game/core/DataInputStream.hpp"

ModelPartCubeDefinition::ModelPartCubeDefinition()
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    width = 0;
    height = 0;
    depth = 0;
    texU = 0.0f;
    texV = 0.0f;
    inflate = 0.0f;
    mirror = false;
}

void ModelPartCubeDefinition::readModelPartCube(DataInputStream* input)
{
    x = input->readFloat();
    y = input->readFloat();
    z = input->readFloat();
    width = input->readInt();
    height = input->readInt();
    depth = input->readInt();
    texU = input->readFloat();
    texV = input->readFloat();
    inflate = input->readFloat();
    mirror = input->readBoolean();
}
