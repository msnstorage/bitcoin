// Copyright (c) 2017 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <storage/storagedb.h>
#include "storageman.h"

static const char DB_FLAG = 'F';

CStorageHeadersDB::CStorageHeadersDB(size_t nCacheSize, bool fMemory, bool fWipe) : CDBWrapper(GetStorageDir() / "headers", nCacheSize, fMemory, fWipe) {}

bool CStorageHeadersDB::WriteHead(const std::pair<uint256, uint256> hashpair, const CStorageHead& headFile, const bool& status)
{
    return Write(hashpair, std::make_pair(headFile, status));
}

bool CStorageHeadersDB::ReadHead(const std::pair<uint256, uint256> hashpair, std::pair<CStorageHead , bool> &pair)
{
    return Read(hashpair, pair);
}

bool CStorageHeadersDB::HeadExists(const std::pair<uint256, uint256> hashpair)
{
    return Exists(hashpair);
}

bool CStorageHeadersDB::HeadErase(const std::pair<uint256, uint256> hashpair)
{
    return Erase(hashpair);
}

bool CStorageHeadersDB::LoadHeaders()
{
    CDBIterator* pcursor(NewIterator());
    pcursor->Seek(std::pair<uint256, uint256>());
    for (pcursor->SeekToFirst(); pcursor->Valid(); pcursor->Next()) {
        std::pair<uint256, uint256> key;
        if (pcursor->GetKey(key)) {
            std::pair<CStorageHead , bool> value;
            if(pcursor->GetValue(value)) {
                if (storageman.HeadersSize()<500 && !value.second) {
                    storageman.AddHeader(value.first, value.second);
                }
            }
        }
    }
    delete pcursor;
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
    CDBIterator* pcursor(NewIterator());
    pcursor->Seek(uint256());
    for (pcursor->SeekToFirst(); pcursor->Valid(); pcursor->Next()) {
        uint256 key;
        if (pcursor->GetKey(key)) {
            std::pair<CHeadFile , bool> value;
            if(pcursor->GetValue(value)) {
                if (storageman.HeadersFilesSize()<500 && !value.second) {
                    storageman.AddHeaderFiles(value.first, value.second);
                }
            }
        }
    }
    delete pcursor;
    return true;
}

CStorageFilesPartsDB::CStorageFilesPartsDB(size_t nCacheSize, bool fMemory, bool fWipe) : CDBWrapper(GetStorageDir() / "filesparts", nCacheSize, fMemory, fWipe) {}

uint32_t CStorageFilesPartsDB::GetSize()
{
    uint32_t size = 0;
    std::string name = "size";
    if (Read(std::make_pair(DB_FLAG, name), size))
        return size;
    return size;
}

void CStorageFilesPartsDB::UpdateSize(uint32_t newsize) {
    uint32_t size = 0;
    std::string name = "size";
    if (Read(std::make_pair(DB_FLAG, name), size)) {
        size = size + newsize;
    } else {
        size = size + newsize;
    }
    WriteSize(size);
}

bool CStorageFilesPartsDB::WriteSize(const uint32_t size) {
    std::string name = "size";
    return Write(std::make_pair(DB_FLAG, name), size);
}

bool CStorageFilesPartsDB::WriteFilesParts(const std::pair<std::pair<uint256, uint256>, uint256>& hashpair, const std::pair<std::vector<unsigned char>, const bool>& data)
{
    return Write(hashpair, data);
}

bool CStorageFilesPartsDB::ReadFilesParts(const std::pair<std::pair<uint256, uint256>, uint256> hashpair, std::pair<std::vector<unsigned char>, bool> &data)
{
    return Read(hashpair, data);
}

bool CStorageFilesPartsDB::FilesPartsExists(const std::pair<std::pair<uint256, uint256>, uint256> hashpair)
{
    return Exists(hashpair);
}

bool CStorageFilesPartsDB::FilesPartsErase(const std::pair<std::pair<uint256, uint256>, uint256> hashpair)
{
    return Erase(hashpair);
}

bool CStorageFilesPartsDB::LoadFilesParts()
{
    CDBIterator* pcursor(NewIterator());
    pcursor->Seek(std::make_pair(std::make_pair(uint256(), uint256()), uint256()));
    for (pcursor->SeekToFirst(); pcursor->Valid(); pcursor->Next()) {
        std::pair<std::pair<uint256, uint256>, uint256> key;
        if (pcursor->GetKey(key)) {
            std::pair<std::vector<unsigned char>, bool> value;
            if(pcursor->GetValue(value)) {
                LogPrintf("CStorageDB::LoadFilesParts parthash=%s status=%u\n", key.second.ToString(), value.second);
                if (storageman.FilesPartsSize()<500 && !value.second) {
                    storageman.AddFilesParts(key, value);
                }
            }
        }
    }
    delete pcursor;
    return true;
}