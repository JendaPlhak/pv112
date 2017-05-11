#pragma once

struct GameOptions {
    bool machine_gun = true;
    float game_time = 35;
    float ball_time = 10;
};

int run_game(const GameOptions& opts);
