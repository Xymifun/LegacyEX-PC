#ifndef GAME_UI_FJ_LABEL_BOOK_NORMAL_HPP
#define GAME_UI_FJ_LABEL_BOOK_NORMAL_HPP

#include "game/ui/FJ_Label_Book.hpp"

class fuiRenderNode;

class FJ_Label_Book_Normal : public FJ_Label_Book
{
public:
    explicit FJ_Label_Book_Normal(fuiRenderNode* renderNode);
    static FJ_Label_Book_Normal* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
