#define CATCH_CONFIG_MAIN
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../source/Datum.hpp"


TEST_CASE("Test Datum Constructors", "[core]")
{
    SECTION("Default constructor creates NaN")
    {
        Datum d;
        CHECK(std::isnan(d.AsDouble()));
    }

    SECTION("Double constructor")
    {
        Datum d(3.14);
        CHECK(d.IsDouble());
        CHECK(d.AsDouble() == 3.14);
    }

    SECTION("Bool constructor")
    {
        Datum t(true);
        Datum f(false);
        CHECK(t.IsBool());
        CHECK(f.IsBool());
        CHECK(t.AsBool() == true);
        CHECK(f.AsBool() == false);
    }

    SECTION("String constructor")
    {
        Datum d(std::string("hello"));
        CHECK(d.IsString());
        CHECK(d.AsString() == "hello");
    }

    SECTION("C-string constructor")
    {
        Datum d("hello");
        CHECK(d.IsString());
        CHECK(d.AsString() == "hello");
    }
}

TEST_CASE("Test Datum Type Checking", "[core]")
{
    SECTION("IsDouble is exclusive")
    {
        Datum d(3.14);
        CHECK(d.IsDouble());
        CHECK(!d.IsBool());
        CHECK(!d.IsString());
    }

    SECTION("IsBool is exclusive")
    {
        Datum d(true);
        CHECK(d.IsBool());
        CHECK(!d.IsDouble());
        CHECK(!d.IsString());
    }

    SECTION("IsString is exclusive")
    {
        Datum d("hello");
        CHECK(d.IsString());
        CHECK(!d.IsDouble());
        CHECK(!d.IsBool());
    }
}

TEST_CASE("Test Datum AsString", "[core]")
{
    SECTION("Double to string")
    {
        Datum d(3.14);
        CHECK(d.AsString() == "3.140000");
    }

    SECTION("Bool true to string")
    {
        Datum d(true);
        CHECK(d.AsString() == "true");
    }

    SECTION("Bool false to string")
    {
        Datum d(false);
        CHECK(d.AsString() == "false");
    }

    SECTION("String returns itself")
    {
        Datum d("hello");
        CHECK(d.AsString() == "hello");
    }
}

TEST_CASE("Test Datum AsDouble", "[core]")
{
    SECTION("Double returns itself")
    {
        Datum d(3.14);
        CHECK(d.AsDouble() == 3.14);
    }

    SECTION("Bool true returns 1.0")
    {
        Datum d(true);
        CHECK(d.AsDouble() == 1.0);
    }

    SECTION("Bool false returns 0.0")
    {
        Datum d(false);
        CHECK(d.AsDouble() == 0.0);
    }

    SECTION("Numeric string returns double")
    {
        Datum d("3.14");
        CHECK(d.AsDouble() == 3.14);
    }

    SECTION("Non-numeric string returns NaN")
    {
        Datum d("hello");
        CHECK(std::isnan(d.AsDouble()));
    }

    SECTION("Empty string returns NaN")
    {
        Datum d("");
        CHECK(std::isnan(d.AsDouble()));
    }
}

TEST_CASE("Test Datum AsBool", "[core]")
{
    SECTION("Bool returns itself")
    {
        CHECK(Datum(true).AsBool() == true);
        CHECK(Datum(false).AsBool() == false);
    }

    SECTION("Non-zero double returns true")
    {
        CHECK(Datum(3.14).AsBool() == true);
    }

    SECTION("Zero double returns false")
    {
        CHECK(Datum(0.0).AsBool() == false);
    }

    SECTION("NaN double returns false")
    {
        Datum d;
        CHECK(d.AsBool() == false);
    }

    SECTION("Non-empty string returns true")
    {
        CHECK(Datum("hello").AsBool() == true);
    }

    SECTION("Empty string returns false")
    {
        CHECK(Datum("").AsBool() == false);
    }
}

TEST_CASE("Test Datum Arithmetic", "[core]")
{
    SECTION("Double addition")
    {
        Datum a(3.0), b(2.0);
        CHECK((a + b).AsDouble() == 5.0);
    }

    SECTION("Double subtraction")
    {
        Datum a(3.0), b(2.0);
        CHECK((a - b).AsDouble() == 1.0);
    }

    SECTION("Double multiplication")
    {
        Datum a(3.0), b(2.0);
        CHECK((a * b).AsDouble() == 6.0);
    }

    SECTION("Double division")
    {
        Datum a(6.0), b(2.0);
        CHECK((a / b).AsDouble() == 3.0);
    }

    SECTION("Division by zero returns NaN")
    {
        Datum a(6.0), b(0.0);
        CHECK(std::isnan((a / b).AsDouble()));
    }

    SECTION("NaN arithmetic propagates NaN")
    {
        Datum a, b(2.0);
        CHECK(std::isnan((a + b).AsDouble()));
        CHECK(std::isnan((a - b).AsDouble()));
        CHECK(std::isnan((a * b).AsDouble()));
        CHECK(std::isnan((a / b).AsDouble()));
    }

    SECTION("Bool arithmetic")
    {
        Datum a(true), b(true);
        CHECK((a + b).AsDouble() == 2.0);
    }

    SECTION("Numeric string arithmetic")
    {
        Datum a("3.0"), b("2.0");
        CHECK((a + b).AsDouble() == 5.0);
    }

    SECTION("Non-numeric string arithmetic returns NaN")
    {
        Datum a("hello"), b(2.0);
        CHECK(std::isnan((a + b).AsDouble()));
    }
}

TEST_CASE("Test Datum Comparison Operators", "[core]")
{
    SECTION("Equal doubles")
    {
        CHECK(Datum(3.14) == Datum(3.14));
        CHECK_FALSE(Datum(3.14) == Datum(2.0));
    }

    SECTION("Equal bools")
    {
        CHECK(Datum(true) == Datum(true));
        CHECK(Datum(false) == Datum(false));
        CHECK_FALSE(Datum(true) == Datum(false));
    }

    SECTION("Equal strings")
    {
        CHECK(Datum("hello") == Datum("hello"));
        CHECK_FALSE(Datum("hello") == Datum("world"));
    }

    SECTION("NaN is not equal to NaN")
    {
        Datum a, b;
        CHECK_FALSE(a == b);
    }

    SECTION("Not equal operator")
    {
        CHECK(Datum(1.0) != Datum(2.0));
        CHECK_FALSE(Datum(1.0) != Datum(1.0));
    }

    SECTION("Less than doubles")
    {
        CHECK(Datum(1.0) < Datum(2.0));
        CHECK_FALSE(Datum(2.0) < Datum(1.0));
        CHECK_FALSE(Datum(1.0) < Datum(1.0));
    }

    SECTION("Less than strings lexicographic")
    {
        CHECK(Datum("apple") < Datum("banana"));
        CHECK_FALSE(Datum("banana") < Datum("apple"));
    }

    SECTION("Greater than")
    {
        CHECK(Datum(2.0) > Datum(1.0));
        CHECK_FALSE(Datum(1.0) > Datum(2.0));
    }

    SECTION("Less than or equal")
    {
        CHECK(Datum(1.0) <= Datum(2.0));
        CHECK(Datum(1.0) <= Datum(1.0));
        CHECK_FALSE(Datum(2.0) <= Datum(1.0));
    }

    SECTION("Greater than or equal")
    {
        CHECK(Datum(2.0) >= Datum(1.0));
        CHECK(Datum(1.0) >= Datum(1.0));
        CHECK_FALSE(Datum(1.0) >= Datum(2.0));
    }

    SECTION("NaN comparisons return false")
    {
        Datum nan;
        CHECK_FALSE(nan < Datum(1.0));
        CHECK_FALSE(nan > Datum(1.0));
        CHECK_FALSE(nan <= Datum(1.0));
        CHECK_FALSE(nan >= Datum(1.0));
    }
}