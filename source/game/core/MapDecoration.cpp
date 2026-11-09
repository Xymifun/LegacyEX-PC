#include "game/core/MapDecoration.hpp"

#include <cstddef>

#include "game/core/Mth.hpp"

const MapDecoration::Type MapDecoration::Type::TYPES[28] = {
    MapDecoration::Type(0, false, true, -1),        MapDecoration::Type(1, false, true, -1),
    MapDecoration::Type(2, false, true, -1),        MapDecoration::Type(3, false, true, -1),
    MapDecoration::Type(4, false, false, -1),       MapDecoration::Type(5, false, false, -1),
    MapDecoration::Type(6, false, false, -1),       MapDecoration::Type(7, true, false, -1),
    MapDecoration::Type(8, false, true, -1),        MapDecoration::Type(9, false, true, -1),
    MapDecoration::Type(10, false, true, -1),       MapDecoration::Type(11, false, true, -1),
    MapDecoration::Type(12, false, false, -1),      MapDecoration::Type(13, false, false, -1),
    MapDecoration::Type(14, true, false, 0x524C44), MapDecoration::Type(15, true, false, 0x3A7265),
    MapDecoration::Type(16, false, true, -1),       MapDecoration::Type(17, false, true, -1),
    MapDecoration::Type(18, false, true, -1),       MapDecoration::Type(19, false, true, -1),
    MapDecoration::Type(20, true, false, 0x675AAD), MapDecoration::Type(21, false, false, -1),
    MapDecoration::Type(22, false, false, -1),      MapDecoration::Type(23, true, false, -1),
    MapDecoration::Type(24, false, true, -1),       MapDecoration::Type(25, false, true, -1),
    MapDecoration::Type(26, false, true, -1),       MapDecoration::Type(27, false, true, -1)};

MapDecoration::Type::Type(int icon, bool renderedOnFrame, bool player, int mapColor)
{
    this->icon = icon;
    this->renderedOnFrame = renderedOnFrame;
    this->player = player;
    this->mapColor = mapColor;
}

const MapDecoration::Type* MapDecoration::Type::byIcon(unsigned char icon)
{
    return &TYPES[Mth::clamp((int)icon, 0, 27)];
}

unsigned char MapDecoration::Type::getIcon() const
{
    return this->icon;
}

bool MapDecoration::Type::isRenderedOnFrame() const
{
    return this->renderedOnFrame;
}

bool MapDecoration::Type::isPlayer() const
{
    return this->player;
}

bool MapDecoration::Type::hasMapColor() const
{
    return this->mapColor >= 0;
}

int MapDecoration::Type::getMapColor() const
{
    return this->mapColor;
}

MapDecoration::MapDecoration()
{
    this->type = NULL;
    this->x = 0;
    this->y = 0;
    this->rot = 0;
    this->id = 0;
    this->visible = false;
}

MapDecoration::MapDecoration(const Type* type, char x, char y, char rot, int id, bool visible)
{
    this->type = type;
    this->x = x;
    this->y = y;
    this->rot = rot;
    this->id = id;
    this->visible = visible;
}

const MapDecoration::Type* MapDecoration::getType() const
{
    return this->type;
}

unsigned int MapDecoration::getImg() const
{
    return this->type->getIcon();
}

int MapDecoration::getX() const
{
    return (char)this->x;
}

int MapDecoration::getY() const
{
    return (char)this->y;
}

int MapDecoration::getRot() const
{
    return this->rot;
}

void MapDecoration::setX(char x)
{
    this->x = x;
}

void MapDecoration::setY(char y)
{
    this->y = y;
}

void MapDecoration::setRot(char rot)
{
    this->rot = rot;
}

bool MapDecoration::renderOnFrame() const
{
    return this->type->isRenderedOnFrame();
}

int MapDecoration::hashCode() const
{
    int hash = this->type->getIcon();
    hash = hash * 31 + this->x;
    hash = hash * 31 + this->y;
    hash = hash * 31 + this->rot;
    return hash;
}

bool MapDecoration::equals(MapDecoration& other)
{
    return this->type == other.type && this->x == other.x && this->y == other.y && this->rot == other.rot;
}
