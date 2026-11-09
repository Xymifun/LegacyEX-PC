#ifndef GAME_UI_FJ_LABEL_HPP
#define GAME_UI_FJ_LABEL_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_Base.hpp"

class fuiRenderNode;
class fuiRenderNodeEditText;

class FJ_Label : public FJ_Base
{
public:
    enum ETextAlignment
    {
        eTextAlignment_Default = 0,
        eTextAlignment_Left = 1,
        eTextAlignment_Center = 2
    };

    explicit FJ_Label(fuiRenderNode* renderNode);
    virtual ~FJ_Label() {}
    static FJ_Base* Create(fuiRenderNode* renderNode);
    void SetTextAlignment(ETextAlignment alignment);
    void setUseHtmlText(bool useHtmlText);
    virtual unsigned int getType();
    virtual void Init(const std::wstring& label);
    void ResetLabel();
    virtual void SetLabel(const std::wstring& label);
    int GetAverageCharHeight();
    virtual void GetLabelTooWide(fuiRenderNodeEditText* textField);
    void setAction(unsigned char action, char* value);

protected:
    ETextAlignment m_textAlignment;
    bool m_multiline;
    bool m_wordWrap;
    bool m_useEllipsis;
    bool m_manualWordWrap;
};

#endif
