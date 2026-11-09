#ifndef GAME_WORLD_GENERATION_STRUCTUREFREQUENCYANALYSER_HPP
#define GAME_WORLD_GENERATION_STRUCTUREFREQUENCYANALYSER_HPP

#include <vector>

class StructureFrequencyAnalyser
{
public:
    struct Results
    {
        long long seed;
        int woodlandMansions;
        int oceanMonuments;
        int shipwrecks;
    };

    StructureFrequencyAnalyser(int sizeInChunks, bool printResults, bool printAverages);

    void run(unsigned int seedCount);
    void print() const;
    static void runTests();

private:
    int mSizeInChunks;
    int mAreaInChunks;
    bool mPrintResults;
    bool mPrintAverages;
    std::vector<Results> mResults;
};

#endif
