#ifndef GAME_UI_FUIRENDERNODESTAGE_HPP
#define GAME_UI_FUIRENDERNODESTAGE_HPP

#include "game/ui/fuiRenderNode.hpp"

class fuiRenderNodeStage : public fuiRenderNode
{
private:
    fuiRenderNode* m_child;

public:
    fuiRenderNodeStage(fuiFile* file);
    virtual ~fuiRenderNodeStage();
    void addChild(fuiRenderNode* child);
    fuiRenderNode* getChild();
    virtual void gather(bool force);
    virtual void preRender();
    virtual void render(fuiRect* rect, fuiMatrix* matrix);
    virtual fuiRenderNode* findNode(const char* name);
    float getStageWidth();
    float getStageHeight();
};

#endif
