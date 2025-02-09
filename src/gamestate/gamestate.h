#pragma once
#include <vector>
#include <string>
#include "board.h"
#include "moves.h"

// Extra info about each move
struct MoveRecord {
    Move move;
    std::string notation;   // SAN
    std::string fenBefore;
    std::string fenAfter;
};

// Player type
enum class PlayerType {
    Human,
    Engine
};

struct PlayerInfo {
    std::string name;
    PlayerType type;
    // optional rating, etc.
};

class GameState {
public:
    GameState();

    // Original newGame() => defaults to "White vs Black (both human)"
    void newGame();

    // Overloaded newGame => specify which players are Human or Engine + names
    void newGame(PlayerType whiteType, const std::string &whiteName,
                 PlayerType blackType, const std::string &blackName);

    bool makeMove(const Move &m);
    bool loadFEN(const std::string &fen);

    const Board& currentBoard() const { return board; }
    const std::vector<MoveRecord>& moveHistory() const { return history; }

    // Jump to any move index: 0..history.size()
    bool jumpToMoveIndex(int idx);

    // Player profiles
    PlayerInfo whitePlayer;
    PlayerInfo blackPlayer;

private:
    Board board;
    std::vector<MoveRecord> history;

    void generateAndStoreMoveRecord(const Move &m, const std::string &fenBefore);
};
