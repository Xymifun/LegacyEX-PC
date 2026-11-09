#ifndef GAME_UI_FJ_LISTBUTTONSCOREBOARD_SMALL_HPP
#define GAME_UI_FJ_LISTBUTTONSCOREBOARD_SMALL_HPP

#include <stdint.h>

#include "game/ui/FJ_ListButtonScoreBoard.hpp"

class fuiRenderNode;

class FJ_ListButtonScoreBoard_Small : public FJ_ListButtonScoreBoard
{
public:
    explicit FJ_ListButtonScoreBoard_Small(fuiRenderNode* renderNode);

    static FJ_ListButtonScoreBoard* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
