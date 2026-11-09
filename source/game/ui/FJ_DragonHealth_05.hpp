#ifndef GAME_UI_FJ_DRAGONHEALTH_05_HPP
#define GAME_UI_FJ_DRAGONHEALTH_05_HPP

#include "game/ui/FJ_DragonHealth.hpp"

class fuiRenderNode;

class FJ_DragonHealth_05 : public FJ_DragonHealth
{
public:
    explicit FJ_DragonHealth_05(fuiRenderNode* renderNode);
    static FJ_DragonHealth* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
