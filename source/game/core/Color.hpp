#ifndef GAME_CORE_COLOR_HPP
#define GAME_CORE_COLOR_HPP

class Color
{
public:
    Color(int red, int green, int blue);
    Color(float red, float green, float blue);

    static Color getHSBColor(float hue, float saturation, float brightness);

    int getRGB();

private:
    int value;
};

#endif
