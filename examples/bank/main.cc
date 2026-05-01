#include "server.h"

int main() {
  Server s;
  bool ok = true;
  for (int i = 0; i < 100; ++i) {
    s.spin();
    ok &= s.logger.check();
  }
  std::cout << (ok ? "PASSED" : "FAILED") << std::endl;
  return 0;
}