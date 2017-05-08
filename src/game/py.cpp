#include "game/py.hpp"
#include "game/game.hpp"

PYBIND11_PLUGIN(_game) {
    pybind11::module m("_game");
    game::py_bind(m);
    return m.ptr();
}

namespace game {

void py_bind(py::module& m) {
    m.def("run", []() {
        std::cout << "WEEEEE\n";
        run_game();
    });
}

}
