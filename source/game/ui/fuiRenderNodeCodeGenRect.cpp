#include "game/ui/fuiRenderNodeCodeGenRect.hpp"

#include <cstring>

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiFile.hpp"

extern Renderer* gRenderer;

fuiRenderNodeCodeGenRect::fuiRenderNodeCodeGenRect(fuiRenderNode* parent, fuiRect* rect, const char* name,
                                                   unsigned int textureWidth, unsigned int textureHeight, int textureId,
                                                   bool customTexture, bool textureAlpha)
    : fuiRenderNode(parent, 0, 0, 0, eFuiObjectType_CodeGenRect, 0, 0, 0, false, 0)
{
    m_bounds = *rect;
    std::strncpy(m_name, name, sizeof(m_name));
    mTextureId = textureId;
    mCustomTexture = customTexture;
    mTextureWidth = textureWidth;
    mTextureHeight = textureHeight;
    mTextureAlpha = textureAlpha;
}

fuiRenderNodeCodeGenRect::fuiRenderNodeCodeGenRect(fuiRenderNode* parent, fuiRect* rect, const char* name)
    : fuiRenderNode(parent, 0, 0, 0, eFuiObjectType_CodeGenRect, 0, 0, 0, false, 0)
{
    m_bounds = *rect;
    std::strncpy(m_name, name, sizeof(m_name));
    mCustomTexture = 0;
    mTextureWidth = 0;
    mTextureId = -1;
    mTextureHeight = 0;
}

fuiRenderNodeCodeGenRect::~fuiRenderNodeCodeGenRect()
{
    if (mTextureId >= 0)
    {
        if (mCustomTexture)
        {
            if (!mTextureAlpha)
            {
                gFui->releaseCustomTexture(mTextureId);
            }
        }
        else
        {
            gRenderer->TextureFree(mTextureId);
        }
    }
}

void fuiRenderNodeCodeGenRect::gather(bool recurse)
{
    (void)recurse;
}

void fuiRenderNodeCodeGenRect::preRender()
{
}

void fuiRenderNodeCodeGenRect::render(fuiRect* bounds, fuiMatrix* matrix)
{
    if (!isVisible())
        return;

    if (std::strncmp(m_name, "iggy_", 5) == 0)
    {
        if (getRoot()->m_file->customDrawCallback != 0)
        {
            gRenderer->StateSetFaceCull(true);
            gRenderer->StateSetFaceCullCW(true);
            gRenderer->StateSetAlphaTestEnable(true);
            gRenderer->StateSetDepthTestEnable(true);
            gRenderer->StateSetDepthMask(true);
            gRenderer->StateSetColour(1.0f, 1.0f, 1.0f, 1.0f);
            pushMatrix();
            fuiFile* rootFile = getRoot()->m_file;
            rootFile->customDrawCallback(rootFile->customDrawContext, m_name + 5, &m_bounds);
            popMatrix();
            gRenderer->StateSetFaceCull(false);
            gRenderer->StateSetBlendEnable(true);
            gRenderer->StateSetBlendFunc(1, 5);
            gRenderer->StateSetAlphaTestEnable(false);
            gRenderer->StateSetDepthTestEnable(false);
            gRenderer->StateSetDepthMask(false);
        }
        return;
    }

    pushMatrix();
    pushColor();
    pushFilterFlags();
    const float bottom = m_bounds.yMin + (m_bounds.yMax - m_bounds.yMin);
    const float right = m_bounds.xMin + (m_bounds.xMax - m_bounds.xMin);
    float vertices[12] = {m_bounds.xMin, m_bounds.yMin, right, bottom,        m_bounds.xMin, bottom,
                          m_bounds.xMin, m_bounds.yMin, right, m_bounds.yMin, right,         bottom};
    if (mTextureId == -1)
    {
        gRenderer->TextureBind(-1);
        gRenderer->StateSetColour(1.0f, 0.0f, 0.0f, 0.0f);
        gRenderer->DrawVertices(C4JRender::ePrimitiveType_Triangles, 6, vertices, C4JRender::eVertexType_UI,
                                C4JRender::ePixelShaderType_UIColourAdd, NULL);
    }
    else
    {
        const fui::ColorState& color = gFui->m_colorStack[gFui->m_colorDepth];
        const float redAdd = color.redAdd;
        const float greenAdd = color.greenAdd;
        const float blueAdd = color.blueAdd;
        const float alphaAdd = color.alphaAdd;
        gRenderer->StateSetColour(color.redMultiply, color.greenMultiply, color.blueMultiply, color.alphaMultiply);
        gRenderer->StateSetColourAdd(redAdd, greenAdd, blueAdd, alphaAdd);
        gRenderer->TextureBind(mTextureId);
        const float width = m_bounds.xMax - m_bounds.xMin;
        const float height = m_bounds.yMax - m_bounds.yMin;
        gRenderer->StateSetTextureScaleUI(1.0f / width, 1.0f / height, -m_bounds.xMin / width, -m_bounds.yMin / height);
        gRenderer->DrawVertices(C4JRender::ePrimitiveType_Triangles, 6, vertices, C4JRender::eVertexType_UI,
                                C4JRender::ePixelShaderType_UIColourAdd, NULL);
    }
    popFilterFlags();
    popColor();
    popMatrix();
}

void fuiRenderNodeCodeGenRect::setWidth(float width)
{
    m_bounds.xMax = m_bounds.xMin + width;
}

void fuiRenderNodeCodeGenRect::setHeight(float height)
{
    m_bounds.yMax = m_bounds.yMin + height;
}

void fuiRenderNodeCodeGenRect::setTexture(unsigned int textureWidth, unsigned int textureHeight, int textureId,
                                          bool textureAlpha)
{
    if (mTextureId >= 0)
    {
        if (mCustomTexture)
        {
            if (!mTextureAlpha)
            {
                gFui->releaseCustomTexture(mTextureId);
            }
        }
        else
        {
            gRenderer->TextureFree(mTextureId);
        }
    }
    mTextureId = textureId;
    mTextureWidth = textureWidth;
    mTextureHeight = textureHeight;
    mTextureAlpha = textureAlpha;
}
