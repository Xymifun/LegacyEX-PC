#include "game/util/RandomValueBounds.hpp"

RandomValueBounds::RandomValueBounds() : min(0.0f), max(0.0f)
{
}

RandomValueBounds::RandomValueBounds(int min, int max) : min(static_cast<float>(min)), max(static_cast<float>(max))
{
}

RandomValueBounds::RandomValueBounds(double min, double max)
    : min(static_cast<float>(min)),
      max(static_cast<float>(max))
{
}

RandomValueBounds::RandomValueBounds(int min, double max) : min(static_cast<float>(min)), max(static_cast<float>(max))
{
}

RandomValueBounds::RandomValueBounds(int value) : min(static_cast<float>(value)), max(static_cast<float>(value))
{
}

RandomValueBounds::RandomValueBounds(float min, float max) : min(min), max(max)
{
}

RandomValueBounds::RandomValueBounds(float value) : min(value), max(value)
{
}

RandomValueBounds::RandomValueBounds(double value) : min(static_cast<float>(value)), max(static_cast<float>(value))
{
}
