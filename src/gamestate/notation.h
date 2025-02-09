#pragma once
#include "board.h"
#include "moves.h"
#include <string>

// Build a basic SAN
std::string buildSAN(const Board &boardBefore, const Move &move);
