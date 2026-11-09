#include "game/core/GuiComponent.hpp"

#include <cmath>

#include "game/client/render/Font.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"

GuiComponent::GuiComponent() : zLevel(0.0f)
{
}

void GuiComponent::fill(int x0, int y0, int x1, int y1, int color)
{
    if (x0 < x1)
    {
        int x = x0;
        x0 = x1;
        x1 = x;
    }
    if (y0 < y1)
    {
        int y = y0;
        y0 = y1;
        y1 = y;
    }

    float alpha = static_cast<float>((color >> 24) & 255) / 255.0f;
    float red = static_cast<float>((color >> 16) & 255) / 255.0f;
    float green = static_cast<float>((color >> 8) & 255) / 255.0f;
    float blue = static_cast<float>(color & 255) / 255.0f;
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    GlStateManager::enableBlend();
    GlStateManager::disableTexture();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::color4f(red, green, blue, alpha);
    builder->begin();
    builder->vertex(static_cast<float>(x0), static_cast<float>(y1), 0.0f);
    builder->vertex(static_cast<float>(x1), static_cast<float>(y1), 0.0f);
    builder->vertex(static_cast<float>(x1), static_cast<float>(y0), 0.0f);
    builder->vertex(static_cast<float>(x0), static_cast<float>(y0), 0.0f);
    builder->end();
    GlStateManager::enableTexture();
    GlStateManager::disableBlend();
}

void GuiComponent::fillGradient(int x0, int y0, int x1, int y1, int color0, int color1)
{
    float alpha0 = static_cast<float>((color0 >> 24) & 255) / 255.0f;
    float red0 = static_cast<float>((color0 >> 16) & 255) / 255.0f;
    float green0 = static_cast<float>((color0 >> 8) & 255) / 255.0f;
    float blue0 = static_cast<float>(color0 & 255) / 255.0f;
    float alpha1 = static_cast<float>((color1 >> 24) & 255) / 255.0f;
    float red1 = static_cast<float>((color1 >> 16) & 255) / 255.0f;
    float green1 = static_cast<float>((color1 >> 8) & 255) / 255.0f;
    float blue1 = static_cast<float>(color1 & 255) / 255.0f;
    GlStateManager::disableTexture();
    GlStateManager::enableBlend();
    GlStateManager::disableAlphaTest();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::shadeModel(0);
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    builder->begin();
    builder->color(red0, green0, blue0, alpha0);
    builder->vertex(static_cast<float>(x1), static_cast<float>(y0), zLevel);
    builder->vertex(static_cast<float>(x0), static_cast<float>(y0), zLevel);
    builder->color(red1, green1, blue1, alpha1);
    builder->vertex(static_cast<float>(x0), static_cast<float>(y1), zLevel);
    builder->vertex(static_cast<float>(x1), static_cast<float>(y1), zLevel);
    builder->end();
    GlStateManager::shadeModel(0);
    GlStateManager::disableBlend();
    GlStateManager::enableAlphaTest();
    GlStateManager::enableTexture();
}

void GuiComponent::drawCenteredString(Font* font, const std::wstring& text, int x, int y, int color)
{
    int width = font->width(text);
    font->drawShadow(text, x - (width / 2), y, color);
}

void GuiComponent::drawString(Font* font, const std::wstring& text, int x, int y, int color)
{
    font->drawShadow(text, x, y, color);
}

void GuiComponent::blit(int x, int y, int u, int v, int width, int height)
{
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    builder->begin();
    float scale = 0.75f;
    float xOffset = scale * Minecraft::GetInstance()->mWidth;
    xOffset = xOffset / Minecraft::GetInstance()->mFullWidth / gGuiScreenScale;
    float yOffset = scale / gGuiScreenScale;
    float x0 = floorf(static_cast<float>(x) * gGuiScreenScale) / gGuiScreenScale;
    float y0 = floorf(static_cast<float>(y) * gGuiScreenScale) / gGuiScreenScale;
    float scaledWidth = floorf(static_cast<float>(width) * gGuiScreenScale) / gGuiScreenScale;
    float scaledHeight = floorf(static_cast<float>(height) * gGuiScreenScale) / gGuiScreenScale;
    float us = 0.00390625f;
    float vs = 0.00390625f;
    builder->vertexUV(x0 - xOffset, y0 + scaledHeight - yOffset, zLevel, static_cast<float>(u) * us,
                      static_cast<float>(v + height) * vs);
    builder->vertexUV(x0 + scaledWidth - xOffset, y0 + scaledHeight - yOffset, zLevel,
                      static_cast<float>(u + width) * us, static_cast<float>(v + height) * vs);
    builder->vertexUV(x0 + scaledWidth - xOffset, y0 - yOffset, zLevel, static_cast<float>(u + width) * us,
                      static_cast<float>(v) * vs);
    builder->vertexUV(x0 - xOffset, y0 - yOffset, zLevel, static_cast<float>(u) * us, static_cast<float>(v) * vs);
    builder->end();
}
