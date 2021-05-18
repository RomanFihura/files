#pragma once

#include "files.h"

#include <fstream>
#include <string>

class FileStringList : public StringList
{
public:
    FileStringList(const std::string &filepath);

    ~FileStringList();

    bool insert(const std::string &s, uint32_t pos) override;

    bool remove(uint32_t pos) override;

    const std::string string(uint32_t pos) override;

private:
    struct Header
    {
        uint32_t size = 0;
        uint32_t deletedHeadOffset = 0;
        uint32_t existedHeadOffset = 0;
    } mHeader;

    struct RecordMeta
    {
        uint32_t capacity;
        uint32_t nextOffset;
    };

    std::fstream mDataFile;

    bool createNew(const std::string &filepath);

    uint32_t getOffsetToReuse(uint32_t size);

    uint32_t addRecord(const std::string str);
};
