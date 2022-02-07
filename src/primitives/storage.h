// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef STORAGE_H
#define STORAGE_H

#include <hash.h>
#include <serialize.h>
#include <uint256.h>
#include <util/strencodings.h>

/** An head of a storage.
 */
class CStorageHead
{
public:
    uint256 hash;
    uint32_t size;

    static constexpr uint32_t NULL_INDEX = std::numeric_limits<uint32_t>::max();

    CStorageHead(): size(NULL_INDEX) { }
    CStorageHead(const uint256& hashIn, uint32_t sizeIn): hash(hashIn), size(sizeIn) { }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(hash);
        READWRITE(size);
    }

    void SetNull() { hash.SetNull(); size = NULL_INDEX; }
    bool IsNull() const { return (hash.IsNull() && size == NULL_INDEX); }

    friend bool operator==(const CStorageHead& a, const CStorageHead& b)
    {
        return (a.hash == b.hash && a.size == b.size);
    }

    friend bool operator!=(const CStorageHead& a, const CStorageHead& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An storage of a transaction.
 */
class CTxStorage
{
public:
    std::string sName;
    std::vector<CStorageHead> vhead;

    CTxStorage()
    {
        SetNull();
    }


    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(sName);
        READWRITE(vhead);
    }

    void SetNull()
    {
        sName = "";
    }

    bool IsNull() const
    {
        return (sName.empty() && vhead.empty());
    }

    friend bool operator==(const CTxStorage& a, const CTxStorage& b)
    {
        return (a.sName == b.sName && a.vhead == b.vhead);
    }

    friend bool operator!=(const CTxStorage& a, const CTxStorage& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An head of a storage.
 */
class CHeadFilePart
{
public:
    uint256 hash;
    uint32_t part_begin;
    uint32_t part_end;

    static constexpr uint32_t NULL_INDEX = std::numeric_limits<uint32_t>::max();

    CHeadFilePart(): part_begin(NULL_INDEX), part_end(NULL_INDEX) { }
    CHeadFilePart(const uint256& hashIn, uint32_t part_beginIn, uint32_t part_endIn): hash(hashIn), part_begin(part_beginIn), part_end(part_endIn) { }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(hash);
        READWRITE(part_begin);
        READWRITE(part_end);
    }

    void SetNull() { hash.SetNull(); part_begin = NULL_INDEX; part_end = NULL_INDEX;}
    bool IsNull() const { return (hash.IsNull() && part_begin == NULL_INDEX && part_end == NULL_INDEX); }

    friend bool operator==(const CHeadFilePart& a, const CHeadFilePart& b)
    {
        return (a.hash == b.hash && a.part_begin == b.part_begin && a.part_end == b.part_end);
    }

    friend bool operator!=(const CHeadFilePart& a, const CHeadFilePart& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An head of a storage.
 */
class CHeadFilePartL
{
public:
    uint256 hash;
    uint32_t part_begin;
    uint32_t part_end;

    //memory onjy
    bool loaded;
    int64_t lasttime;
    uint256 filehash;

    static constexpr uint32_t NULL_INDEX = std::numeric_limits<uint32_t>::max();

    CHeadFilePartL(): part_begin(NULL_INDEX), part_end(NULL_INDEX), loaded(false), lasttime(0), filehash(uint256S("")) { }
    CHeadFilePartL(const uint256& hashIn, uint32_t part_beginIn, uint32_t part_endIn): hash(hashIn), part_begin(part_beginIn), part_end(part_endIn) { }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(hash);
        READWRITE(part_begin);
        READWRITE(part_end);
        READWRITE(filehash);
    }

    void SetNull() { hash.SetNull(); part_begin = NULL_INDEX; part_end = NULL_INDEX; loaded = false; lasttime =0; filehash.SetNull();}
    bool IsNull() const { return (hash.IsNull() && part_begin == NULL_INDEX && part_end == NULL_INDEX); }

    friend bool operator==(const CHeadFilePartL& a, const CHeadFilePartL& b)
    {
        return (a.hash == b.hash && a.part_begin == b.part_begin && a.part_end == b.part_end && a.filehash == b.filehash);
    }

    friend bool operator!=(const CHeadFilePartL& a, const CHeadFilePartL& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An head of a storage.
 */
class CHeadFile
{
public:
    uint256 hash;
    uint32_t parts;
    std::vector<CHeadFilePart> vhead;

    static constexpr uint32_t NULL_INDEX = std::numeric_limits<uint32_t>::max();

    CHeadFile(): parts(NULL_INDEX) { }
    CHeadFile(const uint256& hashIn, uint32_t partsIn, std::vector<CHeadFilePart> vheadIn): hash(hashIn), parts(partsIn), vhead(vheadIn) { }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(hash);
        READWRITE(parts);
        READWRITE(vhead);
    }

    void SetNull() { hash.SetNull(); parts = NULL_INDEX; }
    bool IsNull() const { return (hash.IsNull() && parts == NULL_INDEX); }

    friend bool operator==(const CHeadFile& a, const CHeadFile& b)
    {
        return (a.hash == b.hash && a.parts == b.parts);
    }

    friend bool operator!=(const CHeadFile& a, const CHeadFile& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An head of a storage.
 */
class CHeadFileStatus
{
public:
    uint256 hash;
    uint32_t status;

    static constexpr uint32_t NULL_INDEX = std::numeric_limits<uint32_t>::max();

    CHeadFileStatus(): status(NULL_INDEX) { }
    CHeadFileStatus(const uint256& hashIn, uint32_t statusIn): hash(hashIn), status(statusIn) { }


    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(hash);
        READWRITE(status);
    }

    void SetNull() { hash.SetNull();}
    bool IsNull() const { return (hash.IsNull()); }

    friend bool operator==(const CHeadFileStatus& a, const CHeadFileStatus& b)
    {
        return (a.hash == b.hash && a.status == b.status);
    }

    friend bool operator!=(const CHeadFileStatus& a, const CHeadFileStatus& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An head of a storage.
 */
class CFH
{
public:
    uint256 hash;
    uint256 filehash;
    std::vector<unsigned char> data;

    static constexpr uint32_t NULL_INDEX = std::numeric_limits<uint32_t>::max();

    CFH(): hash(uint256S("")), filehash(uint256S("")) { }
    CFH(const uint256& hashIn, const uint256& filehashIn, std::vector<unsigned char> dataIn): hash(hashIn), filehash(filehashIn), data(dataIn) { }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(hash);
        READWRITE(filehash);
        READWRITE(data);
    }

    void SetNull() { hash.SetNull(); filehash.SetNull(); data.clear(); }
    bool IsNull() const { return (hash.IsNull() && filehash.IsNull() && data.size() == 0); }

    friend bool operator==(const CFH& a, const CFH& b)
    {
        return (a.hash == b.hash && a.filehash == b.filehash && a.data == b.data);
    }

    friend bool operator!=(const CFH& a, const CFH& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An head of a storage.
 */
class CFP
{
public:
    uint256 hash;
    uint256 filehash;
    uint32_t part_begin;
    uint32_t part_end;
    std::vector<unsigned char> data;

    static constexpr uint32_t NULL_INDEX = std::numeric_limits<uint32_t>::max();

    CFP(): hash(uint256S("")) { }
    CFP(const uint256& hashIn, const uint256& filehashIn, uint32_t part_beginIn, uint32_t part_endIn, std::vector<unsigned char> dataIn): hash(hashIn), filehash(filehashIn), part_begin(part_beginIn), part_end(part_endIn), data(dataIn) { }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(hash);
        READWRITE(filehash);
        READWRITE(part_begin);
        READWRITE(part_end);
        READWRITE(data);
    }

    void SetNull() { hash.SetNull(); filehash.SetNull(); part_begin = 0; part_end = 0; data.clear(); }
    bool IsNull() const { return (hash.IsNull() && filehash.IsNull() && part_begin == 0 && part_end == 0 && data.size() == 0); }

    friend bool operator==(const CFP& a, const CFP& b)
    {
        return (a.hash == b.hash && a.filehash == b.filehash && a.part_begin == b.part_begin && a.part_end == b.part_end && a.data == b.data);
    }

    friend bool operator!=(const CFP& a, const CFP& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};


#endif // STORAGE_H