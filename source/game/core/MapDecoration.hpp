#ifndef GAME_CORE_MAPDECORATION_HPP
#define GAME_CORE_MAPDECORATION_HPP

class MapDecoration
{
public:
    class Type
    {
    public:
        static const Type TYPES[28];

        Type(int icon, bool renderedOnFrame, bool player, int mapColor);

        static const Type* byIcon(unsigned char icon);

        unsigned char getIcon() const;
        bool isRenderedOnFrame() const;
        bool isPlayer() const;
        bool hasMapColor() const;
        int getMapColor() const;

    private:
        unsigned char icon;
        bool renderedOnFrame;
        bool player;
        int mapColor;
    };

    MapDecoration();
    MapDecoration(const Type* type, char x, char y, char rot, int id, bool visible);

    const Type* getType() const;
    unsigned int getImg() const;
    int getX() const;
    int getY() const;
    int getRot() const;
    void setX(char x);
    void setY(char y);
    void setRot(char rot);
    bool renderOnFrame() const;
    int hashCode() const;
    bool equals(MapDecoration& other);

private:
    const Type* type;
    int x;
    int y;
    char rot;

public:
    int id;
    bool visible;
};

#endif
