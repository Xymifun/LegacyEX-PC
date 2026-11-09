#ifndef GAME_UI_FUIRENDERNODEBITMAP_HPP
#define GAME_UI_FUIRENDERNODEBITMAP_HPP

#include "game/ui/fuiRenderNode.hpp"

struct fuiFile;

class fuiRenderNodeBitmap : public fuiRenderNode
{
public:
    fuiRenderNodeBitmap(fuiRenderNode* parent, fuiRenderNode* root, fuiObject* object, unsigned int symbolIndex,
                        unsigned char depth, fuiRGBA* color, unsigned char clipDepth, bool visible, fuiFile* file);
    virtual ~fuiRenderNodeBitmap();
    virtual void gather(bool recurse);
    virtual void preRender();
    virtual void render(fuiRect* bounds, fuiMatrix* matrix);
};

#endif
