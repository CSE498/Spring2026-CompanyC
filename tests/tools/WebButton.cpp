/**
 * @file WebButton.cpp
 * @brief Unit tests for WebButton class
 * @author Tess Gonda
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/WebButton.hpp"
#include <stdexcept>

TEST_CASE("WebButton - Basic Construction", "[WebButton]") {
    SECTION("Default constructor") {
        WebButton btn;
        REQUIRE(btn.GetLabel() == "Button");
        REQUIRE(btn.IsEnabled() == true);
        REQUIRE(btn.IsPressed() == false);
    }
    
    SECTION("Constructor with label") {
        WebButton btn("Test Label");
        REQUIRE(btn.GetLabel() == "Test Label");
        REQUIRE(btn.IsEnabled() == true);
    }
}

TEST_CASE("WebButton - Label Management", "[WebButton]") {
    WebButton btn("Initial");
    
    SECTION("Get initial label") {
        REQUIRE(btn.GetLabel() == "Initial");
    }
    
    SECTION("Set new label") {
        btn.SetLabel("Updated");
        REQUIRE(btn.GetLabel() == "Updated");
    }
    
    SECTION("Set empty label") {
        btn.SetLabel("");
        REQUIRE(btn.GetLabel() == "");
    }
}

TEST_CASE("WebButton - Enable/Disable", "[WebButton]") {
    WebButton btn("Test");
    
    SECTION("Initially enabled") {
        REQUIRE(btn.IsEnabled() == true);
    }
    
    SECTION("Can be disabled") {
        btn.SetEnabled(false);
        REQUIRE(btn.IsEnabled() == false);
    }
    
    SECTION("Can be re-enabled") {
        btn.SetEnabled(false);
        btn.SetEnabled(true);
        REQUIRE(btn.IsEnabled() == true);
    }
}

TEST_CASE("WebButton - Pressed State", "[WebButton]") {
    WebButton btn("Toggle");
    
    SECTION("Initially not pressed") {
        REQUIRE(btn.IsPressed() == false);
    }
    
    SECTION("Can be pressed") {
        btn.SetPressed(true);
        REQUIRE(btn.IsPressed() == true);
    }
    
    SECTION("Toggle functionality") {
        REQUIRE(btn.IsPressed() == false);
        btn.TogglePressed();
        REQUIRE(btn.IsPressed() == true);
        btn.TogglePressed();
        REQUIRE(btn.IsPressed() == false);
    }
}

TEST_CASE("WebButton - Callbacks", "[WebButton]") {
    WebButton btn("Click Me");
    
    SECTION("Initially no callback") {
        REQUIRE(btn.HasCallback() == false);
    }
    
    SECTION("Callback can be set and executed") {
        bool callback_executed = false;
        btn.SetOnClick([&callback_executed]() {
            callback_executed = true;
        });
        
        REQUIRE(btn.HasCallback() == true);
        btn.Click();
        REQUIRE(callback_executed == true);
    }
    
    SECTION("Disabled button ignores clicks") {
        bool callback_executed = false;
        btn.SetOnClick([&callback_executed]() {
            callback_executed = true;
        });
        
        btn.SetEnabled(false);
        btn.Click();
        REQUIRE(callback_executed == false);
    }
}

TEST_CASE("WebButton - Sizing", "[WebButton]") {
    WebButton btn("Size Test");
    
    SECTION("SetSize changes both dimensions") {
        btn.SetSize(200, 50);
        auto [w, h] = btn.GetSize();
        REQUIRE(w == 200);
        REQUIRE(h == 50);
    }
    
    SECTION("SetWidth changes only width") {
        btn.SetSize(200, 50);
        btn.SetWidth(300);
        auto [w, h] = btn.GetSize();
        REQUIRE(w == 300);
        REQUIRE(h == 50);
    }
    
    SECTION("SetHeight changes only height") {
        btn.SetSize(200, 50);
        btn.SetHeight(80);
        auto [w, h] = btn.GetSize();
        REQUIRE(w == 200);
        REQUIRE(h == 80);
    }
}

TEST_CASE("WebButton - Error Handling", "[WebButton]") {
    WebButton btn("Error Test");
    
    SECTION("Negative width throws") {
        REQUIRE_THROWS_AS(btn.SetWidth(-10), std::invalid_argument);
    }
    
    SECTION("Negative height throws") {
        REQUIRE_THROWS_AS(btn.SetHeight(-5), std::invalid_argument);
    }
    
    SECTION("Negative border width throws") {
        REQUIRE_THROWS_AS(btn.SetBorderWidth(-1), std::invalid_argument);
    }
    
    SECTION("Negative border radius throws") {
        REQUIRE_THROWS_AS(btn.SetBorderRadius(-1), std::invalid_argument);
    }
    
    SECTION("Empty parent ID throws") {
        REQUIRE_THROWS_AS(btn.AppendTo(""), std::invalid_argument);
    }
}

TEST_CASE("WebButton - Styling", "[WebButton]") {
    WebButton btn("Style Test");
    
    SECTION("Background color") {
        btn.SetBackgroundColor("#FF0000");
        REQUIRE(btn.GetBackgroundColor() == "#FF0000");
    }
    
    SECTION("Text and border colors can be set") {
        btn.SetTextColor("#FFFFFF");
        btn.SetBorderColor("#000000");
        // No getters, verify no crash
        REQUIRE(true);
    }
    
    SECTION("Border properties") {
        btn.SetBorderWidth(5);
        btn.SetBorderRadius(10);
        // No getters, verify no crash
        REQUIRE(true);
    }
    
    SECTION("SetBorder convenience method") {
        btn.SetBorder(3, "#123456", 8);
        // No getters, verify no crash
        REQUIRE(true);
    }
}

TEST_CASE("WebButton - Style Reset", "[WebButton]") {
    WebButton btn("Reset Test");
    
    btn.SetSize(500, 200);
    btn.SetBackgroundColor("#FF00FF");
    btn.SetBorderRadius(50);
    
    btn.ResetStyle();
    
    auto [w, h] = btn.GetSize();
    REQUIRE(w == 100);
    REQUIRE(h == 30);
    REQUIRE(btn.GetBackgroundColor() == "#f0f0f0");
}

TEST_CASE("WebButton - ToString Debug Output", "[WebButton]") {
    WebButton btn("Debug Test");
    btn.SetEnabled(false);
    btn.SetPressed(true);
    
    std::string str = btn.ToString();
    REQUIRE(str.find("Debug Test") != std::string::npos);
    REQUIRE(str.find("enabled=false") != std::string::npos);
    REQUIRE(str.find("pressed=true") != std::string::npos);
}

TEST_CASE("WebButton - Callback Exception Handling", "[WebButton]") {
    WebButton btn("Exception Test");
    
    // Callback that throws, should be caught internally
    btn.SetOnClick([]() {
        throw std::runtime_error("Test exception");
    });
    
    // Should not crash or propagate exception
    REQUIRE_NOTHROW(btn.Click());
}

TEST_CASE("WebButton - Move Semantics", "[WebButton]") {
    SECTION("Move constructor") {
        WebButton btn1("Original");
        btn1.SetBackgroundColor("#123456");
        btn1.SetSize(150, 60);
        
        WebButton btn2(std::move(btn1));
        REQUIRE(btn2.GetLabel() == "Original");
        REQUIRE(btn2.GetBackgroundColor() == "#123456");
        auto [w, h] = btn2.GetSize();
        REQUIRE(w == 150);
        REQUIRE(h == 60);
    }
    
    SECTION("Move assignment") {
        WebButton btn1("Original");
        btn1.SetBackgroundColor("#123456");
        
        WebButton btn2("Temp");
        btn2 = std::move(btn1);
        REQUIRE(btn2.GetLabel() == "Original");
        REQUIRE(btn2.GetBackgroundColor() == "#123456");
    }
}

TEST_CASE("WebButton - Unique IDs", "[WebButton]") {
    WebButton btn1("Button 1");
    WebButton btn2("Button 2");
    WebButton btn3("Button 3");
    
    std::string id1 = btn1.GetElementID();
    std::string id2 = btn2.GetElementID();
    std::string id3 = btn3.GetElementID();
    
    // All IDs should be different
    REQUIRE(id1 != id2);
    REQUIRE(id2 != id3);
    REQUIRE(id1 != id3);
}

TEST_CASE("WebButton - Constructor with Callback", "[WebButton]") {
    bool callback_executed = false;
    
    WebButton btn("Click Me", [&callback_executed]() {
        callback_executed = true;
    });
    
    REQUIRE(btn.HasCallback() == true);
    REQUIRE(btn.GetLabel() == "Click Me");
    
    btn.Click();
    REQUIRE(callback_executed == true);
}