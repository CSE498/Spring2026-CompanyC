#include "../../source/web/WebCanvas.hpp"

using namespace cse498;

int main() {
    WebCanvas canvas("main_canvas", 800, 600);

    canvas.Clear();

    canvas.SetFillColor("#2b6cb0");
    canvas.FillRect(50, 50, 200, 120);

    canvas.SetStrokeColor("#ffffff");
    canvas.SetLineWidth(4);
    canvas.DrawLine(50, 50, 250, 170);

    canvas.SetFillColor("#e53e3e");
    canvas.FillCircle(400, 200, 60);

    return 0;
}
