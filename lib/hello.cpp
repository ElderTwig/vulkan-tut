#include "hello.hpp"

#include <string>

auto
world() noexcept -> std::string
{
    return {"Hello world!"};
}