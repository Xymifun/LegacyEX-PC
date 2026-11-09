#ifndef GAME_UI_FJ_BOOKANDQUILLMENU_HPP
#define GAME_UI_FJ_BOOKANDQUILLMENU_HPP

#include <string>

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class FJ_Label;
class fuiRenderNode;

class FJ_BookAndQuillMenu : public FJ_Document
{
private:
    FJ_Label* m_label;
    FJ_Label* m_caret;
    unsigned int m_unused;
    int m_updateCounter;
    bool m_caretVisible;
    bool m_cursorAtLineEnd;
    std::wstring m_pageText;
    int m_currentPage;
    bool m_bookIsSigned;

public:
    FJ_BookAndQuillMenu(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void SetBookFocus(bool focus);
    void ChangePage(int page, int pageCount);
    int GetCaretOffset(int position);
    void SetCaretPosition(int position, bool atEnd, bool force);
    void UpdateBookAndQuill();
    static void _UpdateBookAndQuill(FJ_FuiNode* node, FJ_Event* event);
    virtual void ASConstructor();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
    void SetBookIsSigned();
    void SetFontSizeOverride(int size);
    std::wstring GetPageText(bool secondPage);
    bool SetPageText(const std::wstring& text, int mode);
    void SetFocusToBook(bool focus);
    bool GetBookHasFocus();
    virtual void OnFocusChange(bool focused);
    virtual void DoHorizontalResizeCheck();
};

#endif
