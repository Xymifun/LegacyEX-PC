#ifndef GAME_UI_FJ_HELPERS_HPP
#define GAME_UI_FJ_HELPERS_HPP

class FJ_Helpers
{
public:
    FJ_Helpers();

    static FJ_Helpers* RetInstance();

    float nextRandomNumber();
    int nextInt(int maxValue);
    void SetRndSeed(int seed);

private:
    int mRndSeed;
};

#endif
