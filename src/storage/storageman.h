// Copyright (c) 2014-2017 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef STORAGEMAN_H
#define STORAGEMAN_H

#include <primitives/transaction.h>
#include <sync.h>
#include <net.h>

using namespace std;

class CStorageHeadersDB;
class CStorageHeadersFilesDB;
class CStorageFilesPartsDB;

/** Global variable that points to the storage database (protected by cs_main) */
extern std::unique_ptr<CStorageHeadersDB> pStorageHeadersDB;
extern std::unique_ptr<CStorageHeadersFilesDB> pStorageHeadersFilesDB;
extern std::unique_ptr<CStorageFilesPartsDB> pStorageFilesPartsDB;

class CStorageMan;

extern CStorageMan storageman;

class CStorageMan
{
public:

private:
    // critical section to protect the inner data structures
    mutable CCriticalSection cs;
    // map to hold all MNs
    std::map<uint256, std::pair<CStorageHead , bool>> mapHeaders;
    std::map<uint256, std::pair<CHeadFile , bool>> mapHeadersFiles;
    std::map<std::pair<uint256, uint256>, std::pair<std::vector<unsigned char>, bool>> mapFilesParts;

public:

    CStorageMan();

    /// Add an entry
    bool AddHeader(CStorageHead hf, bool status);
    bool AddHeaderFiles(CHeadFile hf, bool status);
    bool AddFilesParts(std::pair<uint256, uint256> hashpair, std::pair<std::vector<unsigned char>, bool> data);

    /// Check all Masternodes and remove inactive
    void CheckAndRemove(CConnman& connman);

    void ProcessMessage(CNode* pfrom, const std::string& strCommand, CDataStream& vRecv, CConnman& connman);
    void ProcesseTxStorage(const CTransaction& tx);

    void LoadHeaders();
    void LoadHeadersFiles();
    void LoadFilesParts();

    /// Return the number of (unique) Storages headers
    int HeadersSize() { return mapHeaders.size(); }
    /// Return the number of (unique) Storages headers files
    int HeadersFilesSize() { return mapHeadersFiles.size(); }
    /// Return the number of (unique) Storages files parts
    int FilesPartsSize() { return mapFilesParts.size(); }
};

void ThreadCheckStorage(CConnman& connman);

#endif // STORAGEMAN_H
