#include "core/VentilationController.h"
#include <iostream>
int main() {
    ventilation::VentilationController controller;
    const auto out = controller.update({14,80,0,true}, {5,90,0,true}, 0);
    std::cout << ventilation::stateName(out.state) << ": " << out.reason << '\n';
    return out.fanRequested ? 1 : 0;
}
