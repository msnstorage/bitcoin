// Copyright (c) 2017 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <storage/storagedb.h>
#include "storageman.h"

CStorageHeadersDB::CStorageHeadersDB(size_t nCacheSize, bool fMemory, bool fWipe) : CDBWrapper(GetStorageDir() / "headers", nCacheSize, fMemory, fWipe) {}

bool CStorageHeadersDB::WriteHead(const uint256 hash, const CStorageHead& headFile, const bool& status)
{
    return Write(hash, std::make_pair(headFile, status));
}

bool CStorageHeadersDB::ReadHead(const uint256 hash, std::pair<CStorageHead , bool> &pair)
{
    return Read(hash, pair);
}

bool CStorageHeadersDB::HeadExists(const uint256 hash)
{
    return Exists(hash);
}

bool CStorageHeadersDB::HeadErase(const uint256 hash)
{
    return Erase(hash);
}

bool CStorageHeadersDB::LoadHeaders()
{
    std::unique_ptr<CDBIterator> pcursor(NewIterator());

    pcursor->Seek(uint256());
    // Load mapBlockIndex
    while (pcursor->Valid()) {
        uint256 key;
        if (pcursor->GetKey(key)) {
            std::pair<CStorageHead , bool> value;
            if (pcursor->GetValue(value)) {
                if (storageman.HeadersSize()<500 && !value.second) {
                    storageman.AddHeader(value.first, value.second);
                }
                pcursor->Next();
            }
        } else {
            break;
        }
    }
    return true;
}

CStorageHeadersFilesDB::CStorageHeadersFilesDB(size_t nCacheSize, bool fMemory, bool fWipe) : CDBWrapper(GetStorageDir() / "headersfiles", nCacheSize, fMemory, fWipe) {}

bool CStorageHeadersFilesDB::WriteHeadFiles(const uint256 hash, const CHeadFile& headFile, const bool& status)
{
    return Write(hash, std::make_pair(headFile, status));
}

bool CStorageHeadersFilesDB::ReadHeadFiles(const uint256 hash, std::pair<CHeadFile , bool> &pair)
{
    return Read(hash, pair);
}

bool CStorageHeadersFilesDB::HeadFilesExists(const uint256 hash)
{
    return Exists(hash);
}

bool CStorageHeadersFilesDB::HeadFilesErase(const uint256 hash)
{
    return Erase(hash);
}

bool CStorageHeadersFilesDB::LoadHeadersFiles()
{
    std::unique_ptr<CDBIterator> pcursor(NewIterator());

    pcursor->Seek(uint256());
    // Load mapBlockIndex
    while (pcursor->Valid()) {
        uint256 key;
        if (pcursor->GetKey(key)) {
            std::pair<CHeadFile , bool> value;
            if (pcursor->GetValue(value)) {
                if (storageman.HeadersFilesSize()<500 && !value.second) {
                    storageman.AddHeaderFiles(value.first, value.second);
                }
                pcursor->Next();
            }
        } else {
            break;
        }
    }
    return true;
}

CStorageFilesPartsDB::CStorageFilesPartsDB(size_t nCacheSize, bool fMemory, bool fWipe) : CDBWrapper(GetStorageDir() / "filesparts", nCacheSize, fMemory, fWipe) {}

bool CStorageFilesPartsDB::WriteFilesParts(const std::pair<uint256, uint256>& hashpair, const std::pair<std::vector<unsigned char>, const bool>& data)
{
    return Write(hashpair, data);
}

bool CStorageFilesPartsDB::ReadFilesParts(const std::pair<uint256, uint256> hashpair, std::pair<std::vector<unsigned char>, bool> &data)
{
    return Read(hashpair, data);
}

bool CStorageFilesPartsDB::FilesPartsExists(const std::pair<uint256, uint256> hashpair)
{
    return Exists(hashpair);
}

bool CStorageFilesPartsDB::FilesPartsErase(const std::pair<uint256, uint256> hashpair)
{
    return Erase(hashpair);
}

bool CStorageFilesPartsDB::LoadFilesParts()
{
    std::unique_ptr<CDBIterator> pcursor(NewIterator());

    pcursor->Seek(std::make_pair(uint256(), uint256()));
    // Load mapBlockIndex
    while (pcursor->Valid()) {
        std::pair<uint256, uint256> key;
        if (pcursor->GetKey(key)) {
            std::pair<std::vector<unsigned char>, bool> value;
            if (pcursor->GetValue(value)) {
                if (storageman.FilesPartsSize()<500 && !value.second) {
                    storageman.AddFilesParts(key, value);
                }
                pcursor->Next();
            }
        } else {
            break;
        }
    }
    return true;
}