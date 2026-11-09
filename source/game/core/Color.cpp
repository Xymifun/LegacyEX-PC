#include "game/core/Color.hpp"

Color::Color(int red, int green, int blue)
{
    this->value = 0xff000000 | ((red & 0xff) << 16) | ((green & 0xff) << 8) | (blue & 0xff);
}

Color::Color(float red, float green, float blue)
{
    this->value = 0xff000000 | (((int)(red * 255.0f) & 0xff) << 16) | (((int)(green * 255.0f) & 0xff) << 8)
                  | ((int)(blue * 255.0f) & 0xff);
}

Color Color::getHSBColor(float hue, float saturation, float brightness)
{
    int sector = (int)(hue * 6.0f) % 6;
    float offset = hue * 6.0f - (float)sector;
    float shaded = brightness * (1.0f - saturation);
    float falling = brightness * (1.0f - offset * saturation);
    float rising = brightness * (1.0f - (1.0f - offset) * saturation);
    float red;
    float green;
    float blue;

    switch (sector)
    {
    case 0:
        red = brightness;
        green = rising;
        blue = shaded;
        break;
    case 1:
        red = falling;
        green = brightness;
        blue = shaded;
        break;
    case 2:
        red = shaded;
        green = brightness;
        blue = rising;
        break;
    case 3:
        red = shaded;
        green = falling;
        blue = brightness;
        break;
    case 4:
        red = rising;
        green = shaded;
        blue = brightness;
        break;
    case 5:
        red = brightness;
        green = shaded;
        blue = falling;
        break;
    default:
        red = 0.0f;
        green = 0.0f;
        blue = 0.0f;
        break;
    }

    return Color((int)(red * 255.0f), (int)(green * 255.0f), (int)(blue * 255.0f));
}

int Color::getRGB()
{
    return this->value;
}
