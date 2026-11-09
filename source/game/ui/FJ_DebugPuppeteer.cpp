#include "game/ui/FJ_DebugPuppeteer.hpp"

FJ_DebugPuppeteer::FJ_DebugPuppeteer(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_DebugPuppeteer* FJ_DebugPuppeteer::Create(fuiRenderNode* renderNode)
{
    return new FJ_DebugPuppeteer(renderNode);
}

unsigned int FJ_DebugPuppeteer::getType()
{
    return 0x39;
}
