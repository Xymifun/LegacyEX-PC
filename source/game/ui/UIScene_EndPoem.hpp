#ifndef GAME_UI_UISCENE_ENDPOEM_HPP
#define GAME_UI_UISCENE_ENDPOEM_HPP

#include <string>
#include <vector>

#include "game/ui/UIScene.hpp"

class UILayer;

class UIScene_EndPoem : public UIScene
{
public:
    UIScene_EndPoem(int type, void* data, UILayer* layer);

    void updateNoise();
    virtual void updateTooltips();
    virtual std::wstring getMoviePath();
    virtual void tick();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleDestroy();
    virtual void handleRequestMoreData(int id, bool forward);
    virtual int getSceneType();

private:
    std::wstring m_poemText;
    std::wstring m_noisyText;
    std::vector<int> m_noiseLengths;
    bool m_finished;
    int m_currentLabel;
    std::vector<std::wstring> m_labels;
};

#endif
