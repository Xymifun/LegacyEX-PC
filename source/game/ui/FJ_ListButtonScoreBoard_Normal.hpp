#ifndef GAME_UI_FJ_LISTBUTTONSCOREBOARD_NORMAL_HPP
#define GAME_UI_FJ_LISTBUTTONSCOREBOARD_NORMAL_HPP

#include <stdint.h>

#include "game/ui/FJ_ListButtonScoreBoard.hpp"

class fuiRenderNode;

class FJ_ListButtonScoreBoard_Normal : public FJ_ListButtonScoreBoard
{
public:
    explicit FJ_ListButtonScoreBoard_Normal(fuiRenderNode* renderNode);
    static FJ_ListButtonScoreBoard* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
