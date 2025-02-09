#pragma once
#include <vector>
#include "board.h"

struct Move {
    int fromRow, fromCol;
    int toRow, toCol;
    bool isCapture;
    bool isEnPassant;
    bool isCastling;
    PieceType promotionPiece;

    Move()
      : fromRow(0), fromCol(0), toRow(0), toCol(0),
        isCapture(false), isEnPassant(false),
        isCastling(false), promotionPiece(PieceType::None)
    {}

    Move(int fr, int fc, int tr, int tc,
         bool cap=false, bool enPassant=false,
         PieceType promo=PieceType::None, bool castle=false);
};

std::vector<Move> generateLegalMoves(const Board &b);

bool isStalemate(const Board &b);
bool isInsufficientMaterial(const Board &b);
