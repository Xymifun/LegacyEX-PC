#ifndef GAME_UI_FJ_FACTORY_HPP
#define GAME_UI_FJ_FACTORY_HPP

class FJ_FuiNode;
class fuiRenderNode;

class FJ_Factory
{
public:
    static void initialise();
    static FJ_FuiNode* createObject(const char* symbol, fuiRenderNode* parent, bool createFromSymbol,
                                    const char* className);
};

#endif
