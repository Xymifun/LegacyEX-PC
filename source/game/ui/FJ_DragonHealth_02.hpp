#ifndef GAME_UI_FJ_DRAGONHEALTH_02_HPP
#define GAME_UI_FJ_DRAGONHEALTH_02_HPP

#include "game/ui/FJ_DragonHealth.hpp"

class fuiRenderNode;

class FJ_DragonHealth_02 : public FJ_DragonHealth
{
public:
    explicit FJ_DragonHealth_02(fuiRenderNode* renderNode);
    static FJ_DragonHealth* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
