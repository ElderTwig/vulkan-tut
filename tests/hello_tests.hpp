#ifndef NAME_HELLO_TESTS_HPP
#define NAME_HELLO_TESTS_HPP

#include "hello.hpp"

#include <catch2/catch.hpp>

#include <string>
#include <type_traits>

TEST_CASE("world() returns \"Hello world!\"", "[world]")
{
    STATIC_REQUIRE(std::is_same_v<decltype(world()), std::string>);
    REQUIRE(world() == std::string("Hello world!"));
}

#endif    // NAME_HELLO_TESTS_HPP