#ifndef GAME_UI_FJ_LABELOUTLINE_HPP
#define GAME_UI_FJ_LABELOUTLINE_HPP

#include <string>
#include <vector>

#include "game/ui/FJ_Base.hpp"

class fuiRenderNode;
class fuiRenderNodeEditText;

class FJ_LabelOutline : public FJ_Base
{
public:
    explicit FJ_LabelOutline(fuiRenderNode* renderNode);
    virtual ~FJ_LabelOutline() {}
    virtual void SetLabel(const std::wstring& label);
    void Init(const std::wstring& label);

private:
    bool m_centreLabel;
    fuiRenderNodeEditText* m_labelText;
    std::vector<fuiRenderNodeEditText*> m_outlineText;
    std::vector<float> m_outlineX;
    int m_labelX;
    int m_halfWidth;
};

#endif
