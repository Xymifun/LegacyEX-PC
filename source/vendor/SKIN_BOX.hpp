#ifndef VENDOR_SKIN_BOX_HPP
#define VENDOR_SKIN_BOX_HPP

struct SKIN_BOX
{
    int id;
    float x;
    float y;
    float z;
    float width;
    float height;
    float depth;
    float xTexOffs;
    float yTexOffs;
    float delta;
    unsigned int armourFlags;
    unsigned int mirror;
};

#endif
