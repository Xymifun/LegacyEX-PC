#ifndef GAME_UI_FJ_DRAGONHEALTH_01_HPP
#define GAME_UI_FJ_DRAGONHEALTH_01_HPP

#include "game/ui/FJ_DragonHealth.hpp"

class fuiRenderNode;

class FJ_DragonHealth_01 : public FJ_DragonHealth
{
public:
    explicit FJ_DragonHealth_01(fuiRenderNode* renderNode);
    static FJ_DragonHealth* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
