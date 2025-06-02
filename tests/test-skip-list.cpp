#include "../SkipList.h"
#include <gtest/gtest.h>

class SkipListTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        sl1.insert(3);
        sl1.insert(1);
        sl1.insert(4);
    }

    skip_list<int> empty;
    skip_list<int> sl1;
};

TEST_F(SkipListTest, DefaultConstructor)
{
    EXPECT_TRUE(empty.empty());
    EXPECT_EQ(empty.size(), 0);
}

TEST_F(SkipListTest, InsertAndSize)
{
    skip_list<int> sl;
    sl.insert(5);
    sl.insert(2);
    sl.insert(8);
    EXPECT_EQ(sl.size(), 3);
}

TEST_F(SkipListTest, IteratorTraversal)
{
    auto it = sl1.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 3);
    it++;
    EXPECT_EQ(*it, 4);
    ++it;
    EXPECT_EQ(it, sl1.end());
}

TEST_F(SkipListTest, ConstIterator)
{
    const auto& csl = sl1;
    auto it = csl.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 3);
}

TEST_F(SkipListTest, FindExisting)
{
    auto it = sl1.find(3);
    EXPECT_NE(it, sl1.end());
    EXPECT_EQ(*it, 3);
}

TEST_F(SkipListTest, FindMissing)
{
    auto it = sl1.find(99);
    EXPECT_EQ(it, sl1.end());
}

TEST_F(SkipListTest, CopyConstructor)
{
    skip_list<int> sl2(sl1);
    EXPECT_EQ(sl1.size(), sl2.size());

    auto it1 = sl1.begin();
    auto it2 = sl2.begin();
    while (it1 != sl1.end() && it2 != sl2.end())
    {
        EXPECT_EQ(*it1, *it2);
        ++it1;
        ++it2;
    }
}

TEST_F(SkipListTest, MoveConstructor)
{
    skip_list<int> sl2(std::move(sl1));
    EXPECT_EQ(sl2.size(), 3);
    EXPECT_TRUE(sl1.empty());
}

TEST_F(SkipListTest, CopyAssignment)
{
    skip_list<int> sl2;
    sl2 = sl1;
    EXPECT_EQ(sl1.size(), sl2.size());
    EXPECT_EQ(*sl2.find(4), 4);
}

TEST_F(SkipListTest, MoveAssignment)
{
    skip_list<int> sl2;
    sl2 = std::move(sl1);
    EXPECT_EQ(sl2.size(), 3);
    EXPECT_TRUE(sl1.empty());
}

TEST_F(SkipListTest, Clear)
{
    EXPECT_FALSE(sl1.empty());
    sl1.clear();
    EXPECT_TRUE(sl1.empty());
    EXPECT_EQ(sl1.size(), 0);
}

TEST_F(SkipListTest, EraseMiddle)
{
    auto it = sl1.find(3);
    it = sl1.erase(it);
    EXPECT_EQ(sl1.size(), 2);
    EXPECT_EQ(*it, 4);
    EXPECT_EQ(sl1.find(3), sl1.end());
}

TEST_F(SkipListTest, EraseFirst)
{
    auto it = sl1.begin();
    it = sl1.erase(it);
    EXPECT_EQ(sl1.size(), 2);
    EXPECT_EQ(*it, 3);
    EXPECT_EQ(sl1.find(1), sl1.end());
}

TEST_F(SkipListTest, EraseLast)
{
    auto it = --sl1.end();
    it = sl1.erase(it);
    EXPECT_EQ(sl1.size(), 2);
    EXPECT_EQ(it, sl1.end());
    EXPECT_EQ(sl1.find(4), sl1.end());
}

TEST_F(SkipListTest, Swap)
{
    skip_list<int> sl2;
    sl2.insert(9);

    sl1.swap(sl2);

    EXPECT_EQ(sl1.size(), 1);
    EXPECT_EQ(sl2.size(), 3);
    EXPECT_EQ(*sl1.begin(), 9);
    EXPECT_EQ(*sl2.begin(), 1);
}

TEST_F(SkipListTest, EqualityOperator)
{
    skip_list<int> sl2;
    sl2.insert(1);
    sl2.insert(3);
    sl2.insert(4);
    EXPECT_TRUE(sl1 == sl2);

    sl2.insert(5);
    EXPECT_FALSE(sl1 == sl2);
}

TEST_F(SkipListTest, InequalityOperator)
{
    skip_list<int> sl2;
    sl2.insert(1);
    EXPECT_TRUE(sl1 != sl2);
}

TEST_F(SkipListTest, ReverseIteration)
{
    auto it = --sl1.end();
    EXPECT_EQ(*it, 4);
    --it;
    EXPECT_EQ(*it, 3);
    it--;
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(it, sl1.begin());
}

TEST_F(SkipListTest, IteratorEdgeCases)
{
    skip_list<int> sl;
    sl.insert(10);

    auto it = sl.begin();
    EXPECT_EQ(*it, 10);

    it = sl.erase(it);
    EXPECT_EQ(it, sl.end());

    EXPECT_THROW(--it, std::out_of_range);
    EXPECT_THROW(it--, std::out_of_range);
}

TEST_F(SkipListTest, AllocatorSupport)
{
    using custom_alloc = std::allocator<int>;
    skip_list<int, std::less<int>, custom_alloc> sl;
    sl.insert(5);
    sl.insert(2);

    auto alloc = sl.get_allocator();
    EXPECT_TRUE((std::is_same_v<decltype(alloc), custom_alloc>));
    EXPECT_EQ(sl.size(), 2);
}

TEST_F(SkipListTest, LargeInsertion)
{
    skip_list<int> sl;
    const int N = 1000;
    for (int i = 0; i < N; ++i)
    {
        sl.insert(N - i - 1);
    }

    EXPECT_EQ(sl.size(), N);
    int count = 0;
    for (auto it = sl.begin(); it != sl.end(); ++it)
    {
        EXPECT_EQ(*it, count++);
    }
}

TEST_F(SkipListTest, EraseAll)
{
    auto it = sl1.begin();
    while (it != sl1.end())
    {
        it = sl1.erase(it);
    }
    EXPECT_TRUE(sl1.empty());
}

TEST_F(SkipListTest, MovePreservesContent)
{
    skip_list<int> sl2 = std::move(sl1);
    auto it = sl2.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 3);
    ++it;
    EXPECT_EQ(*it, 4);
    ++it;
    EXPECT_EQ(it, sl2.end());
}

TEST_F(SkipListTest, Contains)
{
    skip_list<int> sl;
    sl.insert(5);
    sl.insert(2);
    EXPECT_TRUE(sl.contains(2));
    EXPECT_FALSE(sl.contains(3));
}

TEST_F(SkipListTest, Emplace)
{
    skip_list<std::pair<int, int>> sl;
    auto it = sl.emplace(1, 2);
    EXPECT_EQ(it->first, 1);
    EXPECT_EQ(it->second, 2);
    EXPECT_TRUE(sl.contains({1, 2}));
}

TEST_F(SkipListTest, PushFrontBack)
{
    skip_list<int> sl;
    sl.push_front(3);
    sl.push_front(1);
    sl.push_back(5);

    auto it = sl.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 3);
    ++it;
    EXPECT_EQ(*it, 5);
}

TEST_F(SkipListTest, PopFrontBack)
{
    skip_list<int> sl;
    sl.insert(1);
    sl.insert(2);
    sl.insert(3);

    sl.pop_front();
    EXPECT_EQ(*sl.begin(), 2);

    sl.pop_back();
    EXPECT_EQ(*--sl.end(), 2);
}

TEST_F(SkipListTest, Resize)
{
    skip_list<int> sl;
    sl.insert(1);
    sl.insert(3);

    sl.resize(4, 5);
    EXPECT_EQ(sl.size(), 4);
    EXPECT_EQ(*--sl.end(), 5);

    sl.resize(2);
    EXPECT_EQ(sl.size(), 2);
    EXPECT_EQ(*--sl.end(), 3);

    sl.resize(2);
    EXPECT_EQ(sl.size(), 2);
}

TEST_F(SkipListTest, PopEmpty)
{
    skip_list<int> sl;
    EXPECT_THROW(sl.pop_front(), std::out_of_range);
    EXPECT_THROW(sl.pop_back(), std::out_of_range);
}