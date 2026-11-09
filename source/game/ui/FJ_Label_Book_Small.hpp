#ifndef GAME_UI_FJ_LABEL_BOOK_SMALL_HPP
#define GAME_UI_FJ_LABEL_BOOK_SMALL_HPP

#include "game/ui/FJ_Label_Book.hpp"

class fuiRenderNode;

class FJ_Label_Book_Small : public FJ_Label_Book
{
public:
    FJ_Label_Book_Small(fuiRenderNode* pRenderNode) : FJ_Label_Book(pRenderNode) {}
    static FJ_Label* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
