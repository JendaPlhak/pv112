#include "game/py.hpp"
#include "game/game.hpp"

PYBIND11_PLUGIN(_game) {
    pybind11::module m("_game");
    game::py_bind(m);
    return m.ptr();
}

namespace game {

void py_bind(py::module& m) {
    py::class_<GameOptions>(m, "Options")
    .def(py::init<>())
    .def_readwrite("machine_gun", &GameOptions::machine_gun)
    .def_readwrite("game_time",   &GameOptions::game_time)
    .def_readwrite("ball_time",   &GameOptions::ball_time);

    m.def("run", run_game);

}

}
