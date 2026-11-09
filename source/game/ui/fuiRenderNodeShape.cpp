#include "game/ui/fuiRenderNodeShape.hpp"

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiBitmap.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiMatrix.hpp"
#include "game/ui/fuiObject.hpp"
#include "game/ui/fuiRGBA.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fuiShape.hpp"
#include "game/ui/fuiShapeComponent.hpp"
#include "game/ui/fuiVert.hpp"

extern Renderer* gRenderer;

fuiRenderNodeShape::fuiRenderNodeShape(fuiRenderNode* stage, fuiRenderNode* parent, fuiObject* object,
                                       unsigned int symbolIndex, unsigned char depth, fuiRGBA* color,
                                       unsigned char clipDepth, bool visible, fuiFile* file)
    : fuiRenderNode(stage, parent, object, symbolIndex, eFuiObjectType_Shape, depth, color, clipDepth, visible, file)
{
    fuiShape* shape = static_cast<fuiShape*>(m_object);
    m_bounds = shape->bounds;
}

fuiRenderNodeShape::~fuiRenderNodeShape()
{
}

void fuiRenderNodeShape::gather(bool recurse)
{
}

void fuiRenderNodeShape::preRender()
{
}

void fuiRenderNodeShape::render(fuiRect* bounds, fuiMatrix* matrix)
{
    if (isVisible())
    {
        pushMatrix();
        pushColor();
        pushFilterFlags();

        fuiShape* shape = static_cast<fuiShape*>(m_object);
        fuiShapeComponent* component = m_file->shapeComponentData + shape->shapeComponentIndex;
        for (unsigned int i = 0; i < shape->shapeComponentCount; ++i)
        {
            unsigned int vertexCount = component->vertexCount;
            fuiVert* vertices = m_file->vertexData + component->vertexOffset;
            if (vertexCount != 0)
            {
                C4JRender::eVertexType vertexType = C4JRender::eVertexType_ScalingGridUI;
                C4JRender::ePixelShaderType pixelShaderType = C4JRender::ePixelShaderType_UIColourAdd;
                float red = 1.0f;
                float green = 1.0f;
                float blue = 1.0f;
                float alpha = 1.0f;
                if (component->fillType == (eFuiFillType)1)
                {
                    unsigned int rgba = component->color.rgba;
                    red = ((rgba >> 24) & 0xff) / 255.0f;
                    green = ((rgba >> 16) & 0xff) / 255.0f;
                    blue = ((rgba >> 8) & 0xff) / 255.0f;
                    alpha = (rgba & 0xff) / 255.0f;
                    gRenderer->TextureBind(-1);
                }
                else
                {
                    fuiBitmap* bitmap = m_file->bitmaps + component->bitmapId;
                    if (bitmap == NULL)
                    {
                        continue;
                    }
                    m_file->initialiseRenderTexture(bitmap, true);
                    gRenderer->TextureBind(bitmap->textureId);
                    if (component->fillType == (eFuiFillType)3)
                    {
                        fuiMatrix scaleToPixels;
                        scaleToPixels.scaleX = 32768.0f;
                        scaleToPixels.rotateSkew0 = 0.0f;
                        scaleToPixels.rotateSkew1 = 0.0f;
                        scaleToPixels.scaleY = 32768.0f;
                        scaleToPixels.translateX = -16384.0f;
                        scaleToPixels.translateY = -16384.0f;
                        fuiMatrix scaleToTwips;
                        scaleToTwips.scaleX = 0.05f;
                        scaleToTwips.rotateSkew0 = 0.0f;
                        scaleToTwips.rotateSkew1 = 0.0f;
                        scaleToTwips.scaleY = 0.05f;
                        scaleToTwips.translateX = 0.0f;
                        scaleToTwips.translateY = 0.0f;
                        fuiMatrix bitmapScale;
                        fuiMatrix textureMatrix;
                        fuiMatrix::mul(scaleToPixels, scaleToTwips, bitmapScale);
                        fuiMatrix::mul(bitmapScale, component->bitmapMatrix, textureMatrix);
                        vertexType = C4JRender::eVertexType_TiledUI;
                        pixelShaderType = C4JRender::ePixelShaderType_UIAlphaTest;
                        gRenderer->StateSetTextureScaleUI(1.0f / textureMatrix.scaleX, 1.0f / textureMatrix.scaleY,
                                                          -(textureMatrix.translateX / textureMatrix.scaleX),
                                                          -(textureMatrix.translateY / textureMatrix.scaleY));
                    }
                    else
                    {
                        float width = (float)bitmap->width;
                        float height = (float)bitmap->height;
                        gRenderer->StateSetTextureScaleUI(1.0f / width, 1.0f / height,
                                                          -(component->bitmapMatrix.translateX / width),
                                                          -(component->bitmapMatrix.translateY / height));
                    }
                    if (gFui->m_filterStack[gFui->m_filterDepth] & 1)
                    {
                        gRenderer->TextureSetParam(2, 1);
                    }
                    else
                    {
                        gRenderer->TextureSetParam(2, 0);
                    }
                }

                const fui::ColorState& colour = gFui->m_colorStack[gFui->m_colorDepth];
                const float redAdd = colour.redAdd;
                const float greenAdd = colour.greenAdd;
                const float blueAdd = colour.blueAdd;
                const float alphaAdd = colour.alphaAdd;
                gRenderer->StateSetColour(red * colour.redMultiply, green * colour.greenMultiply,
                                          blue * colour.blueMultiply, alpha * colour.alphaMultiply);
                gRenderer->StateSetColourAdd(redAdd, greenAdd, blueAdd, alphaAdd);

                if (bounds == NULL)
                {
                    gRenderer->DrawVertices(C4JRender::ePrimitiveType_Triangles, vertexCount, vertices,
                                            C4JRender::eVertexType_UI, pixelShaderType, NULL);
                }
                else
                {
                    gRenderer->StateSetScalingGridUI(shape->bounds.xMin, bounds->xMin, bounds->xMax, shape->bounds.xMax,
                                                     shape->bounds.yMin, bounds->yMin, bounds->yMax, shape->bounds.yMax,
                                                     matrix->scaleX, matrix->scaleY);
                    gRenderer->DrawVertices(C4JRender::ePrimitiveType_Triangles, vertexCount, vertices, vertexType,
                                            pixelShaderType, NULL);
                }
            }
            ++component;
        }

        popFilterFlags();
        popColor();
        popMatrix();
    }
}
