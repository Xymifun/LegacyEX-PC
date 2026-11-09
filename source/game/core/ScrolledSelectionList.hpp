#ifndef GAME_CORE_SCROLLEDSELECTIONLIST_HPP
#define GAME_CORE_SCROLLEDSELECTIONLIST_HPP

#include <vector>

class Button;
class Minecraft;
class Tesselator;

class ScrolledSelectionList
{
public:
    ScrolledSelectionList(Minecraft* minecraft, int width, int height, int top, int bottom, int slotHeight);

    virtual int getNumberOfItems() = 0;
    virtual void selectItem(int index, bool doubleClick) = 0;
    virtual bool isSelectedItem(int index) = 0;
    virtual int getMaxPosition();
    virtual void renderBackground() = 0;
    virtual void renderItem(int index, int x, int y, int height, Tesselator* tesselator) = 0;

    int getItemAtPosition(int x, int y);
    void init(std::vector<Button*>* buttons, int scrollUpButtonId, int scrollDownButtonId);
    void capYPosition();
    void buttonClicked(Button* button);
    void setRenderSelection(bool renderSelection);
    void setRenderHeader(bool renderHeader, int headerHeight);
    void renderHeader(int x, int y, Tesselator* tesselator);
    int clickedHeader(int x, int y);
    void renderDecorations(int x, int y);
    void render(int x, int y, float partialTicks);
    void renderHoleBackground(int x0, int y0, int x1, int y1);

protected:
    Minecraft* minecraft;
    int width;
    int height;
    int top;
    int bottom;
    int right;
    int left;
    int slotHeight;
    int scrollUpButtonId;
    int scrollDownButtonId;
    float initialClickY;
    float scrollMultiplier;
    float amountScrolled;
    int selectedElement;
    long long lastClicked;
    bool renderSelection;
    bool renderHeaderEnabled;
    int headerPadding;
};

#endif
