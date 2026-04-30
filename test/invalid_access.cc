/*
Accessing a variable outside a transaction is prohibited.
This tests tries to access TVar's outside an atomic block
expecting an exception.
Note that TVar's are copy assignable/constructible. They are NOT move
assignable or constructible. The following codes are thus identical:
TVar<int> b(0), a(1);
atomically([&]()
{
    b = a;
}
);

atomically([&]()
{
    b = static_cast<int>(a);
})

atomically([&]()
{
    b = std::move(a);
})
*/
#include "hash_table.h"
#include "stm.h"
#include "tl2/tl2.h"
#include <gtest/gtest.h>
#include <thread>
#include <type_traits>
#include <vector>

using namespace tl2;

TEST(SimpleTests, InvalidAccess) {
  TVar<int> test(0);
  const auto invalid_read = [&]() {
    volatile int val = static_cast<int>(test);
  };
  const auto invalid_write = [&]() { test = 1; };
  const auto invalid_copy_assignment = [&]() { test = TVar<int>(1); };
  const auto invalid_move_assignment = [&]() {
    test = std::move(TVar<int>(1));
  };
  static_assert(std::is_move_constructible_v<TVar<int>>,
                "TVar should be move constructible. "
                "while this may lead to undefined behaviour since we access "
                "TVars outside a transaction, "
                "we need to support move for std containers.\n");
  static_assert(std::is_copy_constructible_v<TVar<int>>,
                "TVar should be copy constructible. "
                "while this may lead to undefined behaviour since we access "
                "TVars outside a transaction, "
                "we need to support copy for std containers.\n");
  ASSERT_ANY_THROW(invalid_write());
  ASSERT_ANY_THROW(invalid_read());
  ASSERT_ANY_THROW(invalid_copy_assignment());
  ASSERT_ANY_THROW(invalid_move_assignment());
  atomically([&]() { test = 1; });
  // Making sure state is reset.
  ASSERT_ANY_THROW(invalid_write());
  ASSERT_ANY_THROW(invalid_read());
  ASSERT_ANY_THROW(invalid_copy_assignment());
  ASSERT_ANY_THROW(invalid_move_assignment());
}