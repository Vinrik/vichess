# vichess

A chess visualiser for watching engines play each other. It is the first step of an interpretability question: can an engine's decisions be translated into human language and chess knowledge?

vichess is a desktop app written in C++17 and Qt 6. It has its own implementation of the rules, lets you play or step through a game, and drives an external UCI engine (Stockfish), either for a single move or to play both sides while you watch.

Why

Engines play far beyond human strength, but they don't explain themselves: a best move arrives with a score and no reasons. vichess is the observation layer for studying that gap. It records every move of a game in standard notation, with the position before and after it, so engine-versus-engine games can be replayed, saved as PGN and examined. The next step is to describe what the engine is doing in terms a human player would recognise.

What it does

In the app:

Play by clicking. Select a piece to see its legal moves highlighted. Pawn promotion offers a queen, rook, bishop or knight.
Engine move. Ask Stockfish for its best move in the current position (search depth 8).
Engine vs engine. Stockfish plays both sides, one move a second.
Move list and replay. Every move appears in standard algebraic notation (SAN), with buttons to jump to the start, step back or forward, and jump to the end.
Save PGN. Export the game for other chess tools.

Under the hood:

The full rules: castling, en passant, promotion, check, stalemate, insufficient material, threefold repetition and the 50-move rule.
FEN import and export, and PGN import and export.
A small built-in search (minimax with alpha–beta pruning over a material-count evaluation). The tests use it; the app does not yet.
Build and run

You need:

a C++17 compiler and CMake 3.14 or later
Qt 6.2 or later (Widgets)
GoogleTest (the build currently requires it, for the tests)
Stockfish, for the engine features
bash
cmake -S . -B build
cmake --build build
./build/src/gui/vichess_gui

The app looks for Stockfish at /usr/bin/stockfish, set in src/gui/mainwindow.cpp. On macOS or Windows, change that path to wherever Stockfish is installed.

Tests

The tests use GoogleTest. They cover piece movement, castling, en passant and the built-in search:

bash
./build/tests/test_visualizer
./build/tests/test_enpassant
./build/tests/test_analysis
Project layout
Folder	What it holds
src/core	The board, move rules, legal-move generation and FEN
src/gamestate	Game history, SAN notation, and PGN import and export
src/analysis	Material evaluation and minimax search
src/engine	The link to an external UCI engine, through QProcess
src/gui	The Qt window, the board drawing and the promotion choice
resources	Piece images
tests	GoogleTest suites
Known gaps
The Stockfish path is fixed in the code.
PGN import, FEN loading and an analysis mode exist in the code, but the toolbar doesn't offer them yet.
tests/test_engine_integration.cpp is not yet listed in tests/CMakeLists.txt, so it isn't built.
Draws are reported in the console, not in the window.
Licence

vichess is released under the PolyForm Noncommercial License 1.0.0. You may use, change and share it for any non-commercial purpose, including personal projects, study and research. Commercial use is not permitted.
