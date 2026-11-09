#ifndef GAME_UI_FUIRENDERNODESHAPE_HPP
#define GAME_UI_FUIRENDERNODESHAPE_HPP

#include "game/ui/fuiRenderNode.hpp"

struct fuiFile;

class fuiRenderNodeShape : public fuiRenderNode
{
public:
    fuiRenderNodeShape(fuiRenderNode* stage, fuiRenderNode* parent, fuiObject* object, unsigned int symbolIndex,
                       unsigned char depth, fuiRGBA* color, unsigned char clipDepth, bool visible, fuiFile* file);
    virtual ~fuiRenderNodeShape();
    virtual void gather(bool recurse);
    virtual void preRender();
    virtual void render(fuiRect* bounds, fuiMatrix* matrix);
};

#endif
