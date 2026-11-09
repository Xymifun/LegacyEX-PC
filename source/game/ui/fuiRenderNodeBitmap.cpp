#include "game/ui/fuiRenderNodeBitmap.hpp"

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiBitmap.hpp"
#include "game/ui/fuiFile.hpp"

fuiRenderNodeBitmap::fuiRenderNodeBitmap(fuiRenderNode* parent, fuiRenderNode* root, fuiObject* object,
                                         unsigned int symbolIndex, unsigned char depth, fuiRGBA* color,
                                         unsigned char clipDepth, bool visible, fuiFile* file)
    : fuiRenderNode(parent, root, object, symbolIndex, eFuiObjectType_Bitmap, depth, color, clipDepth, visible, file)
{
    fuiBitmap* bitmap = static_cast<fuiBitmap*>(m_object);
    m_bounds.xMin = 0.0f;
    m_bounds.xMax = (float)bitmap->width;
    m_bounds.yMin = 0.0f;
    m_bounds.yMax = (float)bitmap->height;
}

fuiRenderNodeBitmap::~fuiRenderNodeBitmap()
{
}

void fuiRenderNodeBitmap::gather(bool)
{
}

void fuiRenderNodeBitmap::preRender()
{
}

#ifdef MINECRAFT_PC
#include "game/ui/fuiSymbol.hpp"

int PcButtonGlyphTexture(const char* name, int textureId);
int PcTitleLogoTexture(const char* name, int textureId);
int PcPanoramaTexture(const char* name, int textureId);
#endif

void fuiRenderNodeBitmap::render(fuiRect* bounds, fuiMatrix* matrix)
{
    if (!isVisible())
    {
        return;
    }

    pushMatrix();
    pushColor();
    pushFilterFlags();

    {
        fuiBitmap* bitmap = static_cast<fuiBitmap*>(m_object);
        float vertices[12];
        vertices[0] = 0.0f;
        vertices[1] = 0.0f;
        vertices[2] = (float)bitmap->width;
        vertices[3] = (float)bitmap->height;
        vertices[4] = 0.0f;
        vertices[5] = (float)bitmap->height;
        vertices[6] = 0.0f;
        vertices[7] = 0.0f;
        vertices[8] = (float)bitmap->width;
        vertices[9] = 0.0f;
        vertices[10] = (float)bitmap->width;
        vertices[11] = (float)bitmap->height;

        m_file->initialiseRenderTexture(bitmap, true);
#ifdef MINECRAFT_PC
        int textureId = bitmap->textureId;
        if (bitmap->symbolIndex != static_cast<unsigned int>(-1))
        {
            textureId = PcButtonGlyphTexture(m_file->symbolData[bitmap->symbolIndex].name, textureId);
            textureId = PcTitleLogoTexture(m_file->symbolData[bitmap->symbolIndex].name, textureId);
            textureId = PcPanoramaTexture(m_file->symbolData[bitmap->symbolIndex].name, textureId);
        }
        gRenderer->TextureBind(textureId);
#else
        gRenderer->TextureBind(bitmap->textureId);
#endif
        gRenderer->StateSetTextureScaleUI(1.0f / (float)bitmap->width, 1.0f / (float)bitmap->height, 0.0f, 0.0f);

        const float redMultiply = gFui->m_colorStack[gFui->m_colorDepth].redMultiply;
        const float greenMultiply = gFui->m_colorStack[gFui->m_colorDepth].greenMultiply;
        const float blueMultiply = gFui->m_colorStack[gFui->m_colorDepth].blueMultiply;
        const float alphaMultiply = gFui->m_colorStack[gFui->m_colorDepth].alphaMultiply;
        const float redAdd = gFui->m_colorStack[gFui->m_colorDepth].redAdd;
        const float greenAdd = gFui->m_colorStack[gFui->m_colorDepth].greenAdd;
        const float blueAdd = gFui->m_colorStack[gFui->m_colorDepth].blueAdd;
        const float alphaAdd = gFui->m_colorStack[gFui->m_colorDepth].alphaAdd;

        if (gFui->m_filterStack[gFui->m_filterDepth] & 1)
        {
            gRenderer->TextureSetParam(1, 1);
            gRenderer->TextureSetParam(2, 1);
        }
        else
        {
            gRenderer->TextureSetParam(1, 0);
            gRenderer->TextureSetParam(2, 0);
        }
        gRenderer->StateSetColour(redMultiply, greenMultiply, blueMultiply, alphaMultiply);
        gRenderer->StateSetColourAdd(redAdd, greenAdd, blueAdd, alphaAdd);
        gRenderer->DrawVertices(C4JRender::ePrimitiveType_Triangles, 6, vertices, C4JRender::eVertexType_UI,
                                C4JRender::ePixelShaderType_UIColourAdd, NULL);
    }

    popFilterFlags();
    popColor();
    popMatrix();
}
