#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/ui/fuiRenderNodeStage.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"
#include "game/client/render/Renderer.hpp"

#include <cmath>
#include <cstring>

static unsigned int g_fuiOldScissor[4] = {0, 0, 0, 0};

fuiRenderNode::fuiRenderNode(fuiRenderNode* stage, fuiRenderNode* parent, fuiObject* object, unsigned int symbolIndex,
                             eFuiObjectType type, unsigned char renderFlags, fuiRGBA* color, unsigned char visible,
                             bool ctorEnabled, fuiFile* file)
{
    m_stage = (fuiRenderNodeStage*)stage;
    m_file = file;
    m_type = type;
    m_symbolIndex = symbolIndex;
    m_frame = 0;
    m_object = object;
    m_parent = parent;
    m_localMatrix.scaleX = 1.0f;
    m_localMatrix.rotateSkew0 = 0.0f;
    m_localMatrix.rotateSkew1 = 0.0f;
    m_localMatrix.scaleY = 1.0f;
    m_localMatrix.translateX = 0.0f;
    m_localMatrix.translateY = 0.0f;
    m_redAdd = 0.0f;
    m_greenAdd = 0.0f;
    m_blueAdd = 0.0f;
    m_alphaAdd = 0.0f;
    m_redMultiply = 1.0f;
    m_greenMultiply = 1.0f;
    m_blueMultiply = 1.0f;
    m_alpha = 1.0f;
    m_color = color == 0 ? 0 : color->rgba;
    m_visible = visible == 0 ? 1 : visible;
    m_renderFlags = renderFlags & 3;
    m_filterFlags = renderFlags & 0xf0;
    m_node = 0;
    m_flags = ctorEnabled ? 0x14b : 0x4b;
    generateGlobalMatrix();
    m_bounds.xMin = 0.0f;
    m_bounds.xMax = 0.0f;
    m_bounds.yMin = 0.0f;
    m_bounds.yMax = 0.0f;
    m_name[0] = 0;
    m_scrollRect.xMin = 0.0f;
    m_scrollRect.xMax = 0.0f;
    m_scrollRect.yMin = 0.0f;
    m_scrollRect.yMax = 0.0f;
}

fuiRenderNode::~fuiRenderNode()
{
    delete m_node;
    m_node = 0;
}

void fuiRenderNode::shutdown()
{
}

void fuiRenderNode::handleAddedToStage()
{
    if (m_node != 0)
    {
        m_node->dispatchEvent(new FJ_Event(eFJEventType_Init, false, false));
    }
}

fuiRenderNode* fuiRenderNode::findNode(const char* name)
{
    return std::strlen(name) == 0 ? this : 0;
}

fuiRenderNode* fuiRenderNode::getNodeFromPath(const char* path)
{
    if (std::strcmp(m_name, path) == 0)
        return this;
    if (std::strcmp(path, "?") == 0)
        return this;
    return 0;
}

void fuiRenderNode::generateGlobalMatrix()
{
    if (m_parent != 0)
    {
        fuiMatrix result;
        fuiMatrix::mul(m_localMatrix, m_parent->m_globalMatrix, result);
        m_globalMatrix = result;
    }
    else
        m_globalMatrix = m_localMatrix;
}

void fuiRenderNode::disableCtor()
{
    m_flags &= ~1u;
}

void fuiRenderNode::pushMatrix(fuiMatrix& matrix)
{
    ++gFui->m_matrixDepth;
    fuiMatrix result;
    fuiMatrix::mul(matrix, gFui->m_matrixStack[gFui->m_matrixDepth - 1], result);
    fui* state = gFui;
    state->m_matrixStack[state->m_matrixDepth] = result;
    state->m_rendererMatrix[0] = matrix.scaleX;
    state->m_rendererMatrix[1] = matrix.rotateSkew0;
    state->m_rendererMatrix[4] = matrix.rotateSkew1;
    state->m_rendererMatrix[5] = matrix.scaleY;
    state->m_rendererMatrix[12] = matrix.translateX;
    state->m_rendererMatrix[13] = matrix.translateY;
    gRenderer->MatrixPush();
    gRenderer->MatrixMult(gFui->m_rendererMatrix);
}

void fuiRenderNode::setScrollRectViewport()
{
    if (m_scrollRect.xMax - m_scrollRect.xMin == 0.0f)
        return;

    gRenderer->StateGetScissorRect(g_fuiOldScissor[0], g_fuiOldScissor[1], g_fuiOldScissor[2], g_fuiOldScissor[3]);

    unsigned int left = (unsigned int)(0.0f * m_globalMatrix.scaleX + gFui->m_originX + m_globalMatrix.translateX);
    unsigned int top = (unsigned int)(0.0f * m_globalMatrix.scaleY + gFui->m_originY + m_globalMatrix.translateY);
    unsigned int right = (unsigned int)((m_scrollRect.xMax - m_scrollRect.xMin) * m_globalMatrix.scaleX
                                        + gFui->m_originX + m_globalMatrix.translateX);
    unsigned int bottom = (unsigned int)((m_scrollRect.yMax - m_scrollRect.yMin) * m_globalMatrix.scaleY
                                         + gFui->m_originY + m_globalMatrix.translateY);
    gRenderer->StateSetScissorRect(left, top, right, bottom);

    fuiMatrix translation = {1.0f, 0.0f, 0.0f, 1.0f, -m_scrollRect.xMin, -m_scrollRect.yMin};
    pushMatrix(translation);
}

void fuiRenderNode::popMatrix()
{
    --gFui->m_matrixDepth;
    gRenderer->MatrixPop();
}

void fuiRenderNode::clearScrollRectViewport()
{
    if (m_scrollRect.xMax - m_scrollRect.xMin != 0.0f)
    {
        gRenderer->StateSetScissorRect(g_fuiOldScissor[0], g_fuiOldScissor[1], g_fuiOldScissor[2], g_fuiOldScissor[3]);
        g_fuiOldScissor[0] = 0;
        g_fuiOldScissor[2] = 0;
        popMatrix();
    }
}

void fuiRenderNode::setVisibility(bool visible)
{
    if (visible)
        m_flags |= 8;
    else
        m_flags &= ~8u;
}

bool fuiRenderNode::isVisible()
{
    return (m_flags >> 3 & 1) != 0;
}

void fuiRenderNode::setAlpha(float alpha)
{
    m_alpha = alpha;
}

float fuiRenderNode::getAlpha()
{
    return m_alpha;
}

fuiRenderNode* fuiRenderNode::asTimeline()
{
    return m_type == eFuiObjectType_Timeline ? this : 0;
}

void fuiRenderNode::progogateBounds()
{
    if (m_parent != 0)
    {
        fuiRenderNode* timeline = m_parent->asTimeline();
        if (timeline != 0)
            static_cast<fuiRenderNodeTimeline*>(timeline)->progogateBoundsFromChild();
    }
}

void fuiRenderNode::setX(float x)
{
    m_localMatrix.translateX = x;
    generateGlobalMatrix();
    progogateBounds();
}

void fuiRenderNode::setY(float y)
{
    m_localMatrix.translateY = y;
    generateGlobalMatrix();
    progogateBounds();
}

float fuiRenderNode::getX()
{
    return m_localMatrix.translateX;
}

float fuiRenderNode::getY()
{
    return m_localMatrix.translateY;
}

float fuiRenderNode::getScaleX()
{
    return std::sqrt(m_localMatrix.scaleX * m_localMatrix.scaleX
                     + m_localMatrix.rotateSkew1 * m_localMatrix.rotateSkew1);
}

void fuiRenderNode::setScaleX(float scale)
{
    float oldScale = getScaleX();
    m_localMatrix.scaleX *= scale / oldScale;
    m_localMatrix.rotateSkew1 *= scale / oldScale;
    generateGlobalMatrix();
    progogateBounds();
}

void fuiRenderNode::setWidth(float width)
{
    float oldWidth = getWidth();
    float oldScale = getScaleX();
    if (width > 0.0f)
        setScaleX(oldScale * (width / oldWidth));
    if (getWidth() != width)
        getWidth();
}

float fuiRenderNode::getScaleY()
{
    return std::sqrt(m_localMatrix.rotateSkew0 * m_localMatrix.rotateSkew0
                     + m_localMatrix.scaleY * m_localMatrix.scaleY);
}

void fuiRenderNode::setScaleY(float scale)
{
    float oldScale = getScaleY();
    m_localMatrix.rotateSkew0 *= scale / oldScale;
    m_localMatrix.scaleY *= scale / oldScale;
    generateGlobalMatrix();
    progogateBounds();
}

void fuiRenderNode::setHeight(float height)
{
    float oldHeight = getHeight();
    if (height > 0.0f)
        setScaleY(getScaleY() * (height / oldHeight));
}

float fuiRenderNode::getWidth()
{
    fuiRect bounds;
    getParentSpaceBounds(&bounds);
    return bounds.xMax - bounds.xMin;
}

float fuiRenderNode::getHeight()
{
    fuiRect bounds;
    getParentSpaceBounds(&bounds);
    return bounds.yMax - bounds.yMin;
}

void fuiRenderNode::getParentSpaceBounds(fuiRect* bounds)
{
    bounds->xMin = m_bounds.xMin * m_localMatrix.scaleX + m_localMatrix.translateX;
    bounds->xMax = m_bounds.xMax * m_localMatrix.scaleX + m_localMatrix.translateX;
    bounds->yMin = m_bounds.yMin * m_localMatrix.scaleY + m_localMatrix.translateY;
    bounds->yMax = m_bounds.yMax * m_localMatrix.scaleY + m_localMatrix.translateY;
}

fuiRenderNodeStage* fuiRenderNode::getStage()
{
    return m_stage;
}

fuiRenderNode* fuiRenderNode::getRoot()
{
    return m_stage->getChild();
}

void fuiRenderNode::pushMatrix()
{
    pushMatrix(m_localMatrix);
}

void fuiRenderNode::pushColor()
{
    int current = ++gFui->m_colorDepth;
    gFui->m_colorStack[current] = gFui->m_colorStack[current - 1];
    gFui->m_colorStack[current].redAdd += m_redAdd;
    gFui->m_colorStack[current].greenAdd += m_greenAdd;
    gFui->m_colorStack[current].blueAdd += m_blueAdd;
    gFui->m_colorStack[current].alphaAdd += m_alphaAdd;
    gFui->m_colorStack[current].redMultiply *= m_redMultiply;
    gFui->m_colorStack[current].greenMultiply *= m_greenMultiply;
    gFui->m_colorStack[current].blueMultiply *= m_blueMultiply;
    gFui->m_colorStack[current].alphaMultiply *= m_alpha;
}

void fuiRenderNode::popColor()
{
    --gFui->m_colorDepth;
}

void fuiRenderNode::pushFilterFlags()
{
    int current = ++gFui->m_filterDepth;
    gFui->m_filterStack[current] = gFui->m_filterStack[current - 1] | m_renderFlags;
}

void fuiRenderNode::popFilterFlags()
{
    --gFui->m_filterDepth;
}

fuiRenderNodeEditText* fuiRenderNode::asEditText()
{
    return m_type == eFuiObjectType_EditText ? static_cast<fuiRenderNodeEditText*>(this) : 0;
}
