#ifndef GAME_CORE_TICKACTION_HPP
#define GAME_CORE_TICKACTION_HPP

class TickAction
{
public:
    virtual ~TickAction() {}

    virtual void Execute() = 0;
};

#endif
