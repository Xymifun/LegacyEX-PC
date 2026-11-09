#ifndef GAME_CORE_INTERACTIONRESULT_HPP
#define GAME_CORE_INTERACTIONRESULT_HPP

class InteractionResult
{
public:
    enum EInteractionResult
    {
        eInteractionResult_Success = 0,
        eInteractionResult_Pass = 1,
        eInteractionResult_Fail = 2
    };
};

#endif
