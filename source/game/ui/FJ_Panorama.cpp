#include "game/ui/FJ_Panorama.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_Panorama::FJ_Panorama(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    ShowPanoramaDay(true);
}

FJ_Panorama* FJ_Panorama::Create(fuiRenderNode* renderNode)
{
    return new FJ_Panorama(renderNode);
}

void FJ_Panorama::ShowPanoramaDay(bool day)
{
    if (getChildByName(std::string("Panorama_Day")) != NULL)
    {
        getChildByName(std::string("Panorama_Day"))->m_pRenderNode->setVisibility(day);
        getChildByName(std::string("Panorama_Night"))->m_pRenderNode->setVisibility(!day);
    }
}

int FJ_Panorama::GetCurrentFrame()
{
    FJ_FuiNode* dayNode = getChildByName(std::string("Panorama_Day"));
    if (dayNode != NULL && dayNode->getChildAt(0) != NULL)
        return dayNode->getChildAt(0)->currentFrame();
    return 0;
}

void FJ_Panorama::SetCurrentFrame(int frame)
{
    FJ_FuiNode* dayNode = getChildByName(std::string("Panorama_Day"));
    FJ_FuiNode* nightNode = getChildByName(std::string("Panorama_Night"));
    if (dayNode != NULL && dayNode->getChildAt(0) != NULL)
        dayNode->getChildAt(0)->gotoAndPlay(frame);
    else if (nightNode != NULL && nightNode->getChildAt(0) != NULL)
        nightNode->getChildAt(0)->gotoAndPlay(frame);
}

unsigned int FJ_Panorama::getType()
{
    return 4;
}
