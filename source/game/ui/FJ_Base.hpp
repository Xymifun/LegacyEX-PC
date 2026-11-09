#ifndef GAME_UI_FJ_BASE_HPP
#define GAME_UI_FJ_BASE_HPP

#include <stdint.h>
#include <string>
#include <vector>
#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;
class fuiRenderNodeEditText;

class FJ_Base : public FJ_FuiNode
{
    friend class FJ_Document;
    friend class FJ_MessageBox;
    friend class FJ_List;
    friend class FJ_List2D;
    friend class fuiRenderNodeTimeline;

public:
    explicit FJ_Base(fuiRenderNode* renderNode);
    virtual ~FJ_Base();
    FJ_Base* getObjListParent();
    uint32_t getId();
    void LostFocus();
    uint32_t getNewWidth();
    void ResizingComplete();
    void ResetResizing();
    void setObjListParent(FJ_FuiNode* parent);
    uint32_t getListId();
    void setListId(int id);
    void setPreScaleWidth(int width);
    void setId(int id);
    virtual uint8_t getLabelTooWide();
    virtual int getOldWidth();
    void CorrectPixelPosition();
    void InitBase(const std::wstring& name, int id);
    fuiRenderNodeEditText* GetTextField();
    void CheckLabelWidth(const std::wstring& label);
    void CheckLabelWidths(std::vector<std::wstring>& labels);
    void SetLabelAlignment(int alignment);
    void DoManualWordWrapCheck(FJ_FuiNode* container, fuiRenderNodeEditText* textField);
    void DoMultilineWordWrap(FJ_FuiNode* container, fuiRenderNodeEditText* textField);
    void DoEllipsisCheck(FJ_FuiNode* container, fuiRenderNodeEditText* textField);
    virtual void SetLabel(const std::wstring& label);
    const std::wstring& GetLabel();
    void SetTextOnly(const std::wstring& text);
    virtual void GetLabelTooWide(fuiRenderNodeEditText* textField);
    virtual void SetNewObjectWidth(int width);
    virtual void SetNewObjectHeight(int height);
    virtual void HandleCharInput(unsigned int character);
    virtual uint32_t getType();

protected:
    friend class FJ_BookAndQuillMenu;

    std::string m_navUpName;
    std::string m_navDownName;
    std::string m_navLeftName;
    std::string m_navRightName;
    int m_id;
    int m_labelAlignment;
    bool m_isLabel;
    bool m_textContainerChanged;
    bool m_textContainerVisible;
    bool m_manualWrap;
    bool m_htmlText;
    bool m_multilineWrap;
    int m_lineCount;
    std::vector<int> m_lineWidths;
    bool m_manualWrapFits;
    bool m_multilineWrapFits;
    bool m_autoGrowHeight;
    std::wstring m_label;
    float m_centerBiasX;
    float m_objectWidth;
    float m_objectHeight;
    bool m_initialised;
    bool m_resizing;
    bool m_textChanged;
    bool m_labelTooWide;
    bool m_labelOverflowResolved;
    int m_newWidth;
    int m_ellipsisWidth;
    bool m_textOnly;
    int m_ellipsisWidthOverride;
    int m_wrapWidth;
    int m_preScaleWidth;
    int m_maxLabelWidth;
    std::vector<FJ_FuiNode*> m_objects;
    FJ_FuiNode* m_objListParent;
    int m_listId;
};

#endif
