#pragma once

#include <string>
#include <unordered_map>

enum class PieceType { None, King, Queen, Rook, Bishop, Knight, Pawn };
enum class Color { None, White, Black };

struct Piece {
    PieceType type;
    Color color;
    bool hasMoved;
    Piece();
    Piece(PieceType t, Color c);
};

class Board {
public:
    // 8x8 array
    Piece board[8][8];

    Color sideToMove;

    // Castling flags
    bool whiteKingMoved, whiteRookKingsideMoved, whiteRookQueensideMoved;
    bool blackKingMoved, blackRookKingsideMoved, blackRookQueensideMoved;

    // En passant target square
    int enPassantRow, enPassantCol;

    // 50-move rule counter
    int moveCounter;
    int DRAW_RULE_LIMIT;

    // Threefold repetition
    std::unordered_map<std::string, int> boardHistory;

    Board();
    void init();
    void printBoard();

    // Move logic
    bool movePiece(int fromRow, int fromCol, int toRow, int toCol,
                   bool isCapture=false, bool isEnPassant=false,
                   PieceType promotionPiece=PieceType::None, bool isCastling=false);

    // Checks
    bool isKingInCheck(Color color) const;

    // FEN I/O
    std::string toFEN() const;
    bool loadFEN(const std::string &fen);

    // Internal bookkeeping
    void updateBoardHistory();
    bool checkDrawConditions();

private:
    bool validateMove(int fromRow, int fromCol, int toRow, int toCol,
                      bool isCapture, bool isEnPassant,
                      PieceType promotionPiece, bool isCastling);

    bool isPathClear(int fromRow, int fromCol, int toRow, int toCol) const;
    bool canPieceAttack(int fromRow, int fromCol, int toRow, int toCol) const;
};
