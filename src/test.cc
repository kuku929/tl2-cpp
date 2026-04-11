#include <iostream>
#include <vector>

struct STM {
    // need some machinery to deduce lambda return types and so on...
    template<typename Transaction> void atomically(Transaction t);
};

struct TVar {

};


int main(int argc, char *argv[]) {
    std::cout << "Hello World!" << std::endl;
    std::vector<int> a = {1, 2, 3, 4};
    const auto test = [&](this auto &&self) {
    };
    for(size_t i = 0; i < a.size(); ++i) {
        std::cout << a[i] << std::endl;
    }
}
