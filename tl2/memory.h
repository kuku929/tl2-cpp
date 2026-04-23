#include <array>
#include <memory_resource>
#include <new>

static std::array<std::byte, 20'000> buf;
std::pmr::monotonic_buffer_resource res{buf.data(), buf.size(), std::pmr::null_memory_resource()};