#ifndef GAME_CORE_BORDERSTATUS_HPP
#define GAME_CORE_BORDERSTATUS_HPP

class BorderStatus
{
public:
    BorderStatus(int color);
    ~BorderStatus();

    int getColor();

private:
    int color;
};

#endif
