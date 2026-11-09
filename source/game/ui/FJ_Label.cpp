#include "game/ui/FJ_Label.hpp"

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"

FJ_Label::FJ_Label(fuiRenderNode* renderNode)
    : FJ_Base(renderNode),
      m_textAlignment(eTextAlignment_Left),
      m_multiline(false),
      m_wordWrap(false),
      m_useEllipsis(false),
      m_manualWordWrap(false)
{
}

FJ_Base* FJ_Label::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label(renderNode);
}

void FJ_Label::SetTextAlignment(ETextAlignment alignment)
{
    m_textAlignment = alignment;
}

void FJ_Label::setUseHtmlText(bool useHtmlText)
{
    m_htmlText = useHtmlText;
}

unsigned int FJ_Label::getType()
{
    return 0x8059;
}

void FJ_Label::Init(const std::wstring& label)
{
    FJ_Base::SetLabelAlignment(static_cast<int>(m_textAlignment));
    FJ_Base::InitBase(label, -1);
}

void FJ_Label::ResetLabel()
{
    m_objectWidth = -1.0f;
    m_objectHeight = -1.0f;
    m_initialised = false;
    m_newWidth = static_cast<int>(getWidth());
}

void FJ_Label::SetLabel(const std::wstring& label)
{
    FJ_Base::SetLabelAlignment(static_cast<int>(m_textAlignment));
    m_manualWrap = m_manualWordWrap;
    m_textContainerVisible = m_wordWrap;
    m_textOnly = m_useEllipsis;
    m_textContainerChanged = m_multiline;
    m_initialised = true;
    FJ_Base::SetLabel(label);
}

int FJ_Label::GetAverageCharHeight()
{
    int count = 0;
    int total = 0;
    fuiRenderNodeEditText* textField = GetTextField();
    for (unsigned int index = 0; index < textField->getText().size(); ++index)
    {
        total += static_cast<int>(textField->getCharBoundaries(index).yMax);
        ++count;
        if (count >= 5)
        {
            break;
        }
    }
    if (count == 0)
    {
        total = 0;
    }
    else
    {
        total /= count;
    }
    return total;
}

void FJ_Label::GetLabelTooWide(fuiRenderNodeEditText* textField)
{
    float limit = static_cast<float>(m_maxLabelWidth);
    float required = static_cast<float>(
        static_cast<int>(limit <= textField->m_textWidth ? textField->m_textWidth : limit));
    int width = getWidth() < required ? static_cast<int>(required) : static_cast<int>(getWidth());
    int newWidth = m_newWidth;
    m_newWidth = newWidth < width ? width : newWidth;
}

void FJ_Label::setAction(unsigned char action, char* value)
{
    switch (action)
    {
    case 5:
        m_textAlignment = static_cast<ETextAlignment>(std::atoi(value));
        break;
    case 6:
        m_multiline = std::strcmp(value, "true") == 0;
        break;
    case 7:
        m_useEllipsis = std::strcmp(value, "true") == 0;
        break;
    case 8:
        m_wordWrap = std::strcmp(value, "true") == 0;
        break;
    case 9:
        m_manualWordWrap = std::strcmp(value, "true") == 0;
        break;
    }
}

typedef std::vector<FJ_Label*> FJ_LabelVector;
typedef FJ_LabelVector::iterator FJ_LabelIterator;

void FJ_Label_InsertRepeated(FJ_LabelVector& vector, FJ_LabelIterator position, FJ_LabelVector::size_type count,
                             FJ_Label* const& value)
{
    vector.insert(position, count, value);
}

FJ_LabelIterator FJ_Label_Insert(FJ_LabelVector& vector, FJ_LabelIterator position, FJ_Label* const& value)
{
    return vector.insert(position, value);
}

FJ_LabelIterator FJ_Label_Erase(FJ_LabelVector& vector, FJ_LabelIterator position)
{
    return vector.erase(position);
}
