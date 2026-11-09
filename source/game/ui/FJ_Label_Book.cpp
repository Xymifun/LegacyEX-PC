#include "game/ui/FJ_Label_Book.hpp"

void FJ_Label_Book::Init(const std::wstring& label)
{
    FJ_Base::InitBase(label, -1);
}

void FJ_Label_Book::SetLabel(const std::wstring& label)
{
    FJ_Base::SetLabelAlignment(m_textAlignment);
    m_initialised = true;
    FJ_Label::SetLabel(label);
}
