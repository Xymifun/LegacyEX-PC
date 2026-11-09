#include "game/ui/FJ_DebugMarketingGuide.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

namespace
{
std::string platformLookup[4] = {};
}

FJ_DebugMarketingGuide::FJ_DebugMarketingGuide(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    for (int i = 0; i < getNumChildren(); ++i)
    {
        FJ_FuiNode* child = getChildAt(i);
        child->setVisible(false);
    }
}

FJ_Document* FJ_DebugMarketingGuide::Create(fuiRenderNode* renderNode)
{
    return new FJ_DebugMarketingGuide(renderNode);
}

unsigned int FJ_DebugMarketingGuide::getType()
{
    return 54;
}

void FJ_DebugMarketingGuide::SetPlatform(int platform)
{
    if (getChildByName(platformLookup[platform]) != NULL)
        getChildByName(platformLookup[platform])->setVisible(true);
}
