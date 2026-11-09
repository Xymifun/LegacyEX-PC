#ifndef GAME_UI_FJ_FULLSCREENPROGRESS_HPP
#define GAME_UI_FJ_FULLSCREENPROGRESS_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_FullScreenProgress : public FJ_Document
{
public:
    FJ_FullScreenProgress(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void ShowConfirmationButton(bool show);
    void SetLoadingScreenType(int type);
};

#endif
