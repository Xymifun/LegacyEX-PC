#ifndef GAME_CORE_SHAPE_HPP
#define GAME_CORE_SHAPE_HPP

class Shape
{
public:
    float* values;
    unsigned int width;
    unsigned int depth;

    Shape(unsigned int width, unsigned int depth);
    ~Shape();

    float& at(unsigned int x, unsigned int z);
    const float& at(unsigned int x, unsigned int z) const;
};

#endif
