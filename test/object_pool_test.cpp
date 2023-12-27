#include "object_pool.hpp"
#include <gtest/gtest.h>

template <typename T>
class ObjectPoolTest : public ::ObjectPool<T>
{
public:
  ObjectPoolTest(
      size_t capacity, typename ObjectPool<T>::cleaner_type cleaner = [](T* ptr) { (void)ptr; })
    : ::ObjectPool<T>(capacity, cleaner)
  {
  }
  typename ObjectPool<T>::cleaner_type get_cleaner()
  {
    return ObjectPool<T>::cleaner_;
  };

  typename ObjectPool<T>::bitset_type get_bitset()
  {
    return ObjectPool<T>::bitset_;
  };

  typename ObjectPool<T>::pointer data() const noexcept
  {
    return ObjectPool<T>::data_.get();
  };
};

TEST(ObjectPoolTest, ctor)
{
  ObjectPoolTest<int> pool(10);
  EXPECT_EQ(pool.capacity(), 10);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_NE(pool.get_cleaner(), nullptr);
  EXPECT_EQ(pool.get_bitset().size(), DEFAULT_CAPACITY);
  EXPECT_EQ(pool.get_bitset().count(), 10);
}

TEST(ObjectPoolTest, ctor_with_cleaner)
{
  ObjectPoolTest<int> pool(10, [](int* ptr) { (void)ptr; });
  EXPECT_EQ(pool.capacity(), 10);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_NE(pool.get_cleaner(), nullptr);
  EXPECT_EQ(pool.get_bitset().size(), DEFAULT_CAPACITY);
  EXPECT_EQ(pool.get_bitset().count(), 10);
}

TEST(ObjectPoolTest, ctor_with_capacity_0)
{
  ObjectPoolTest<int> pool(0);
  EXPECT_EQ(pool.capacity(), 0);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_NE(pool.get_cleaner(), nullptr);
  EXPECT_EQ(pool.get_bitset().size(), DEFAULT_CAPACITY);
  EXPECT_EQ(pool.get_bitset().count(), 0);
}

TEST(ObjectPoolTest, ctor_with_capacity_0_with_cleaner)
{
  ObjectPoolTest<int> pool(0, [](int* ptr) { (void)ptr; });
  EXPECT_EQ(pool.capacity(), 0);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_NE(pool.get_cleaner(), nullptr);
  EXPECT_EQ(pool.get_bitset().size(), DEFAULT_CAPACITY);
  EXPECT_EQ(pool.get_bitset().count(), 0);
}

TEST(ObjectPoolTest, alloc)
{
  ObjectPoolTest<int> pool(10);
  EXPECT_EQ(pool.capacity(), 10);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_NE(pool.get_cleaner(), nullptr);
  EXPECT_EQ(pool.get_bitset().size(), DEFAULT_CAPACITY);
  EXPECT_EQ(pool.get_bitset().count(), 10);

  int* ptr = pool.alloc();
  *ptr = 233;
  EXPECT_NE(ptr, nullptr);
  EXPECT_EQ(pool.size(), 1);
  EXPECT_EQ(pool.get_bitset().count(), 9);

  const auto& bitset = pool.get_bitset();
  for (size_t i = 0; i < pool.capacity(); ++i)
  {
    if (ptr == pool.data() + i)
    {
      EXPECT_EQ(bitset[i], false);
      EXPECT_EQ(pool.data()[i], 233);
    }
    else
    {
      EXPECT_EQ(bitset[i], true);
    }
  }
}

TEST(ObjectPoolTest, alloc_with_capacity_0)
{
  ObjectPoolTest<int> pool(0);
  EXPECT_EQ(pool.capacity(), 0);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_NE(pool.get_cleaner(), nullptr);
  EXPECT_EQ(pool.get_bitset().size(), DEFAULT_CAPACITY);
  EXPECT_EQ(pool.get_bitset().count(), 0);

  int* ptr = pool.alloc();
  EXPECT_EQ(ptr, nullptr);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_EQ(pool.get_bitset().count(), 0);
}

TEST(ObjectPoolTest, free)
{
  ObjectPoolTest<int> pool(10);
  EXPECT_EQ(pool.capacity(), 10);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_NE(pool.get_cleaner(), nullptr);
  EXPECT_EQ(pool.get_bitset().size(), DEFAULT_CAPACITY);
  EXPECT_EQ(pool.get_bitset().count(), 10);

  int* ptr = pool.alloc();
  *ptr = 233;
  EXPECT_NE(ptr, nullptr);
  EXPECT_EQ(pool.size(), 1);
  EXPECT_EQ(pool.get_bitset().count(), 9);

  pool.free(ptr);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_EQ(pool.get_bitset().count(), 10);

  const auto& bitset = pool.get_bitset();
  for (size_t i = 0; i < pool.capacity(); ++i)
  {
    EXPECT_EQ(bitset[i], true);
  }
}

TEST(ObjectPoolTest, free_with_cleaner)
{
  int count = 0;
  ObjectPoolTest<int> pool(10, [&count](int* ptr) {
    ++count;
    (void)ptr;
  });
  EXPECT_EQ(pool.capacity(), 10);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_NE(pool.get_cleaner(), nullptr);
  EXPECT_EQ(pool.get_bitset().size(), DEFAULT_CAPACITY);
  EXPECT_EQ(pool.get_bitset().count(), 10);

  int* ptr = pool.alloc();
  *ptr = 233;
  EXPECT_NE(ptr, nullptr);
  EXPECT_EQ(pool.size(), 1);
  EXPECT_EQ(pool.get_bitset().count(), 9);

  pool.free(ptr);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_EQ(pool.get_bitset().count(), 10);
  EXPECT_EQ(count, 0);

  const auto& bitset = pool.get_bitset();
  for (size_t i = 0; i < pool.capacity(); ++i)
  {
    EXPECT_EQ(bitset[i], true);
  }
}

TEST(ObjectPoolTest, get_shared_pointer)
{
  ObjectPoolTest<int> pool(10);
  EXPECT_EQ(pool.capacity(), 10);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_NE(pool.get_cleaner(), nullptr);
  EXPECT_EQ(pool.get_bitset().size(), DEFAULT_CAPACITY);
  EXPECT_EQ(pool.get_bitset().count(), 10);

  {
    auto ptr = pool.get_shared_pointer();
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(pool.size(), 1);
    EXPECT_EQ(pool.get_bitset().count(), 9);

    *ptr = 233;
    const auto& bitset = pool.get_bitset();
    for (size_t i = 0; i < pool.capacity(); ++i)
    {
      if (ptr.get() == pool.data() + i)
      {
        EXPECT_EQ(bitset[i], false);
        EXPECT_EQ(pool.data()[i], 233);
      }
      else
      {
        EXPECT_EQ(bitset[i], true);
      }
    }
  }

  EXPECT_EQ(pool.capacity(), 10);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_NE(pool.get_cleaner(), nullptr);
  EXPECT_EQ(pool.get_bitset().size(), DEFAULT_CAPACITY);
  EXPECT_EQ(pool.get_bitset().count(), 10);
}

TEST(ObjectPoolTest, get_shared_pointer_with_cleaner)
{
  int count = 0;
  ObjectPoolTest<int> pool(10, [&count](int* ptr) {
    ++count;
    *ptr = 0;
  });
  EXPECT_EQ(pool.capacity(), 10);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_NE(pool.get_cleaner(), nullptr);
  EXPECT_EQ(pool.get_bitset().size(), DEFAULT_CAPACITY);
  EXPECT_EQ(pool.get_bitset().count(), 10);

  auto index = -1;
  {
    auto ptr = pool.get_shared_pointer();
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(pool.size(), 1);
    EXPECT_EQ(pool.get_bitset().count(), 9);

    *ptr = 233;
    const auto& bitset = pool.get_bitset();
    for (size_t i = 0; i < pool.capacity(); ++i)
    {
      if (ptr.get() == pool.data() + i)
      {
        EXPECT_EQ(bitset[i], false);
        EXPECT_EQ(pool.data()[i], 233);
        index = i;
      }
      else
      {
        EXPECT_EQ(bitset[i], true);
      }
    }
  }

  EXPECT_EQ(pool.data()[index], 0);
  EXPECT_EQ(pool.capacity(), 10);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_NE(pool.get_cleaner(), nullptr);
  EXPECT_EQ(pool.get_bitset().size(), DEFAULT_CAPACITY);
  EXPECT_EQ(pool.get_bitset().count(), 10);
  EXPECT_EQ(count, 1);
}