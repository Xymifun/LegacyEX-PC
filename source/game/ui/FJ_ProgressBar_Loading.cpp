#include "game/ui/FJ_ProgressBar_Loading.hpp"

FJ_ProgressBar_Loading::FJ_ProgressBar_Loading(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    GetBarFiller();
    initFiller();
}

FJ_ProgressBar* FJ_ProgressBar_Loading::Create(fuiRenderNode* renderNode)
{
    return new FJ_ProgressBar_Loading(renderNode);
}

unsigned int FJ_ProgressBar_Loading::getType()
{
    return 0x8055;
}
