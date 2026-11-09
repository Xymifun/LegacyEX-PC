#include "game/ui/FJ_SaveMessage.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/util/Math.hpp"

FJ_SaveMessage::FJ_SaveMessage(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_SaveMessage::Create(fuiRenderNode* renderNode)
{
    return new FJ_SaveMessage(renderNode);
}

void FJ_SaveMessage::AutoResize()
{
    FJ_FuiNode* description = getChildByName(std::string("Description"))->asMovieClip();
    FJ_FuiNode* confirm = getChildByName(std::string("Confirm"))->asMovieClip();
    FJ_FuiNode* background = getChildByName(std::string("BackgroundPanel"))->asMovieClip();

    int roundedOffset = Math::round((description->getY() + description->getHeight()) - confirm->getY()
                                    + confirm->getHeight() * 0.5f);
    confirm->setY(confirm->getY() + roundedOffset);

    FJ_FuiNode* graphic = background->getChildAt(0)->asMovieClip();
    graphic->setHeight(graphic->getHeight() + roundedOffset);

    for (int i = 0; i < m_root->getNumChildren(); ++i)
    {
        FJ_FuiNode* child = getChildAt(i);
        child->setY(child->getY() - static_cast<float>(Math::round(roundedOffset * 0.5f)));
    }
}

unsigned int FJ_SaveMessage::getType()
{
    return 3;
}
