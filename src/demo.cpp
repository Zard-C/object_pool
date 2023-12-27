#include "object_pool.hpp"
#include <thread>
#include <vector>
#include <chrono>
#include <memory>
#include <cstdio>

struct Object
{
  int a;
  int b;
  int c;
  int d;
  int e;
  int f;
  int g;
  double h;
};

int main(int argc, const char* const* argv)
{
  if (argc < 2)
  {
    printf("Usage: %s <0: object_pool, 1: std::make_shared()>\n", argv[0]);
    return 1;
  }

  auto choice = atoi(argv[1]);

  if (choice == 0)
  {
    printf("Using object_pool\n");
  }
  else
  {
    printf("Using std::make_shared()\n");
  }

  ObjectPool<Object> pool(256);
  std::vector<std::thread> threads;
  for (int i = 0; i < 8; ++i)
  {
    threads.emplace_back([&pool, i, choice]() {
      for (int i = 0; i < 65536; ++i)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::shared_ptr<Object> obj;
        if (choice == 0)
        {
          obj = get_shared_pointer_from(pool);
        }
        else
        {
          obj = std::make_shared<Object>();
        }
        obj->a = i;
        obj->b = i;
        obj->c = i;
        obj->d = i;
        obj->e = i;
        obj->f = i;
        obj->g = i;
        obj->h = i;
      }
    });
  }

  for (auto& t : threads)
  {
    t.join();
  }

  return 0;
}