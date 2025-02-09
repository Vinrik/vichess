#include "notation.h"
#include "board.h"
#include "moves.h"
#include <sstream>
#include <cctype>

// We'll assume generateLegalMoves(...) is declared somewhere
extern std::vector<Move> generateLegalMoves(const Board &);

std::string buildSAN(const Board &boardBefore, const Move &move)
{
    // 1) If castling
    if(move.isCastling){
        if(move.toCol > move.fromCol) {
            return "O-O";   // kingside
        } else {
            return "O-O-O"; // queenside
        }
    }

    // 2) Identify the piece moving
    Piece p = boardBefore.board[move.fromRow][move.fromCol];
    bool isCapture = move.isCapture;
    bool isPawn    = (p.type == PieceType::Pawn);

    char pieceLetter = '\0';
    switch(p.type){
        case PieceType::King:   pieceLetter = 'K'; break;
        case PieceType::Queen:  pieceLetter = 'Q'; break;
        case PieceType::Rook:   pieceLetter = 'R'; break;
        case PieceType::Bishop: pieceLetter = 'B'; break;
        case PieceType::Knight: pieceLetter = 'N'; break;
        default: pieceLetter = '\0'; /* for Pawn */
    }

    std::string san;

    // If it's not a pawn, add the piece letter
    if (pieceLetter != '\0') {
        san.push_back(pieceLetter);
    }

    // 3) Pawn capture => include the from-file letter
    if (isPawn && isCapture) {
        char fileFrom = 'a' + move.fromCol;
        san.push_back(fileFrom);
    }

    // 4) 'x' if capture
    if (isCapture) {
        san.push_back('x');
    }

    // 5) Destination
    char fileTo = 'a' + move.toCol;
    int rankTo  = 8 - move.toRow;
    san.push_back(fileTo);
    san += std::to_string(rankTo);

    // 6) If it's an enPassant capture, append " e.p."
    if (move.isEnPassant) {
        san += " e.p.";
    }

    // 7) Promotion
    if (move.promotionPiece != PieceType::None) {
        san.push_back('=');
        char promo='Q'; 
        switch(move.promotionPiece){
            case PieceType::Rook:   promo='R'; break;
            case PieceType::Bishop: promo='B'; break;
            case PieceType::Knight: promo='N'; break;
            default: promo='Q'; break; 
        }
        san.push_back(promo);
    }

    // 8) Check or mate
    {
        // We'll apply the move on a temp board
        Board temp = boardBefore;
        temp.movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol,
                       move.isCapture, move.isEnPassant, move.promotionPiece, move.isCastling);

        // Opponent color
        Color moverColor = p.color;
        Color oppColor   = (moverColor == Color::White) ? Color::Black : Color::White;

        bool oppInCheck = temp.isKingInCheck(oppColor);

        // If the opponent has no moves => possibly checkmate or stalemate
        auto oppMoves = generateLegalMoves(temp);

        if(oppInCheck && oppMoves.empty()) {
            // checkmate
            san.push_back('#');
        } else if(oppInCheck) {
            // just check
            san.push_back('+');
        }
    }

    return san;
}
