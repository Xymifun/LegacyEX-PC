#include "game/ui/FJ_SocialPost.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_SocialPost::FJ_SocialPost(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_SocialPost::Create(fuiRenderNode* renderNode)
{
    return new FJ_SocialPost(renderNode);
}

unsigned int FJ_SocialPost::getType()
{
    return 75;
}
