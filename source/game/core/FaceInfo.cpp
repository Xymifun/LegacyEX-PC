#include "game/core/FaceInfo.hpp"

#include "game/core/Direction.hpp"

FaceInfo* FaceInfo::DOWN = new FaceInfo(new FaceInfo::VertexInfo(WEST_FACE, DOWN_FACE, SOUTH_FACE),
                                        new FaceInfo::VertexInfo(WEST_FACE, DOWN_FACE, NORTH_FACE),
                                        new FaceInfo::VertexInfo(EAST_FACE, DOWN_FACE, NORTH_FACE),
                                        new FaceInfo::VertexInfo(EAST_FACE, DOWN_FACE, SOUTH_FACE));

FaceInfo* FaceInfo::UP = new FaceInfo(
    new FaceInfo::VertexInfo(WEST_FACE, UP_FACE, NORTH_FACE), new FaceInfo::VertexInfo(WEST_FACE, UP_FACE, SOUTH_FACE),
    new FaceInfo::VertexInfo(EAST_FACE, UP_FACE, SOUTH_FACE), new FaceInfo::VertexInfo(EAST_FACE, UP_FACE, NORTH_FACE));

FaceInfo* FaceInfo::NORTH = new FaceInfo(new FaceInfo::VertexInfo(EAST_FACE, UP_FACE, NORTH_FACE),
                                         new FaceInfo::VertexInfo(EAST_FACE, DOWN_FACE, NORTH_FACE),
                                         new FaceInfo::VertexInfo(WEST_FACE, DOWN_FACE, NORTH_FACE),
                                         new FaceInfo::VertexInfo(WEST_FACE, UP_FACE, NORTH_FACE));

FaceInfo* FaceInfo::SOUTH = new FaceInfo(new FaceInfo::VertexInfo(WEST_FACE, UP_FACE, SOUTH_FACE),
                                         new FaceInfo::VertexInfo(WEST_FACE, DOWN_FACE, SOUTH_FACE),
                                         new FaceInfo::VertexInfo(EAST_FACE, DOWN_FACE, SOUTH_FACE),
                                         new FaceInfo::VertexInfo(EAST_FACE, UP_FACE, SOUTH_FACE));

FaceInfo* FaceInfo::WEST = new FaceInfo(new FaceInfo::VertexInfo(WEST_FACE, UP_FACE, NORTH_FACE),
                                        new FaceInfo::VertexInfo(WEST_FACE, DOWN_FACE, NORTH_FACE),
                                        new FaceInfo::VertexInfo(WEST_FACE, DOWN_FACE, SOUTH_FACE),
                                        new FaceInfo::VertexInfo(WEST_FACE, UP_FACE, SOUTH_FACE));

FaceInfo* FaceInfo::EAST = new FaceInfo(new FaceInfo::VertexInfo(EAST_FACE, UP_FACE, SOUTH_FACE),
                                        new FaceInfo::VertexInfo(EAST_FACE, DOWN_FACE, SOUTH_FACE),
                                        new FaceInfo::VertexInfo(EAST_FACE, DOWN_FACE, NORTH_FACE),
                                        new FaceInfo::VertexInfo(EAST_FACE, UP_FACE, NORTH_FACE));

int FaceInfo::SOUTH_FACE;
int FaceInfo::UP_FACE;
int FaceInfo::EAST_FACE;
int FaceInfo::NORTH_FACE;
int FaceInfo::DOWN_FACE;
int FaceInfo::WEST_FACE;

FaceInfo* FaceInfo::BY_FACING[6];

FaceInfo::VertexInfo::VertexInfo(int xFace, int yFace, int zFace)
{
    this->xFace = xFace;
    this->yFace = yFace;
    this->zFace = zFace;
}

FaceInfo::FaceInfo(VertexInfo* vertex0, VertexInfo* vertex1, VertexInfo* vertex2, VertexInfo* vertex3)
{
    vertexInfos[0] = vertex0;
    vertexInfos[1] = vertex1;
    vertexInfos[2] = vertex2;
    vertexInfos[3] = vertex3;
}

void FaceInfo::staticCtor()
{
    SOUTH_FACE = Direction::SOUTH->get3DDataValue();
    UP_FACE = Direction::UP->get3DDataValue();
    EAST_FACE = Direction::EAST->get3DDataValue();
    NORTH_FACE = Direction::NORTH->get3DDataValue();
    DOWN_FACE = Direction::DOWN->get3DDataValue();
    WEST_FACE = Direction::WEST->get3DDataValue();

    BY_FACING[DOWN_FACE] = DOWN;
    BY_FACING[UP_FACE] = UP;
    BY_FACING[NORTH_FACE] = NORTH;
    BY_FACING[SOUTH_FACE] = SOUTH;
    BY_FACING[WEST_FACE] = WEST;
    BY_FACING[EAST_FACE] = EAST;
}

FaceInfo* FaceInfo::fromFacing(const Direction* facing)
{
    return BY_FACING[facing->get3DDataValue()];
}

FaceInfo::VertexInfo* FaceInfo::getVertexInfo(int index) const
{
    return vertexInfos[index];
}
