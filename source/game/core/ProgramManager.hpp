#ifndef GAME_CORE_PROGRAMMANAGER_HPP
#define GAME_CORE_PROGRAMMANAGER_HPP

class Effect;

class ProgramManager
{
public:
    static void createInstance();
    static ProgramManager* getInstance();
    void releaseProgram(Effect* effect);
    void createProgram();
    void linkProgram(Effect* effect);
};

#endif
