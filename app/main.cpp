#include "helloTriangle.hpp"

#include <cstdlib>
#include <iostream>

auto
main() -> int
{
    auto triangle = HelloTriangle{};

    triangle.run();

    return 0;
}
