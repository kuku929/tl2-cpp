/*
To read from a TVar the user does an explicit cast.
The cast operation records the read operation and returns
a copy of the current value of the TVar.
This test ensures that there is no copy while returning
from the cast.
*/

#include "stm.h"
#include "tl2/tl2.h"
#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <type_traits>
using namespace tl2;

template <typename T> struct force_rvo : T {
  force_rvo() : copy_assign_count(0) {}
  using T::T;
  force_rvo(const force_rvo &);
  force_rvo(force_rvo &&);
  force_rvo<T> &operator=(const force_rvo<T> &other) {
    copy_assign_count++;
    return *this;
  }
  uint copy_assign_count = 0;
};

TEST(SimpleTests, CheckRVO) {
  using type = force_rvo<std::pair<uint, uint>>;
  TVar<type> counter;
  type val, val_check_write_set;
  atomically([&]() {
    val = static_cast<type>(counter);
    val_check_write_set = static_cast<type>(counter);
  });
  EXPECT_EQ(val.copy_assign_count == 1 &&
                val_check_write_set.copy_assign_count == 1,
            true);
}
