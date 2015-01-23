/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
 *
 *	Foobar is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Foobar is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <gtest/gtest.h>

#include <3dlite/3dlite_resource_pack.h>
#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_logger.h>

static void TestCommon(lite3d_resource_pack *pack)
{
    lite3d_resource_file *resource1 = lite3d_resource_pack_file_load(pack,
        "pack/eev.jpg");

    ASSERT_TRUE(resource1 != NULL);
    EXPECT_TRUE(resource1->isLoaded == 1);
    EXPECT_TRUE(resource1->fileBuff != NULL);
    EXPECT_EQ(resource1->fileSize, 89149);

    lite3d_resource_file *resource2 = lite3d_resource_pack_file_load(pack,
        "pack/normandy/ref.jpg");

    ASSERT_TRUE(resource2 != NULL);
    EXPECT_TRUE(resource2->isLoaded == 1);
    EXPECT_TRUE(resource2->fileBuff != NULL);
    EXPECT_EQ(resource2->fileSize, 229837);

    EXPECT_EQ(pack->memoryUsed, 89149 + 229837);

    lite3d_resource_pack_file_purge(resource1);

    EXPECT_TRUE(resource1->isLoaded == 0);
    EXPECT_TRUE(resource1->fileBuff == NULL);
    EXPECT_EQ(resource1->fileSize, 0);
    EXPECT_EQ(pack->memoryUsed, 229837);

    lite3d_resource_file *resource3 = lite3d_resource_pack_file_load(pack,
        "pack/pack.0");

    ASSERT_TRUE(resource3 != NULL);
    EXPECT_TRUE(resource3->isLoaded == 1);
    EXPECT_TRUE(resource3->fileBuff != NULL);
    EXPECT_EQ(resource3->fileSize, 380065);

    lite3d_resource_file *resource4 = lite3d_resource_pack_file_load(pack,
        "pack/normandy/ref.jpg");

    ASSERT_TRUE(resource4 != NULL);
    EXPECT_TRUE(resource4->isLoaded == 1);
    EXPECT_TRUE(resource4->fileBuff != NULL);
    EXPECT_EQ(resource4->fileSize, 229837);
    EXPECT_EQ((size_t) resource2, (size_t) resource4);
    EXPECT_EQ(pack->memoryUsed, 380065 + 229837);

    /* memory limit reached in this point */
    /* expecting purged tail resource */
    lite3d_resource_file *resource5 = lite3d_resource_pack_file_load(pack,
        "pack/normandy/t1.jpg");

    ASSERT_TRUE(resource5 != NULL);
    EXPECT_TRUE(resource5->isLoaded == 1);
    EXPECT_TRUE(resource5->fileBuff != NULL);
    EXPECT_EQ(resource5->fileSize, 122167);
    EXPECT_EQ(pack->memoryUsed, 229837 + 122167);
}

static void TestPerfomanceIndex(lite3d_resource_pack *pack)
{
    for(int i = 0; i < 10000; i++)
    {
        lite3d_resource_file *resource = lite3d_resource_pack_file_load(pack,
            "pack/pack.0");
    
        ASSERT_TRUE(resource != NULL);
        EXPECT_TRUE(resource->isLoaded == 1);
        EXPECT_TRUE(resource->fileBuff != NULL);
        EXPECT_EQ(resource->fileSize, 380065);
    
        lite3d_resource_file *resource2 = lite3d_resource_pack_file_load(pack,
            "pack/normandy/ref.jpg");
    
        ASSERT_TRUE(resource2 != NULL);
        EXPECT_TRUE(resource2->isLoaded == 1);
        EXPECT_TRUE(resource2->fileBuff != NULL);
        EXPECT_EQ(resource2->fileSize, 229837);
    }
}

static void TestPerfomanceLoad(lite3d_resource_pack *pack)
{
    for(int i = 0; i < 10; i++)
    {
        lite3d_resource_file *resource = lite3d_resource_pack_file_load(pack,
            i % 2 ? "pack/pack.0" : "pack/normandy/ref.jpg");
    
        ASSERT_TRUE(resource != NULL);
        EXPECT_TRUE(resource->isLoaded == 1);
        EXPECT_TRUE(resource->fileBuff != NULL);
        EXPECT_EQ(resource->fileSize, i % 2 ? 380065 : 229837);

        lite3d_resource_pack_file_purge(resource);

        EXPECT_TRUE(resource->isLoaded == 0);
        EXPECT_TRUE(resource->fileBuff == NULL);
        EXPECT_EQ(resource->fileSize, 0);
    }
}

class FileSysCache_Test : public ::testing::Test
{
protected:

    // Per-test-case set-up.
    // Called before the first test in this test case.
    // Can be omitted if not needed.

    static void SetUpTestCase()
    {
        /* setup memory */
        lite3d_memory_init(NULL);
        lite3d_logger_setup_stdout();
        lite3d_logger_set_loglevel(LITE3D_LOGLEVEL_ERROR);
    }

    // Per-test-case tear-down.
    // Called after the last test in this test case.
    // Can be omitted if not needed.

    static void TearDownTestCase()
    {
        /* clean memory */
        lite3d_memory_cleanup();
    }

public:

    virtual void SetUp()
    {
        mFileSysPack = lite3d_resource_pack_open("tests/", 0, 700000);
        ASSERT_TRUE(mFileSysPack != NULL);
    }

    virtual void TearDown()
    {
        lite3d_resource_pack_close(mFileSysPack);
    }

protected:
    lite3d_resource_pack *mFileSysPack;
};

TEST_F(FileSysCache_Test, testCommon)
{
    TestCommon(mFileSysPack);
}

TEST_F(FileSysCache_Test, testPerfomanceIndex)
{
    TestPerfomanceIndex(mFileSysPack);
}

TEST_F(FileSysCache_Test, testPerfomanceLoad)
{
    TestPerfomanceLoad(mFileSysPack);
}

class File7zCache_Test : public ::testing::Test
{
protected:

    // Per-test-case set-up.
    // Called before the first test in this test case.
    // Can be omitted if not needed.

    static void SetUpTestCase()
    {
        /* setup memory */
        lite3d_memory_init(NULL);
        lite3d_logger_setup_stdout();
        lite3d_logger_set_loglevel(LITE3D_LOGLEVEL_ERROR);
    }

    // Per-test-case tear-down.
    // Called after the last test in this test case.
    // Can be omitted if not needed.

    static void TearDownTestCase()
    {
        /* clean memory */
        lite3d_memory_cleanup();
    }

public:

    virtual void SetUp()
    {
        mFile7zPack = lite3d_resource_pack_open("tests/pack.1", 1, 700000);
        ASSERT_TRUE(mFile7zPack != NULL);
    }

    virtual void TearDown()
    {
        lite3d_resource_pack_close(mFile7zPack);
    }

protected:
    lite3d_resource_pack *mFile7zPack;
};

TEST_F(File7zCache_Test, testCommon)
{
    TestCommon(mFile7zPack);
}

TEST_F(File7zCache_Test, testPerfomanceIndex)
{
    TestPerfomanceIndex(mFile7zPack);
}

TEST_F(File7zCache_Test, testPerfomanceUncompress)
{
    TestPerfomanceLoad(mFile7zPack);
}
