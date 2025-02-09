#include "gamestate.h"
#include "notation.h"
#include <iostream>

GameState::GameState(){
    // Default is White=Human named "White", Black=Human named "Black"
    whitePlayer.name = "White";
    whitePlayer.type = PlayerType::Human;

    blackPlayer.name = "Black";
    blackPlayer.type = PlayerType::Human;

    newGame();
}

void GameState::newGame() {
    board.init();
    history.clear();
}

void GameState::newGame(PlayerType whiteType, const std::string &whiteName,
                        PlayerType blackType, const std::string &blackName)
{
    board.init();
    history.clear();

    whitePlayer.name = whiteName;
    whitePlayer.type = whiteType;

    blackPlayer.name = blackName;
    blackPlayer.type = blackType;
}

bool GameState::makeMove(const Move &m)
{
    // If we are not currently at the end of the move list, erase future moves
    // Suppose we track a "currentIndex" that says "which move are we on?"
    // In your code, you might store replayIndex in the MainWindow instead. 
    // That’s okay — you just need to pass it in or handle it similarly.

    // Let's assume we keep a 'currentIndex' in GameState or we figure it from history.size().
    // For simplicity, let's do this:
    int currentIndex = (int)history.size(); // or the index you are 'on' if you track it that way

    // If we are in the middle, erase moves beyond currentIndex
    if(currentIndex < (int)history.size()) {
        history.erase(history.begin() + currentIndex, history.end());
    }

    // Now we attempt the board move:
    std::string fenBefore = board.toFEN();
    bool ok = board.movePiece(m.fromRow, m.fromCol, m.toRow, m.toCol,
                              m.isCapture, m.isEnPassant, m.promotionPiece, m.isCastling);
    if(!ok) return false;

    generateAndStoreMoveRecord(m, fenBefore);
    return true;
}


void GameState::generateAndStoreMoveRecord(const Move &m, const std::string &fenBefore){
    // build SAN
    Board temp;
    temp.loadFEN(fenBefore);
    std::string san = buildSAN(temp,m);

    MoveRecord rec;
    rec.move = m;
    rec.notation = san;
    rec.fenBefore = fenBefore;
    rec.fenAfter  = board.toFEN();
    history.push_back(rec);
}

bool GameState::loadFEN(const std::string &fen){
    return board.loadFEN(fen);
}

bool GameState::jumpToMoveIndex(int idx) {
 //   if(idx < 0) idx = 0;
  //  if(idx > (int)history.size()) idx = history.size();

    // If idx=0 => load initial position from scratch
    board.init();
    // 2) Clear the repetition-tracking info
    board.boardHistory.clear();
    // 3) Record the initial position in boardHistory
    board.updateBoardHistory();

    // 4) Re-apply moves up to idx
    for(int i=0; i<idx; i++){
        const auto &r = history[i];
        board.movePiece(r.move.fromRow, r.move.fromCol,
                        r.move.toRow,   r.move.toCol,
                        r.move.isCapture, r.move.isEnPassant,
                        r.move.promotionPiece, r.move.isCastling);
        // after each move, update repetition info
        board.updateBoardHistory();
    }
    return true;
}
