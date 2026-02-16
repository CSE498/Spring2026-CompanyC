#include <iostream>
#include "SerializerBase.hpp"

int main() {
    SerializerBase s;

    std::cout << s.Serialize(42) << std::endl;
    std::cout << s.Serialize(3.14) << std::endl;
    std::cout << s.Serialize(true) << std::endl;
    std::cout << s.Serialize('A') << std::endl;
    std::cout << s.Serialize("hello") << std::endl;

    auto i = s.DeserializeInt(s.Serialize(42));
    if (i) std::cout << "int round-trip: " << *i << std::endl;

    auto d = s.DeserializeDouble(s.Serialize(3.14));
    if (d) std::cout << "double round-trip: " << *d << std::endl;

    auto b = s.DeserializeBool(s.Serialize(true));
    if (b) std::cout << "bool round-trip: " << *b << std::endl;

    auto c = s.DeserializeChar(s.Serialize('A'));
    if (c) std::cout << "char round-trip: " << *c << std::endl;

    auto str = s.DeserializeString(s.Serialize(std::string("hello")));
    if (str) std::cout << "string round-trip: " << *str << std::endl;

    return 0;
}