#ifndef GAME_CORE_GUICOMPONENT_HPP
#define GAME_CORE_GUICOMPONENT_HPP

#include <string>

class Font;

extern float gGuiScreenScale;

class GuiComponent
{
public:
    GuiComponent();

    void fill(int x0, int y0, int x1, int y1, int color);
    void fillGradient(int x0, int y0, int x1, int y1, int color0, int color1);
    void drawCenteredString(Font* font, const std::wstring& text, int x, int y, int color);
    void drawString(Font* font, const std::wstring& text, int x, int y, int color);
    void blit(int x, int y, int u, int v, int width, int height);
    void hLine(int x0, int x1, int y, int color);
    void vLine(int x, int y0, int y1, int color);

    float zLevel;
};

#endif
