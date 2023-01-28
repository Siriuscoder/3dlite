/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
 *
 *	Lite3D is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Lite3D is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <gtest/gtest.h>

#include <lite3d/lite3d_pack.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_logger.h>

static void TestCommon(lite3d_pack *pack)
{
    lite3d_file *resource1 = lite3d_pack_file_load(pack,
        "pack/eev.jpg");

    ASSERT_TRUE(resource1 != NULL);
    EXPECT_TRUE(resource1->isLoaded == 1);
    EXPECT_TRUE(resource1->fileBuff != NULL);
    EXPECT_EQ(resource1->fileSize, 89149u);

    lite3d_file *resource2 = lite3d_pack_file_load(pack,
        "pack/normandy/ref.jpg");

    ASSERT_TRUE(resource2 != NULL);
    EXPECT_TRUE(resource2->isLoaded == 1);
    EXPECT_TRUE(resource2->fileBuff != NULL);
    EXPECT_EQ(resource2->fileSize, 229837u);

    EXPECT_EQ(pack->memoryUsed, 89149u + 229837u);

    lite3d_pack_file_purge(resource1);

    EXPECT_TRUE(resource1->isLoaded == 0);
    EXPECT_TRUE(resource1->fileBuff == NULL);
    EXPECT_EQ(resource1->fileSize, 0u);
    EXPECT_EQ(pack->memoryUsed, 229837u);

    lite3d_file *resource3 = lite3d_pack_file_load(pack,
        "pack/pack.0");

    ASSERT_TRUE(resource3 != NULL);
    EXPECT_TRUE(resource3->isLoaded == 1);
    EXPECT_TRUE(resource3->fileBuff != NULL);
    EXPECT_EQ(resource3->fileSize, 380065u);

    lite3d_file *resource4 = lite3d_pack_file_load(pack,
        "pack/normandy/ref.jpg");

    ASSERT_TRUE(resource4 != NULL);
    EXPECT_TRUE(resource4->isLoaded == 1);
    EXPECT_TRUE(resource4->fileBuff != NULL);
    EXPECT_EQ(resource4->fileSize, 229837u);
    EXPECT_EQ((size_t) resource2, (size_t) resource4);
    EXPECT_EQ(pack->memoryUsed, 380065u + 229837u);

    /* memory limit reached in this point */
    /* expecting purged tail resource */
    lite3d_file *resource5 = lite3d_pack_file_load(pack,
        "pack/normandy/t1.jpg");

    ASSERT_TRUE(resource5 != NULL);
    EXPECT_TRUE(resource5->isLoaded == 1);
    EXPECT_TRUE(resource5->fileBuff != NULL);
    EXPECT_EQ(resource5->fileSize, 122167u);
    EXPECT_EQ(pack->memoryUsed, 229837u + 122167u);
}

static void TestPerfomanceIndex(lite3d_pack *pack)
{
    for(int i = 0; i < 10000; i++)
    {
        lite3d_file *resource = lite3d_pack_file_load(pack,
            "pack/pack.0");
    
        ASSERT_TRUE(resource != NULL);
        EXPECT_TRUE(resource->isLoaded == 1);
        EXPECT_TRUE(resource->fileBuff != NULL);
        EXPECT_EQ(resource->fileSize, 380065u);
    
        lite3d_file *resource2 = lite3d_pack_file_load(pack,
            "pack/normandy/ref.jpg");
    
        ASSERT_TRUE(resource2 != NULL);
        EXPECT_TRUE(resource2->isLoaded == 1);
        EXPECT_TRUE(resource2->fileBuff != NULL);
        EXPECT_EQ(resource2->fileSize, 229837u);
    }
}

static void TestPerfomanceLoad(lite3d_pack *pack)
{
    for(int i = 0; i < 10; i++)
    {
        lite3d_file *resource = lite3d_pack_file_load(pack,
            i % 2 ? "pack/pack.0" : "pack/normandy/ref.jpg");
    
        ASSERT_TRUE(resource != NULL);
        EXPECT_TRUE(resource->isLoaded == 1);
        EXPECT_TRUE(resource->fileBuff != NULL);
        EXPECT_EQ(resource->fileSize, i % 2 ? 380065u : 229837u);

        lite3d_pack_file_purge(resource);

        EXPECT_TRUE(resource->isLoaded == 0);
        EXPECT_TRUE(resource->fileBuff == NULL);
        EXPECT_EQ(resource->fileSize, 0u);
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
        lite3d_logger_setup(NULL);
        lite3d_logger_set_logParams(LITE3D_LOGLEVEL_ERROR, LITE3D_FALSE, LITE3D_TRUE);
    }

    // Per-test-case tear-down.
    // Called after the last test in this test case.
    // Can be omitted if not needed.

    static void TearDownTestCase()
    {
        /* clean memory */
        lite3d_logger_release();
    }

public:

    virtual void SetUp()
    {
        mFileSysPack = lite3d_pack_open("tests/", 0, 700000);
        ASSERT_TRUE(mFileSysPack != NULL);
    }

    virtual void TearDown()
    {
        lite3d_pack_close(mFileSysPack);
    }

protected:
    lite3d_pack *mFileSysPack;
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
        lite3d_logger_setup(NULL);
        lite3d_logger_set_logParams(LITE3D_LOGLEVEL_ERROR, LITE3D_FALSE, LITE3D_TRUE);
    }

    // Per-test-case tear-down.
    // Called after the last test in this test case.
    // Can be omitted if not needed.

    static void TearDownTestCase()
    {
        /* clean memory */
        lite3d_logger_release();
        lite3d_memory_cleanup();
    }

public:

    virtual void SetUp()
    {
        mFile7zPack = lite3d_pack_open("tests/pack/pack.1", 1, 700000);
        ASSERT_TRUE(mFile7zPack != NULL);
    }

    virtual void TearDown()
    {
        lite3d_pack_close(mFile7zPack);
    }

protected:
    lite3d_pack *mFile7zPack;
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
