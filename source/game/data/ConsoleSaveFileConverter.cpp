#include "game/data/ConsoleSaveFileConverter.hpp"

#include <string>
#include <vector>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/FileEntry.hpp"
#include "game/core/RegionFile.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/ConsoleSavePath.hpp"
#include "game/network/ProgressListener.hpp"

static const std::wstring PLAYER_DIR = L"players/";

void ConsoleSaveFileConverter::ProcessSimpleFile(ConsoleSaveFile* saveFileIn, FileEntry* entryIn,
                                                 ConsoleSaveFile* saveFileOut, FileEntry* entryOut)
{
    unsigned int bytesRead = 0;
    unsigned int bytesWritten = 0;

    unsigned char* buffer = new unsigned char[entryIn->m_data.m_fileSize];

    saveFileIn->setFilePointer(entryIn, 0, NULL, 0);
    saveFileIn->readFile(entryIn, buffer, entryIn->m_data.m_fileSize, &bytesRead);

    saveFileOut->setFilePointer(entryOut, 0, NULL, 0);
    saveFileOut->writeFile(entryOut, buffer, bytesRead, &bytesWritten);

    delete[] buffer;
}

void ConsoleSaveFileConverter::ProcessStandardRegionFile(ConsoleSaveFile* saveFileIn, File regionFileIn,
                                                         ConsoleSaveFile* saveFileOut, File regionFileOut)
{
    RegionFile sourceRegion(saveFileIn, &regionFileIn);
    RegionFile destRegion(saveFileOut, &regionFileOut);

    bool compressed = true;

    for (int x = 0; x < 32; x++)
    {
        for (int z = 0; z < 32; z++)
        {
            DataInputStream* pChunkIn = sourceRegion.getChunkDataInputStream(x, z, &compressed);

            if (pChunkIn != NULL)
            {
                int nextByte = pChunkIn->read();

                DataOutputStream* pChunkOut = destRegion.getChunkDataOutputStream(x, z, compressed);

                while (nextByte != -1)
                {
                    pChunkOut->write((unsigned char)nextByte);
                    nextByte = pChunkIn->read();
                }

                pChunkOut->close();
                pChunkOut->deleteChildStream();

                delete pChunkOut;
                delete pChunkIn;
            }
        }
    }
}

void ConsoleSaveFileConverter::ConvertSave(ConsoleSaveFile* saveFileIn, ConsoleSaveFile* saveFileOut,
                                           ProgressListener* progressListener)
{
    ConsoleSavePath levelDatPath(L"level.dat");

    FileEntry* pLevelDatIn = saveFileIn->createFile(levelDatPath);
    FileEntry* pLevelDatOut = saveFileOut->createFile(levelDatPath);

    ProcessSimpleFile(saveFileIn, pLevelDatIn, saveFileOut, pLevelDatOut);

    {
        ConsoleSavePath gameRulesPath(L"requiredGameRules.grf");

        if (saveFileIn->doesFileExist(gameRulesPath))
        {
            FileEntry* pGameRulesIn = saveFileIn->createFile(gameRulesPath);
            FileEntry* pGameRulesOut = saveFileOut->createFile(gameRulesPath);

            ProcessSimpleFile(saveFileIn, pGameRulesIn, saveFileOut, pGameRulesOut);
        }
    }

    std::vector<FileEntry*>* pPlayerFiles = saveFileIn->getFilesWithPrefix(std::wstring(PLAYER_DIR));

    if (pPlayerFiles != NULL)
    {
        for (unsigned int i = 0; i < pPlayerFiles->size(); i++)
        {
            ConsoleSavePath playerPathIn(pPlayerFiles->at(i)->m_fileName);
            ConsoleSavePath playerPathOut(pPlayerFiles->at(i)->m_fileName);

            FileEntry* pPlayerIn = saveFileIn->createFile(playerPathIn);
            FileEntry* pPlayerOut = saveFileOut->createFile(playerPathOut);

            ProcessSimpleFile(saveFileIn, pPlayerIn, saveFileOut, pPlayerOut);

            pPlayerOut->m_data.m_timestamp = pPlayerIn->m_data.m_timestamp;
        }

        delete pPlayerFiles;
    }

    std::vector<FileEntry*>* pAllFiles = saveFileIn->getFilesWithPrefix(std::wstring(L""));

    for (std::vector<FileEntry*>::iterator it = pAllFiles->begin(); it < pAllFiles->end(); ++it)
    {
        FileEntry* pEntry = *it;

        if (pEntry == pLevelDatIn)
        {
            continue;
        }

        std::wstring fileName = pEntry->m_fileName;
        std::wstring regionExtension(L".mcr");

        if (fileName.compare(fileName.size() - regionExtension.size(), regionExtension.size(), regionExtension) == 0)
        {
            ProcessStandardRegionFile(saveFileIn, File(pEntry->m_fileName), saveFileOut, File(pEntry->m_fileName));
        }
    }
}
