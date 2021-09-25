#include "safeMapFind.h"
#include <array>

constexpr std::array<int, 3> testData {0, 1, 2};
static_assert(AdHoc::containerContains(testData, 0));
