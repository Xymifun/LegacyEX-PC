#ifndef GAME_ITEM_WEIGHEDRANDOMITEM_HPP
#define GAME_ITEM_WEIGHEDRANDOMITEM_HPP

class WeighedRandomItem
{
public:
    WeighedRandomItem(int weight) : weight(weight) {}

    virtual ~WeighedRandomItem() {}

    int weight;
};

#endif
