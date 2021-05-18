#include "files.h"
#include "filestring.h"
#include <filesystem>
#include <gtest/gtest.h>

TEST(FileTest, insert)
{
    std::filesystem::remove("file");

    FileStringList f("file");
    std::string s1("AAA");
    std::string s2("BBB");
    std::string s3("CCC");
    std::string s4("DDD");

    f.insert(s1, 0);
    f.insert(s2, 1);
    f.insert(s3, 2);
    f.insert(s4, 5); // AAA BBB CCC DDD

    EXPECT_EQ(s1, f.string(0));
    EXPECT_EQ(s2, f.string(1));
    EXPECT_EQ(s3, f.string(2));
    EXPECT_EQ(s4, f.string(3));
    EXPECT_EQ("", f.string(6));
}
TEST(File, savetest)
{
    std::filesystem::remove("file");
    {
        FileStringList f("file");

        f.insert("Hello", 0);
        f.insert("LONDON", 1);
    }
    {
        FileStringList f("file");

        f.insert("hehe", 2);
        EXPECT_EQ("Hello", f.string(0));
        EXPECT_EQ("LONDON", f.string(1));
        EXPECT_EQ("hehe", f.string(2));
    }
}
TEST(File, remove)
{
    std::filesystem::remove("file");

    FileStringList f("file");

    f.insert("TEST", 0);
    f.insert("remove", 1);
    f.remove(1);
    EXPECT_EQ("TEST", f.string(0));
    EXPECT_EQ("", f.string(1));
}