#ifndef GAME_CORE_STBIFILE_HPP
#define GAME_CORE_STBIFILE_HPP

class StbiFile
{
public:
    static int read(void* user, char* data, int size);
    static void skip(void* user, int count);
    static int eof(void* user);

    int mFile;
    int mFileSize;
    int mFilePosition;
};

#endif
