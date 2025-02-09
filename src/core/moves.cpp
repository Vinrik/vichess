#include "moves.h"
#include <cmath>
#include <iostream> 
#include "debug_log.h"

// Constructor for a Move
Move::Move(int fr, int fc, int tr, int tc,
           bool cap, bool enPassant,
           PieceType promo, bool castle)
  : fromRow(fr), fromCol(fc), toRow(tr), toCol(tc),
    isCapture(cap), isEnPassant(enPassant),
    isCastling(castle), promotionPiece(promo)
{
}

// Generate all legal moves for the current side to move
std::vector<Move> generateLegalMoves(const Board &b) {
    std::vector<Move> moves;
    // debug
	DEBUG_LOG("[generateLegalMoves] sideToMove="
		  << ((b.sideToMove==Color::White)?"White":"Black"));

	DEBUG_LOG("[generateLegalMoves] enPassantRow=" << b.enPassantRow
		  << " enPassantCol=" << b.enPassantCol);

    // Loop over all squares
    for(int fr = 0; fr < 8; fr++){
        for(int fc = 0; fc < 8; fc++){
            Piece p = b.board[fr][fc];
            // Skip empty squares or squares that don't belong to sideToMove
            if(p.type == PieceType::None || p.color != b.sideToMove) 
                continue;

            // Attempt to move or capture on every possible (tr, tc)
            for(int tr = 0; tr < 8; tr++){
                for(int tc = 0; tc < 8; tc++){
                    // 1) Normal (non-capture) move
                    {
                        Board tempBoard = b;
                        // pass isCapture=false, isEnPassant=false
                        if(tempBoard.movePiece(fr, fc, tr, tc, false, false, PieceType::None, false)) {
                            moves.push_back(Move(fr, fc, tr, tc, false, false));
                        }
                    }
                    // 2) Capture move
                    {
                        Board tempBoard = b;
                        // pass isCapture=true, isEnPassant=false
                        if(tempBoard.movePiece(fr, fc, tr, tc, true, false, PieceType::None, false)) {
                            moves.push_back(Move(fr, fc, tr, tc, true, false));
                        }
                    }
                    // 3) En Passant
                    if(p.type == PieceType::Pawn && b.enPassantRow != -1) {
                        int forward = (p.color == Color::White)? -1 : 1;
                        if(fr + forward == b.enPassantRow && std::abs(fc - b.enPassantCol) == 1) {
                            Board copy = b;
                            if(copy.movePiece(fr, fc, b.enPassantRow, b.enPassantCol, true, true, PieceType::None, false)) {
				DEBUG_LOG("[generateLegalMoves] enPassantRow=" << b.enPassantRow
					  << " enPassantCol=" << b.enPassantCol);

                                moves.push_back(Move(fr, fc, b.enPassantRow, b.enPassantCol, true, true));
                            }
                        }
                    }
                    // 4) Castling (king moves 2 squares horizontally)
                    if(p.type == PieceType::King) {
                        // Kingside
                        {
                            Board tempBoard = b;
                            if(tempBoard.movePiece(fr, fc, fr, fc+2, false, false, PieceType::None, true)) {
                                moves.push_back(Move(fr, fc, fr, fc+2, false, false, PieceType::None, true));
                            }
                        }
                        // Queenside
                        {
                            Board tempBoard = b;
                            if(tempBoard.movePiece(fr, fc, fr, fc-2, false, false, PieceType::None, true)) {
                                moves.push_back(Move(fr, fc, fr, fc-2, false, false, PieceType::None, true));
                            }
                        }
                    }
                    // 5) Pawn promotion
                    if(p.type == PieceType::Pawn) {
                        int promotionRow = (p.color == Color::White) ? 0 : 7;
                        if(tr == promotionRow) {
                            for(PieceType pr : {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight}) {
                                Board tempBoard = b;
                                if(tempBoard.movePiece(fr, fc, tr, tc, false, false, pr, false)) {
                                    moves.push_back(Move(fr, fc, tr, tc, false, false, pr, false));
                                }
                            }
                        }
                    }
                } // tc
            } // tr
        } // fc
    } // fr

    return moves;
}

// Check if the current side to move is stalemated
bool isStalemate(const Board &b) {
    // If the king is in check, it's not stalemate
    if(b.isKingInCheck(b.sideToMove)) 
        return false;

    // If there are no legal moves, that's stalemate
    auto possibleMoves = generateLegalMoves(b);
    return possibleMoves.empty();
}

// Check if the position is insufficient material for mate
bool isInsufficientMaterial(const Board &b) {
    int whitePawns = 0, blackPawns = 0;
    int whiteRooks = 0, blackRooks = 0;
    int whiteQueens = 0, blackQueens = 0;
    int whiteMinors = 0, blackMinors = 0;

    for(int r = 0; r < 8; r++){
        for(int c = 0; c < 8; c++){
            Piece p = b.board[r][c];
            if(p.type == PieceType::None) continue;

            if(p.type == PieceType::Pawn) {
                (p.color==Color::White) ? whitePawns++ : blackPawns++;
            } else if(p.type == PieceType::Rook) {
                (p.color==Color::White) ? whiteRooks++ : blackRooks++;
            } else if(p.type == PieceType::Queen) {
                (p.color==Color::White) ? whiteQueens++ : blackQueens++;
            } else if(p.type==PieceType::Bishop || p.type==PieceType::Knight) {
                (p.color==Color::White) ? whiteMinors++ : blackMinors++;
            }
        }
    }

    // If either side still has pawns, rooks, or queens, or multiple minors => not insufficient
    if(whitePawns || blackPawns || whiteRooks || blackRooks || whiteQueens || blackQueens) {
        return false;
    }
    // Only knights/bishops remain
    // If both sides have no minor pieces => K vs K => insufficient
    if(whiteMinors == 0 && blackMinors == 0)
        return true;
    // K + single minor vs K => insufficient
    if((whiteMinors == 1 && blackMinors == 0) ||
       (blackMinors == 1 && whiteMinors == 0)) 
    {
        return true;
    }

    return false;
}
