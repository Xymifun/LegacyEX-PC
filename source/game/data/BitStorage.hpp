#ifndef GAME_DATA_BITSTORAGE_HPP
#define GAME_DATA_BITSTORAGE_HPP

class BitStorage
{
public:
    BitStorage(unsigned int bits, unsigned int size);
    ~BitStorage();

    void set(unsigned int index, unsigned int value);
    unsigned int get(unsigned int index);
    unsigned int* getRaw();
    unsigned int getSize();
    unsigned int getBits();
};

#endif
