#ifndef GAME_CORE_FACEINFO_HPP
#define GAME_CORE_FACEINFO_HPP

class Direction;

class FaceInfo
{
public:
    class VertexInfo
    {
    public:
        VertexInfo(int xFace, int yFace, int zFace);

        int xFace;
        int yFace;
        int zFace;
    };

    static FaceInfo* DOWN;
    static FaceInfo* UP;
    static FaceInfo* NORTH;
    static FaceInfo* SOUTH;
    static FaceInfo* WEST;
    static FaceInfo* EAST;

    FaceInfo(VertexInfo* vertex0, VertexInfo* vertex1, VertexInfo* vertex2, VertexInfo* vertex3);

    static void staticCtor();

    static FaceInfo* fromFacing(const Direction* facing);

    VertexInfo* getVertexInfo(int index) const;

private:
    static int SOUTH_FACE;
    static int UP_FACE;
    static int EAST_FACE;
    static int NORTH_FACE;
    static int DOWN_FACE;
    static int WEST_FACE;

    static FaceInfo* BY_FACING[6];

    VertexInfo* vertexInfos[4];
};

#endif
