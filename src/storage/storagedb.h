// Copyright (c) 2017 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef STORAGEDB_H
#define STORAGEDB_H

#include <dbwrapper.h>
#include <uint256.h>
#include <storage/storage.h>

#include <boost/filesystem/path.hpp>

class CStorageHeadersDB : public CDBWrapper
{
public:
    CStorageHeadersDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);

private:
    CStorageHeadersDB(const CStorageHeadersDB&);
    void operator=(const CStorageHeadersDB&);

public:
    bool WriteHead(const uint256 hash, const CStorageHead& headFile, const bool& status);
    bool ReadHead(const uint256 hash, std::pair<CStorageHead , bool> &pair);
    bool HeadExists(const uint256 hash);
    bool HeadErase(const uint256 hash);
    bool LoadHeaders();
};

class CStorageHeadersFilesDB : public CDBWrapper
{
public:
    CStorageHeadersFilesDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);

private:
    CStorageHeadersFilesDB(const CStorageHeadersFilesDB&);
    void operator=(const CStorageHeadersFilesDB&);

public:
    bool WriteHeadFiles(const uint256 hash, const CHeadFile& headFile, const bool& status);
    bool ReadHeadFiles(const uint256 hash, std::pair<CHeadFile , bool> &pair);
    bool HeadFilesExists(const uint256 hash);
    bool HeadFilesErase(const uint256 hash);
    bool LoadHeadersFiles();
};

class CStorageFilesPartsDB : public CDBWrapper
{
public:
    CStorageFilesPartsDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);

private:
    CStorageFilesPartsDB(const CStorageFilesPartsDB&);
    void operator=(const CStorageFilesPartsDB&);

public:
    bool WriteFilesParts(const std::pair<uint256, uint256>  &hashpair, const std::pair<std::vector<unsigned char>, const bool> &data);
    bool ReadFilesParts(const std::pair<uint256, uint256> hashpair, std::pair<std::vector<unsigned char>, bool> &data);
    bool FilesPartsExists(const std::pair<uint256, uint256> hashpair);
    bool FilesPartsErase(const std::pair<uint256, uint256> hashpair);
    bool LoadFilesParts();
};


#endif // STORAGEDB_H
