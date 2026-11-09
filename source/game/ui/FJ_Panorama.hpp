#ifndef GAME_UI_FJ_PANORAMA_HPP
#define GAME_UI_FJ_PANORAMA_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_Panorama : public FJ_Document
{
public:
    FJ_Panorama(fuiRenderNode* renderNode);
    static FJ_Panorama* Create(fuiRenderNode* renderNode);
    void ShowPanoramaDay(bool day);
    int GetCurrentFrame();
    void SetCurrentFrame(int frame);
    unsigned int getType();
};

#endif
