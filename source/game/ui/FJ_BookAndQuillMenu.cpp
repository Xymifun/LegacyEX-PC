#include "game/ui/FJ_BookAndQuillMenu.hpp"

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"

static std::wstring s_caretLineEnd(L"_");
static std::wstring s_caret(L"|");

FJ_BookAndQuillMenu::FJ_BookAndQuillMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_BookAndQuillMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_BookAndQuillMenu(renderNode);
}

unsigned int FJ_BookAndQuillMenu::getType()
{
    return 21;
}

void FJ_BookAndQuillMenu::SetBookFocus(bool focus)
{
    if (!m_bookIsSigned)
        m_caret->setVisible(focus);
}

void FJ_BookAndQuillMenu::ChangePage(int page, int pageCount)
{
    FJ_FuiNode* left = getChildByName("BookArrowLeft");
    FJ_FuiNode* right = getChildByName("BookArrowRight");
    if (page == 0 && pageCount == 0)
    {
        right->gotoAndStop("ArrowNone");
        left->gotoAndStop("ArrowNone");
        return;
    }

    if (page > m_currentPage)
    {
        if (page < pageCount)
            right->gotoAndPlay("ArrowAnim");
        else
            right->gotoAndPlay("ArrowAnimEnd");
        left->gotoAndStop("ArrowNormal");
    }
    else if (page < m_currentPage)
    {
        if (page > 0)
            left->gotoAndPlay("ArrowAnim");
        else
            left->gotoAndPlay("ArrowAnimEnd");
        right->gotoAndStop("ArrowNormal");
    }
    else
    {
        if (page == 0)
            left->gotoAndStop("ArrowNone");
        else
            left->gotoAndStop("ArrowNormal");
        if (pageCount > 0 && page < pageCount)
            right->gotoAndStop("ArrowNormal");
        else
            right->gotoAndStop("ArrowNone");
    }
    m_currentPage = page;
}

int FJ_BookAndQuillMenu::GetCaretOffset(int position)
{
    int offset = 0;
    if (!m_label->GetLabel().empty())
    {
        int length = 0;
        for (unsigned int i = 0; i < m_label->m_lineWidths.size(); ++i)
        {
            length += m_label->m_lineWidths[i];
            if (position > length)
                ++offset;
        }
    }
    return offset;
}

void FJ_BookAndQuillMenu::SetCaretPosition(int position, bool atEnd, bool force)
{
    if (m_caret == NULL)
        return;

    m_cursorAtLineEnd = atEnd;
    m_caret->SetLabel(m_caretVisible ? (m_cursorAtLineEnd ? s_caretLineEnd : s_caret) : L" ");

    if (!m_label->GetLabel().empty())
    {
        int offset = 0;
        if (!force)
            offset = GetCaretOffset(position);
        unsigned int index = position + offset;
        bool pastEnd = false;
        unsigned int charIndex = index;
        if (index >= m_label->GetLabel().size())
        {
            pastEnd = true;
            charIndex = m_label->GetLabel().size() - 1;
        }

        fuiRect bounds = m_label->GetTextField()->getCharBoundaries(charIndex);
        std::wstring character(m_label->GetLabel(), charIndex, 1);
        if (character.empty())
        {
            if (!force)
            {
                std::wstring label = m_label->GetLabel();
                std::wstring marked = label.substr(0, index) + L"#" + label.substr(index);
                m_label->m_multilineWrap = false;
                m_label->SetLabel(marked);
                SetCaretPosition(index, atEnd, true);
                m_label->SetLabel(label);
                m_label->m_multilineWrap = true;
            }
        }
        else
        {
            fuiRect start = m_caret->GetTextField()->getCharBoundaries(0);
            float lineEndAdjust = m_cursorAtLineEnd ? 0.0f : 1.0f;
            float endWidth = pastEnd ? bounds.xMax - bounds.xMin : 0.0f;
            m_caret->setX(m_label->getX() + bounds.xMin - start.xMin - lineEndAdjust + endWidth);
            start = m_caret->GetTextField()->getCharBoundaries(0);
            m_caret->setY(m_label->getY() + bounds.yMin - start.yMin);
        }
    }
    else
    {
        m_caret->setX(m_label->getX());
        m_caret->setY(m_label->getY());
    }
    m_caretVisible = true;
    m_caret->SetLabel(m_cursorAtLineEnd ? s_caretLineEnd : s_caret);
}

void FJ_BookAndQuillMenu::UpdateBookAndQuill()
{
    if (m_updateCounter % 15 == 1)
    {
        m_caretVisible = !m_caretVisible;
        if (m_caret != NULL)
            m_caret->SetLabel(m_caretVisible ? (m_cursorAtLineEnd ? s_caretLineEnd : s_caret) : L"");
    }
    m_updateCounter = m_updateCounter < 0x7fffffff ? m_updateCounter + 1 : 0;
}

void FJ_BookAndQuillMenu::_UpdateBookAndQuill(FJ_FuiNode* node, FJ_Event*)
{
    static_cast<FJ_BookAndQuillMenu*>(node)->UpdateBookAndQuill();
}

void FJ_BookAndQuillMenu::ASConstructor()
{
    m_label = static_cast<FJ_Label*>(getChildByName("BookAndQuillLabel"));
    m_caret = static_cast<FJ_Label*>(getChildByName("Caret"));
    m_updateCounter = 0;
    m_caretVisible = false;
    m_cursorAtLineEnd = false;
    m_bookIsSigned = false;
    SetBookFocus(true);
    m_pageText = L"";
    m_currentPage = 0;
    ChangePage(0, 0);
    m_label->Init(L"");
    m_label->ResizingComplete();
    m_caret->Init(s_caret);
    SetCaretPosition(0, false, false);
    addEventListener(eFJEventType_EnterFrame, this, _UpdateBookAndQuill, false);
}

void FJ_BookAndQuillMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    if (getStage()->getStageHeight() == 360.0f || getStage()->getStageHeight() == 540.0f)
        setY(top);
}

void FJ_BookAndQuillMenu::SetBookIsSigned()
{
    if (getChildByName("ButtonClose") != NULL && getChildByName("ButtonSign") != NULL)
    {
        removeEventListener(eFJEventType_EnterFrame, this, _UpdateBookAndQuill);
        removeChild(getChildByName("ButtonSign"), true);
        removeChild(getChildByName("ButtonClose"), true);
        removeChild(getChildByName("Caret"), true);
        m_caret = NULL;
    }
    m_bookIsSigned = true;
}

void FJ_BookAndQuillMenu::SetFontSizeOverride(int)
{
}

std::wstring FJ_BookAndQuillMenu::GetPageText(bool secondPage)
{
    std::wstring firstPageText = L"";
    std::wstring secondPageText = L"";
    unsigned int lineCount = m_label->GetTextField()->getNumLines();
    return m_pageText;
}

bool FJ_BookAndQuillMenu::SetPageText(const std::wstring& text, int mode)
{
    std::wstring previousText = m_pageText;
    std::wstring pageText = text;
    m_label->SetLabel(pageText + L" ");
    if (m_label->GetTextField()->getNumLines() > 13)
    {
        if (mode == 0)
        {
            pageText = GetPageText(false);
            m_label->SetLabel(pageText);
        }
        else if (mode == 1)
        {
            m_label->SetLabel(previousText);
        }
        else if (mode == 2)
        {
            pageText = GetPageText(false);
            m_label->SetLabel(pageText);
            m_pageText = pageText;
        }
        return false;
    }
    m_pageText = pageText;
    return true;
}

void FJ_BookAndQuillMenu::SetFocusToBook(bool focus)
{
    if (m_bookIsSigned)
        return;

    if (focus)
    {
        getStage()->setFocus(m_label);
        FJ_Document::OnFocusChange(true);
        SetBookFocus(true);
    }
    else
    {
        SetBookFocus(false);
    }
}

bool FJ_BookAndQuillMenu::GetBookHasFocus()
{
    return !m_bookIsSigned && getStage()->getFocus()->isFJ_Label_Book();
}

void FJ_BookAndQuillMenu::OnFocusChange(bool focused)
{
    FJ_Document::OnFocusChange(focused);
    SetBookFocus(getStage()->getFocus()->isFJ_Label_Book());
}

void FJ_BookAndQuillMenu::DoHorizontalResizeCheck()
{
    FJ_FuiNode* signButton = getChildByName("ButtonSign");
    FJ_FuiNode* closeButton = getChildByName("ButtonClose");
    int signX = 0;
    int signWidth = 0;
    int closeX = 0;
    if (signButton != NULL && closeButton != NULL)
    {
        signWidth = (int)signButton->getWidth();
        signX = (int)signButton->getX();
        closeX = (int)closeButton->getX();
    }

    FJ_Base* base = NULL;
    bool resized = false;
    int widest = 0;
    int left = 0;
    int oldWidth = 0;
    for (int i = 0; i < m_root->getNumChildren(); ++i)
    {
        FJ_FuiNode* child = m_root->getChildAt(i);
        if (child != NULL)
            base = child->asFJ_Base();
        if (base != NULL && base->isFJ_Button())
        {
            if ((int)base->getNewWidth() > widest)
            {
                widest = base->getNewWidth();
                oldWidth = base->getOldWidth();
                left = (int)base->getX();
            }
            if (base->getLabelTooWide())
                resized = true;
        }
    }

    if (resized)
    {
        for (int i = 0; i < m_root->getNumChildren(); ++i)
        {
            FJ_FuiNode* child = m_root->getChildAt(i);
            if (child->isFJ_Base() && child->isFJ_Button())
                child->asFJ_Button()->SetNewObjectWidth(widest);
        }
    }

    for (int i = 0; i < m_root->getNumChildren(); ++i)
    {
        FJ_FuiNode* child = m_root->getChildAt(i);
        if (child != NULL && child->isFJ_Base())
            child->asFJ_Base()->ResizingComplete();
    }

    if (getChildByName("ButtonClose") != NULL && getChildByName("ButtonSign") != NULL)
    {
        signButton->setX(signX - (signButton->getWidth() - signWidth));
        closeButton->setX(closeX);
    }
}
