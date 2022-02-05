// Copyright (c) 2014-2017 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef STORAGE_SYNC_H
#define STORAGE_SYNC_H

#include <net.h>
#include <primitives/transaction.h>
#include <serialize.h>

class CStorageSync;

extern CStorageSync storageSync;

//
// CStorageSync : Sync storage assets in stages
//

class CStorageSync
{
private:
    std::map<uint256, int64_t> mapStorageHeaders;
    std::map<uint256, std::vector<CHeadFilePartL>> mapFilesPartsForDownoads;
public:
    void ProcessMessage(CNode* pfrom, const std::string& strCommand, CDataStream& vRecv, CConnman& connman);
    void ProcessTick(CConnman& connman);
    void ProcessHeaders(const CTransaction& tx);
};

#endif // STORAGE_SYNC_H
