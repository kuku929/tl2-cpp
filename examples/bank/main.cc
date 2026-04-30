#include "server.h"

int main() {
    Server s;
    for(int i = 0; i < 1; ++i) {
        s.spin();
    }
    return 0;
}