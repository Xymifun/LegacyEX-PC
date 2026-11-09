#ifndef GAME_CORE_RECT2I_HPP
#define GAME_CORE_RECT2I_HPP

class Rect2i
{
public:
    Rect2i(int x, int y, int width, int height);

    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;

    bool intersect(const Rect2i* other);

    void setX(int x);
    void setY(int y);
    void setWidth(int width);
    void setHeight(int height);
    void setPosition(int x, int y);

    int x;
    int y;
    int width;
    int height;
};

#endif
