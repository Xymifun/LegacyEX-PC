#include "game/ui/FJ_HtmlText_Normal.hpp"

#include <string>

#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"

FJ_HtmlText* FJ_HtmlText_Normal::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_HtmlText_Normal(pRenderNode);
}

void FJ_HtmlText_Normal::ASConstructor()
{
    FJ_FuiNode* textContainer = getChildByName(std::string("FJ_TextContainer"));
    FJ_FuiNode* textFieldContainer = textContainer->getChildAt(0);
    m_pText = textFieldContainer->getRenderNodeChildByName(std::string("FJ_TextField"))->asEditText();
    addChild(m_pText);
    removeChild(textContainer, true);
}

unsigned int FJ_HtmlText_Normal::getType()
{
    return 0x80a2;
}
