#include "game/ui/FJ_Base.hpp"
#include <cmath>
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

FJ_Base::FJ_Base(fuiRenderNode* renderNode) : FJ_FuiNode(renderNode)
{
    m_isLabel = true;
    m_textContainerChanged = false;
    m_textContainerVisible = false;
    m_manualWrap = false;
    m_centerBiasX = 0.0f;
    m_objectWidth = -1.0f;
    m_id = -1;
    m_htmlText = false;
    m_initialised = false;
    m_labelAlignment = 0;
    m_resizing = false;
    m_textChanged = false;
    m_labelTooWide = false;
    m_labelOverflowResolved = false;
    m_iTabIndex = -1;
    m_objectHeight = -1.0f;
    m_newWidth = static_cast<int>(getWidth());
    m_wrapWidth = 0;
    m_preScaleWidth = static_cast<int>(getWidth());
    m_textOnly = false;
    m_multilineWrap = false;
    m_lineCount = 0;
    m_manualWrapFits = false;
    m_multilineWrapFits = false;
    m_ellipsisWidthOverride = 0;
    m_maxLabelWidth = -1;
    m_objListParent = 0;
    m_autoGrowHeight = false;
    m_listId = 0;
    CorrectPixelPosition();
}

FJ_Base::~FJ_Base()
{
}

void FJ_Base::CheckLabelWidths(std::vector<std::wstring>& labels)
{
    for (std::vector<std::wstring>::size_type i = 0; i < labels.size(); ++i)
    {
        CheckLabelWidth(labels[i]);
    }
}

FJ_Base* FJ_Base::getObjListParent()
{
    return static_cast<FJ_Base*>(m_objListParent);
}

uint32_t FJ_Base::getId()
{
    return static_cast<uint32_t>(m_id);
}

void FJ_Base::LostFocus()
{
    if (m_objListParent != 0)
    {
        if (m_objListParent->isFJ_List())
        {
            static_cast<FJ_List*>(m_objListParent)->LoseListFocus();
            return;
        }
        m_objListParent->isFJ_List2D();
    }
}

uint32_t FJ_Base::getNewWidth()
{
    return static_cast<uint32_t>(m_newWidth);
}
void FJ_Base::ResizingComplete()
{
    m_resizing = true;
}
void FJ_Base::ResetResizing()
{
    m_resizing = false;
}
void FJ_Base::setObjListParent(FJ_FuiNode* parent)
{
    m_objListParent = parent;
}
uint32_t FJ_Base::getListId()
{
    return static_cast<uint32_t>(m_listId);
}
void FJ_Base::setListId(int id)
{
    m_listId = id;
}
void FJ_Base::setPreScaleWidth(int width)
{
    m_preScaleWidth = width;
}
void FJ_Base::setId(int id)
{
    m_id = id;
}
uint8_t FJ_Base::getLabelTooWide()
{
    return m_labelTooWide ? 1 : 0;
}
int FJ_Base::getOldWidth()
{
    return static_cast<int>(getWidth());
}

void FJ_Base::CorrectPixelPosition()
{
    if (std::fmod(getX(), 1.0f) != 0.0f)
    {
        float value = getX();
        float corrected;
        if (getX() < 0.0f)
            corrected = ceilf(value - 0.5f);
        else
            corrected = floorf(value + 0.5f);
        setX(corrected);
    }
    if (std::fmod(getY(), 1.0f) != 0.0f)
    {
        float value = getY();
        float corrected;
        if (getY() < 0.0f)
            corrected = ceilf(value - 0.5f);
        else
            corrected = floorf(value + 0.5f);
        setY(corrected);
    }
}

void FJ_Base::InitBase(const std::wstring& name, int id)
{
    m_initialised = true;
    m_id = id;
    SetLabel(name);
}

fuiRenderNodeEditText* FJ_Base::GetTextField()
{
    fuiRenderNodeEditText* textField = 0;
    FJ_FuiNode* node;
    if (isFJ_Label())
    {
        node = getChildAt(0);
        node = node->asMovieClip();
    }
    else
    {
        node = getChildByName(std::string("FJ_TextContainer"));
        node = (node != 0 && node->getNumChildren() != 0) ? node->getChildAt(0) : 0;
    }
    fuiRenderNode* timelineNode;
    if (node != 0 && (timelineNode = node->m_pRenderNode->asTimeline()) != 0)
        textField = static_cast<fuiRenderNodeTimeline*>(timelineNode)->getChildAt(0)->asEditText();
    return textField;
}

void FJ_Base::CheckLabelWidth(const std::wstring& label)
{
    fuiRenderNodeEditText* field = GetTextField();
    field->setText(label);
    int width = static_cast<int>(field->m_textWidth);
    if (width > m_maxLabelWidth)
        m_maxLabelWidth = width;
    field->setText(std::wstring(L" "));
}

void FJ_Base::SetLabelAlignment(int alignment)
{
    m_labelAlignment = alignment;
}

void FJ_Base::DoManualWordWrapCheck(FJ_FuiNode* container, fuiRenderNodeEditText* field)
{
    if (m_wrapWidth == 0)
        m_wrapWidth = static_cast<int>(getWidth());
    int limit = m_wrapWidth;
    int offset = 0;
    if (container == 0)
        limit = static_cast<int>(static_cast<float>(limit) * getScaleX());
    else
        offset = static_cast<int>(container->getX() * getScaleX()) * 2;
    if (static_cast<float>(offset) + field->m_textWidth <= static_cast<float>(limit))
    {
        m_manualWrapFits = true;
        return;
    }
    std::wstring source = field->getText();
    float total = static_cast<float>(offset) + field->m_textWidth;
    int split = 0;
    if (total <= static_cast<float>(limit * 2))
        m_manualWrapFits = true;
    else
    {
        limit = Math::round(total * 0.5f);
        m_manualWrapFits = false;
    }
    int index = static_cast<int>(source.size()) - 1;
    if (index >= 0)
    {
        unsigned int pairs = static_cast<unsigned int>(source.size()) >> 1;
        bool found = false;
        while (pairs != 0)
        {
            if (source[index] == L' ')
            {
                field->setText(source.substr(0, index));
                if (static_cast<float>(offset) + field->m_textWidth <= static_cast<float>(limit))
                {
                    split = index;
                    found = true;
                    break;
                }
            }
            int second = index - 1;
            if (source[second] == L' ')
            {
                field->setText(source.substr(0, second));
                if (static_cast<float>(offset) + field->m_textWidth <= static_cast<float>(limit))
                {
                    split = second;
                    found = true;
                    break;
                }
            }
            --pairs;
            index -= 2;
        }
        if (!found && (source.size() & 1) != 0 && source[index] == L' ')
        {
            field->setText(source.substr(0, index));
            if (static_cast<float>(offset) + field->m_textWidth <= static_cast<float>(limit))
                split = index;
        }
    }
    field->setText(source.substr(0, split));
    int firstWidth = static_cast<int>(field->m_textWidth);
    field->setText(source.substr(split + 1));
    int secondWidth = static_cast<int>(field->m_textWidth);
    field->setText(std::wstring(L" "));
    int widthDelta = 0;
    if (firstWidth < secondWidth)
        widthDelta = (secondWidth - firstWidth) / 2;
    else if (secondWidth < firstWidth)
        widthDelta = (firstWidth - secondWidth) / 2;
    int padding = Math::round(static_cast<float>(widthDelta)
                              / static_cast<float>(static_cast<int>(field->m_textWidth)));
    std::wstring spaces = std::wstring(L"");
    for (int i = 0; i < padding; ++i)
        spaces.append(L" ");
    if (firstWidth < secondWidth)
        field->setText(spaces + source.substr(0, split) + L"\n" + source.substr(split + 1));
    else if (secondWidth < firstWidth)
        field->setText(source.substr(0, split) + L"\n" + spaces + source.substr(split + 1));
    else
        field->setText(source.substr(0, split) + L"\n" + source.substr(split + 1));
}

void FJ_Base::DoMultilineWordWrap(FJ_FuiNode* container, fuiRenderNodeEditText* field)
{
    if (m_wrapWidth == 0)
        m_wrapWidth = static_cast<int>(getWidth());
    int offset = 0;
    int limit;
    if (container == 0)
        limit = static_cast<int>(static_cast<float>(m_wrapWidth) * getScaleX());
    else
    {
        offset = static_cast<int>(container->getX() * getScaleX());
        limit = m_wrapWidth;
    }
    m_lineCount = 0;
    m_lineWidths.clear();
    if (static_cast<float>(offset) + field->m_textWidth <= static_cast<float>(limit))
        return;
    std::wstring source = field->getText();
    std::wstring accumulated = std::wstring(L"");
    while (static_cast<float>(limit) < static_cast<float>(offset) + field->m_textWidth)
    {
        field->setText(std::wstring(L""));
        int split = 0;
        int latestSpace = 0;
        for (unsigned int index = 0; index < source.size(); ++index)
        {
            field->appendText(source.substr(index, 1));
            if (static_cast<float>(limit) <= static_cast<float>(offset) + field->m_textWidth)
            {
                split = static_cast<int>(index) - 1;
                break;
            }
            if (source[index] == L' ')
                latestSpace = static_cast<int>(index);
        }
        if (latestSpace > 0)
            split = latestSpace;
        ++m_lineCount;
        m_lineWidths.push_back(split);
        accumulated += source.substr(0, split + 1) + L"\n";
        source = source.substr(split + 1);
        field->setText(source);
    }
    field->setText(accumulated + source);
    m_label = field->getText();
}

void FJ_Base::DoEllipsisCheck(FJ_FuiNode* container, fuiRenderNodeEditText* field)
{
    if (m_wrapWidth == 0)
        m_wrapWidth = static_cast<int>(getWidth());
    int offset = 0;
    int suffixAllowance = 5;
    int limitWidth;
    if (container != 0)
    {
        offset = static_cast<int>(container->getX() * getScaleX());
        limitWidth = m_wrapWidth;
    }
    else
        limitWidth = static_cast<int>(static_cast<float>(m_wrapWidth) * getScaleX());
    if (m_ellipsisWidthOverride > 0)
    {
        suffixAllowance = 0;
        limitWidth = m_ellipsisWidthOverride;
    }
    if (static_cast<float>(limitWidth)
        < static_cast<float>(offset) + field->m_textWidth + static_cast<float>(suffixAllowance))
    {
        const float targetWidth = static_cast<float>(limitWidth - offset - suffixAllowance);
        do
        {
            field->setText(field->getText().substr(0, field->getText().size() - 1));
        } while (targetWidth < field->m_textWidth);
        field->setText(field->getText().substr(0, field->getText().size() - 3));
        m_label = field->getText() + L"...";
        field->setText(m_label);
    }
}

void FJ_Base::SetLabel(const std::wstring& label)
{
    m_label = label;
    FJ_FuiNode* container = getChildByName(std::string("FJ_TextContainer"));
    fuiRenderNodeEditText* field = GetTextField();
    if (field == 0)
        return;
    if (!m_initialised)
    {
        field->setText(std::wstring(L"Not Initialised!"));
        return;
    }
    if (m_objectWidth == -1.0f)
    {
        fuiRect rect = getRect(this);
        m_objectWidth = rect.xMin + rect.xMin + getWidth();
    }
    if (m_objectHeight == -1.0f)
    {
        fuiRect rect = getRect(this);
        m_objectHeight = getHeight() + (rect.yMin + rect.yMin);
    }
    field->m_wordWrap = m_textContainerVisible;
    field->m_multiline = m_textContainerChanged;
#ifdef MINECRAFT_PC
    if (!m_htmlText && m_label.find(L"<font") == std::wstring::npos)
#else
    if (!m_htmlText)
#endif
        field->setText(m_label);
    else
        field->setHTMLText(m_label);
    if (container != 0)
    {
        container->setScaleX(1.0f);
        container->setScaleY(1.0f);
    }
    field->setScaleX(1.0f / getScaleX());
    field->setScaleY(1.0f / getScaleY());
    if (!m_manualWrap)
    {
        if (m_multilineWrap)
            DoMultilineWordWrap(container, field);
    }
    else if (!m_multilineWrap)
    {
        int heightBefore = static_cast<int>(field->m_textHeight);
        DoManualWordWrapCheck(container, field);
        int heightAfter = static_cast<int>(field->m_textHeight);
        if (m_autoGrowHeight && heightBefore < heightAfter)
            SetNewObjectHeight(
                static_cast<int>(getHeight() + static_cast<float>(heightAfter) - static_cast<float>(heightBefore)));
        if (!m_manualWrap && m_multilineWrap)
            DoMultilineWordWrap(container, field);
    }
    if (!m_textOnly)
    {
        bool fits = m_manualWrapFits && m_multilineWrapFits;
        if (!m_resizing && !fits)
            GetLabelTooWide(field);
    }
    else
    {
        DoEllipsisCheck(container, field);
    }
    if (m_labelOverflowResolved)
    {
        field->setAutoSize(TextFieldAutoSizeCenter);
        field->setWidth(static_cast<float>(m_ellipsisWidth));
        std::wstring text = L"[...] " + field->getText();
        field->setText(text);
    }
    else if (m_isLabel)
    {
        field->setAutoSize(TextFieldAutoSizeLeft);
    }
    if (m_textContainerVisible)
        field->setWidth(m_objectWidth);
    if (m_labelAlignment == 2)
    {
        if (container != 0)
        {
            container->setX(0.0f);
            container->setY(0.0f);
        }
        if (field->m_alignment != 2)
        {
            float x = Math::round((m_objectWidth / getScaleX() - field->getWidth()) * 0.5f + m_centerBiasX);
            field->setX(x);
            float y = Math::round((m_objectHeight / getScaleY() - field->getHeight()) * 0.5f);
            field->setY(y);
        }
    }
    else if (m_labelAlignment == 3)
    {
        if (container != 0)
        {
            container->setX(0.0f);
            container->setY(0.0f);
        }
        float x = Math::round(m_objectWidth / getScaleX() - field->getWidth());
        field->setX(x);
        float y = Math::round((m_objectHeight / getScaleY() - field->getHeight()) * 0.5f);
        field->setY(y);
    }
    else if (m_labelAlignment == 1)
    {
        if (container != 0)
        {
            container->setX(0.0f);
            container->setY(0.0f);
        }
        field->setX(0.0f);
        float y = Math::round((m_objectHeight / getScaleY() - field->getHeight()) * 0.5f);
        field->setY(y);
    }
    else if (m_labelAlignment == 4)
    {
        if (container != 0)
            container->setY(0.0f);
        field->setX(Math::round(field->getX()));
        float y = Math::round((m_objectHeight / getScaleY() - field->getHeight()) * 0.5f);
        field->setY(y);
    }
    else if (m_isLabel)
    {
        field->setX(0.0f);
        field->setY(0.0f);
    }
}

const std::wstring& FJ_Base::GetLabel()
{
    return m_label;
}

void FJ_Base::SetTextOnly(const std::wstring& text)
{
    fuiRenderNodeEditText* field = GetTextField();
    if (text == L"")
        field->setText(std::wstring(L" "));
    else
        field->setText(text);
}

void FJ_Base::GetLabelTooWide(fuiRenderNodeEditText* field)
{
    float label = static_cast<float>(m_maxLabelWidth);
    float required = static_cast<float>(static_cast<int>(label <= field->m_textWidth ? field->m_textWidth : label));
    int width = required <= getWidth() ? static_cast<int>(getWidth()) : static_cast<int>(required);
    bool resolved = m_labelOverflowResolved;
    int newWidth = m_newWidth;
    if (width > newWidth)
        newWidth = width;
    m_newWidth = newWidth;
    if (!resolved && getWidth() < required)
    {
        m_ellipsisWidth = static_cast<int>(m_objectWidth * 0.8);
        m_labelTooWide = true;
        m_labelOverflowResolved = true;
    }
}

void FJ_Base::SetNewObjectWidth(int width)
{
    if (static_cast<int>(getWidth()) < width)
    {
        SetTextOnly(std::wstring(L" "));
        setWidth(static_cast<float>(width));
        m_preScaleWidth = width;
        fuiRect rect = getRect(this);
        m_objectWidth = getWidth() + (rect.xMin + rect.xMin);
        m_labelTooWide = false;
        m_labelOverflowResolved = false;
        SetLabel(m_label);
    }
}

void FJ_Base::SetNewObjectHeight(int height)
{
    if (static_cast<int>(getHeight()) < height && !m_textChanged)
    {
        m_textChanged = true;
        SetTextOnly(std::wstring(L" "));
        setHeight(static_cast<float>(height));
        fuiRect rect = getRect(this);
        m_objectHeight = getHeight() + (rect.yMin + rect.yMin);
        SetLabel(m_label);
    }
}

void FJ_Base::HandleCharInput(unsigned int)
{
}
uint32_t FJ_Base::getType()
{
    return 0x8000;
}
