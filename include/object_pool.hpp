#ifndef __OBJECT_POOL_HPP__
#define __OBJECT_POOL_HPP__

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>

static const size_t DEFAULT_CAPACITY = 1024;

template <typename T>
class ObjectPool
{
public:
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using unique_arr_ptr_type = std::unique_ptr<T[]>;
  using mutex_type = std::mutex;
  using lock_type = std::lock_guard<mutex_type>;
  using unique_lock_type = std::unique_lock<mutex_type>;
  using shared_ptr_type = std::shared_ptr<T>;
  using cleaner_type = std::function<void(T*)>;
  using bitset_type = std::bitset<DEFAULT_CAPACITY>;

public:
  ObjectPool(
      size_t capacity, cleaner_type cleaner = [](T* ptr) { (void)ptr; })
    : capacity_(capacity), cleaner_(cleaner)
  {
    for (size_type i = 0; i < capacity_; ++i)
    {
      bitset_.set(i, true);
    }

    try
    {
      data_ = unique_arr_ptr_type(new T[capacity_]);
    }
    catch (std::bad_alloc& e)
    {
      throw e;
      capacity_ = 0;
      bitset_.reset();
    }
  }

  ObjectPool(const ObjectPool&) = delete;
  ObjectPool(ObjectPool&&) = delete;
  ObjectPool& operator=(const ObjectPool&) = delete;
  ObjectPool& operator=(ObjectPool&&) = delete;

  ~ObjectPool()
  {
    for (size_type i = 0; i < capacity_; ++i)
    {
      if (bitset_[i])
      {
        cleaner_(&data_[i]);
      }
    }
  }

  shared_ptr_type get_shared_pointer()
  {
    return std::shared_ptr<value_type>(alloc(), [this](value_type* ptr) {
      if (ptr)
      {
        cleaner_(ptr);
        free(ptr);
      }
    });
  }

  size_type capacity() const noexcept
  {
    return capacity_;
  }

  size_type size() noexcept
  {
    std::lock_guard<mutex_type> lock(mutex_);
    return capacity_ - bitset_.count();
  }

  pointer alloc() noexcept
  {
    std::lock_guard<mutex_type> lock(mutex_);
    size_type index = bitset_._Find_first();
    if (index == bitset_.size())
    {
      return nullptr;
    }
    bitset_.set(index, false);
    return &data_[index];
  }

  void free(pointer ptr) noexcept
  {
    std::lock_guard<mutex_type> lock(mutex_);
    bitset_.set(ptr - data_.get(), true);
  }

protected:
  size_type capacity_{ 0 };
  unique_arr_ptr_type data_{ nullptr };
  mutex_type mutex_;
  cleaner_type cleaner_;
  bitset_type bitset_;
};

template <typename T>
std::shared_ptr<T> get_shared_pointer_from(ObjectPool<T>& pool)
{
  auto ptr = pool.get_shared_pointer();
  if (!ptr)
  {
    ptr = std::make_shared<T>();
  }
  return ptr;
}

#endif
