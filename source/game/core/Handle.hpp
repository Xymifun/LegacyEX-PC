#ifndef GAME_CORE_HANDLE_HPP
#define GAME_CORE_HANDLE_HPP

struct Handle
{
    unsigned int index;
    unsigned int generation;

    Handle() : index(0), generation(0) {}
};

#endif
