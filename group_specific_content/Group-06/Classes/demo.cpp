// demo.cpp
#include <iostream>
#include <string>

#include "MemoryFactory.hpp"

// A small type to allocate from the pool
struct Widget {
  int id;
  std::string name;

  Widget(int i, std::string n) : id(i), name(std::move(n)) {
    std::cout << "Widget consructor  id=" << id << " name=" << name << "\n";
  }

  ~Widget() {
    std::cout << "Widget destructor  id=" << id << " name=" << name << "\n";
  }

  void Hello() const {
    std::cout << "Hello from Widget(" << id << ", " << name << ")\n";
  }
};

int main() {
  MemoryFactory<Widget, 4> pool(
      /*initialBlocks=*/1); // initialize memory factory with 1 block (4 widgets
                            // per block)
  // Raw Make and Delete
  Widget *a = pool.Make(1, "alpha");
  a->Hello();

  Widget *b = pool.Make(2, "beta");
  b->Hello();

  // Return to pool (calls destructor + pushes slot back to freelist)
  pool.Delete(a);
  pool.Delete(b);

  // c and d will reuse slots
  Widget *c = pool.Make(3, "gamma");
  Widget *d = pool.Make(4, "delta");

  pool.Delete(c);
  pool.Delete(d);

  // MakeUnique
  {
    auto u1 = pool.MakeUnique(5, "unique-one");
    u1->Hello();

    auto u2 = pool.MakeUnique(6, "unique-two");
    u2->Hello();
  }
  // u1 and u2 fall out of scope and are freed and destructed
  return 0;
}
