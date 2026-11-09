#ifndef GAME_UI_FJ_PROGRESSBAR_LOADING_HPP
#define GAME_UI_FJ_PROGRESSBAR_LOADING_HPP

#include "game/ui/FJ_ProgressBar.hpp"

class FJ_ProgressBar_Loading : public FJ_ProgressBar
{
public:
    FJ_ProgressBar_Loading(fuiRenderNode* renderNode);
    static FJ_ProgressBar* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
