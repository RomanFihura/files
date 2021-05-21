#include "filestring.h"

#include <filesystem>
#include <memory> //unique_ptr

constexpr uint32_t headerOffset = 0;

namespace fs = std::filesystem;
// data - pointer to memory of object, len - object size in bytes
static bool writeToFile(std::fstream &file, uint32_t offset, const char *data, uint32_t len)
{
    file.seekp(offset);
    file.write(data, len);
    return file.good(); // check after write
}

static bool readFromFile(std::fstream &file, uint32_t offset, char *data, uint32_t len)
{
    file.seekg(offset);
    file.read(data, len);
    return file.good();
}

FileStringList::FileStringList(const std::string &filepath)
{
    if (!fs::exists(fs::path(filepath)))
    {
        createNew(filepath);
    }

    mDataFile.open(filepath, std::ios::binary | std::ios::in | std::ios::out);
    if (mDataFile) // reading header
    {
        readFromFile(mDataFile, headerOffset, (char *)&mHeader, sizeof(mHeader));
    }
}

FileStringList::~FileStringList()
{
    writeToFile(mDataFile, headerOffset, (const char *)&mHeader, sizeof(mHeader));
}

bool FileStringList::insert(const std::string &s, uint32_t pos)
{
    uint32_t curPos = 0;
    uint32_t prevOffset = 0;
    uint32_t curOffset = mHeader.existedHeadOffset;

    RecordMeta prev;
    RecordMeta cur;
    RecordMeta newMeta;

    while ((curPos < pos) && curOffset)
    {
        readFromFile(mDataFile, curOffset, (char *)&cur, sizeof(cur));
        prev = cur;
        prevOffset = curOffset;
        curOffset = cur.nextOffset;

        ++curPos;
    }

    const auto newOffset = addRecord(s);
    if (!newOffset)
        return false;

    readFromFile(mDataFile, newOffset, (char *)&newMeta, sizeof(newMeta));

    if (prevOffset) // if not first
    {
        prev.nextOffset = newOffset;
        writeToFile(mDataFile, prevOffset, (const char *)&prev, sizeof(prev));
    }

    if (curOffset) // if not last
    {
        newMeta.nextOffset = curOffset;
    }

    if (pos == 0) // if first
    {
        newMeta.nextOffset = mHeader.existedHeadOffset;
        mHeader.existedHeadOffset = newOffset;
    }

    writeToFile(mDataFile, newOffset, (const char *)&newMeta, sizeof(newMeta));

    return mDataFile.good();
}

bool FileStringList::remove(uint32_t pos)
{
    if (mHeader.size <= pos)
        return false;

    uint32_t curPos = 0;

    uint32_t prevOffset = 0;
    uint32_t curOffset = mHeader.existedHeadOffset;

    RecordMeta prevRecordMeta;
    RecordMeta curRecordMeta;

    while (true)
    {
        readFromFile(mDataFile, curOffset, (char *)&curRecordMeta, sizeof(curRecordMeta));

        if (curPos == pos) // if we reached our string
            break;

        prevRecordMeta = curRecordMeta;
        prevOffset = curOffset;
        curOffset = curRecordMeta.nextOffset;
        curPos++;
    }

    if (prevOffset) // if not first
    {
        prevRecordMeta.nextOffset = curRecordMeta.nextOffset;
        writeToFile(mDataFile, prevOffset, (const char *)&prevRecordMeta, sizeof(prevRecordMeta));
    }
    else
    {
        mHeader.existedHeadOffset = curRecordMeta.nextOffset;
    }

    curRecordMeta.nextOffset = mHeader.deletedHeadOffset;
    mHeader.deletedHeadOffset = curOffset;

    writeToFile(mDataFile, curOffset, (const char *)&curRecordMeta, sizeof(curRecordMeta));

    mHeader.size--;

    return true;
}

const std::string FileStringList::string(uint32_t pos)
{
    if (pos >= mHeader.size)
        return "";

    uint32_t curOffset = 0;
    uint32_t nextOffset = mHeader.existedHeadOffset;

    RecordMeta meta;
    for (int i = 0; i <= pos; i++)
    {
        readFromFile(mDataFile, nextOffset, (char *)&meta, sizeof(meta));
        curOffset = nextOffset;
        nextOffset = meta.nextOffset;
    }

    std::unique_ptr<char[]> buff{new char[meta.capacity]};
    readFromFile(mDataFile, curOffset + sizeof(RecordMeta), buff.get(), meta.capacity);

    return std::string{buff.get()};
}

bool FileStringList::createNew(const std::string &filepath)
{
    std::ofstream file{filepath, std::ios::binary | std::ios::trunc};

    file.write((const char *)&mHeader, sizeof(mHeader));

    return file.good();
}

uint32_t FileStringList::getOffsetToReuse(uint32_t strSize)
{
    uint32_t prevOffset = 0;
    uint32_t nextOffset = mHeader.deletedHeadOffset;

    RecordMeta recordMeta;
    RecordMeta prevRecordMeta;

    while (nextOffset)
    {
        readFromFile(mDataFile, nextOffset, (char *)&recordMeta, sizeof(recordMeta));

        if (recordMeta.capacity <= strSize)
        {
            prevRecordMeta = recordMeta;
            prevOffset = nextOffset;
            nextOffset = recordMeta.nextOffset;
            continue;
        }
        else if (prevOffset) // found place to reuse (not first)
        {
            prevRecordMeta.nextOffset = recordMeta.nextOffset;
            writeToFile(mDataFile, prevOffset, (const char *)&prevRecordMeta,
                        sizeof(prevRecordMeta));
            return nextOffset;
        }
        else // found place to reuse (first)
        {
            mHeader.deletedHeadOffset = recordMeta.nextOffset;
            return nextOffset;
        }
    }

    return 0;
}

uint32_t FileStringList::addRecord(const std::string str)
{
    uint32_t offset = getOffsetToReuse(str.size());

    if (!offset) // not found offset to reuse
    {
        mDataFile.seekp(0, std::ios::end);
        offset = mDataFile.tellp();

        RecordMeta newMeta{str.size() + 1, 0};

        writeToFile(mDataFile, offset, (const char *)&newMeta, sizeof(newMeta));
    }

    writeToFile(mDataFile, offset + sizeof(RecordMeta), str.c_str(), str.size() + 1);
    mHeader.size++;

    return mDataFile.good() ? offset : 0;
}