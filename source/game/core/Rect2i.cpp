#include "game/core/Rect2i.hpp"

Rect2i::Rect2i(int x, int y, int width, int height)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
}

int Rect2i::getX() const
{
    return x;
}

int Rect2i::getY() const
{
    return y;
}

int Rect2i::getWidth() const
{
    return width;
}

int Rect2i::getHeight() const
{
    return height;
}

bool Rect2i::intersect(const Rect2i* other)
{
    const int right = x + width;
    const int bottom = y + height;
    const int otherRight = other->x + other->width;
    const int otherBottom = other->y + other->height;

    if (x < other->x)
    {
        x = other->x;
    }
    if (y < other->y)
    {
        y = other->y;
    }

    const int newRight = right < otherRight ? right : otherRight;
    const int newBottom = bottom < otherBottom ? bottom : otherBottom;
    width = newRight - x;
    height = newBottom - y;

    return width > 0 && height > 0;
}

void Rect2i::setX(int x)
{
    this->x = x;
}

void Rect2i::setY(int y)
{
    this->y = y;
}

void Rect2i::setWidth(int width)
{
    this->width = width;
}

void Rect2i::setHeight(int height)
{
    this->height = height;
}

void Rect2i::setPosition(int x, int y)
{
    this->x = x;
    this->y = y;
}
