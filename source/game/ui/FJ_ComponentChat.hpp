#ifndef GAME_UI_FJ_COMPONENTCHAT_HPP
#define GAME_UI_FJ_COMPONENTCHAT_HPP

#include <string>

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_ComponentChat : public FJ_FuiNode
{
public:
    FJ_ComponentChat(fuiRenderNode* pRenderNode);

    static FJ_FuiNode* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    void SetAlpha(const std::string& name, float alpha);
};

#endif
