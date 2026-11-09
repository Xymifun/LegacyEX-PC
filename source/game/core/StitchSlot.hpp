#ifndef GAME_CORE_STITCHSLOT_HPP
#define GAME_CORE_STITCHSLOT_HPP

#include <string>
#include <vector>

class TextureHolder;

class StitchSlot
{
public:
    StitchSlot(int x, int y, int width, int height);

    TextureHolder* getHolder();
    int getX();
    int getY();
    bool add(TextureHolder* holder);
    void collectAssignments(std::vector<StitchSlot*>* assignments);
    std::wstring toString();
};

#endif
