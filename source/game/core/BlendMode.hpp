#ifndef GAME_CORE_BLENDMODE_HPP
#define GAME_CORE_BLENDMODE_HPP

class BlendMode
{
public:
    void _init(bool, bool, int, int, int, int, int);

    BlendMode();
    BlendMode(int, int, int);
    BlendMode(int, int, int, int, int);

    bool equals(BlendMode* other);
    bool isOpaque();
    void apply();
    int hashCode();
};

#endif
