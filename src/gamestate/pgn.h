#pragma once
#include <string>
#include <vector>
#include "gamestate.h"

// Minimal PGN import/export
namespace PGN {
    bool importPGN(const std::string &pgnText, GameState &game);
    std::string exportPGN(const GameState &game);
}
