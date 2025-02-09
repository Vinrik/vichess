#pragma once
#include "board.h"
#include "moves.h"

// Minimax with alpha-beta
int minimax(Board board, int depth, int alpha, int beta, bool maximizingPlayer);

// Find best move via iterative deepening
Move findBestMove(Board board, int maxDepth);
