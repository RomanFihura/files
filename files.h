#pragma once

#include <string>

class StringList
{
public:
    virtual ~StringList() = default;

    /**
     * Inserts a string into provided position.
     *
     * @param s String to insert.
     * @param pos Index of the new string in the list. If this parameter is more or equal to the
     *            list size it appends string to the end.
     * @return true If string has been inserted successfully.
     **/
    virtual bool insert(const std::string &s, uint32_t pos) = 0;

    /**
     * Removes a string with provided index.
     *
     * @param pos Index of string to remove.
     * @return true If index is valid and string has been removed successfully.
     **/
    virtual bool remove(uint32_t pos) = 0;

    /**
     * Obtians a string reference from the list.
     *
     * @param pos Index of the string in the list.
     * @return String value of the element at the provided index if it less then list size, empty
     *         string otherwise.
     **/
    virtual const std::string string(uint32_t pos) = 0;
};