#include <gtest/gtest.h>
#include "board.h"
#include "moves.h"
#include <iostream>
#include "debug_log.h"

static void clearBoard(Board &b) {
    for(int r=0; r<8; r++){
        for(int c=0; c<8; c++){
            b.board[r][c].type = PieceType::None;
            b.board[r][c].color= Color::None;
            b.board[r][c].hasMoved = false;
        }
    }
    b.sideToMove = Color::White;
    b.whiteKingMoved = b.whiteRookKingsideMoved = b.whiteRookQueensideMoved = false;
    b.blackKingMoved = b.blackRookKingsideMoved = b.blackRookQueensideMoved = false;
    b.enPassantRow = b.enPassantCol = -1;
    b.moveCounter = 0;
    b.boardHistory.clear();
    b.updateBoardHistory();
}

TEST(EnPassantTest, WhiteEnPassantCapture) {
    Board b;
    clearBoard(b);

    // White King on h1 => row=7,col=7
    b.board[7][7] = Piece(PieceType::King, Color::White);
    // Black King on a8 => row=0,col=0
    b.board[0][0] = Piece(PieceType::King, Color::Black);

    // White pawn on e5 => row=3,col=4
    b.board[3][4] = Piece(PieceType::Pawn, Color::White);

    // Black pawn on d7 => row=1,col=3
    b.board[1][3] = Piece(PieceType::Pawn, Color::Black);

    // We want to let Black move first => sideToMove=Black
    b.sideToMove = Color::Black;

    // Now do black's double push: d7->d5 => (1,3)->(3,3)
    bool blackDouble = b.movePiece(1,3,3,3, false,false, PieceType::None,false);
    ASSERT_TRUE(blackDouble) << "Black's double push d7->d5 should succeed";

    DEBUG_LOG("[DEBUG] After black's double push => enPassantRow="
              << b.enPassantRow << " enPassantCol=" << b.enPassantCol 
              << " sideToMove=" << ((b.sideToMove==Color::White)?"White":"Black") << std::endl;
    );

    // Expect enPassantRow=2, enPassantCol=3
    EXPECT_EQ(b.enPassantRow, 2) << "Should set enPassantRow=2 for black's double push from row=1->3.";
    EXPECT_EQ(b.enPassantCol, 3) << "Should set enPassantCol=3 for black's double push from col=3.";

    // sideToMove should be White now
    EXPECT_EQ(b.sideToMove, Color::White);

    // White's en passant capture: e5->d6 => (3,4)->(2,3)
    // Let's see if generateLegalMoves(...) includes that
    auto legal = generateLegalMoves(b);
    bool foundEnp = false;
    Move enpMove;
    for(const auto &m : legal) {
        if(m.fromRow==3 && m.fromCol==4 && m.toRow==2 && m.toCol==3
           && m.isCapture && m.isEnPassant)
        {
            foundEnp = true;
            enpMove = m;
            break;
        }
    }
    ASSERT_TRUE(foundEnp) << "generateLegalMoves should contain en passant from e5->d6 (3,4->2,3)";

    // Actually apply it
    bool whiteCaptures = b.movePiece(enpMove.fromRow, enpMove.fromCol, 
                                     enpMove.toRow,   enpMove.toCol,
                                     enpMove.isCapture, enpMove.isEnPassant,
                                     enpMove.promotionPiece, enpMove.isCastling);
    ASSERT_TRUE(whiteCaptures) << "White's en passant capture e5->d6 should succeed.";

    // Confirm White's pawn ended up at row=2,col=3 => d6
    EXPECT_EQ(b.board[2][3].type, PieceType::Pawn);
    EXPECT_EQ(b.board[2][3].color, Color::White);

    // The black pawn that hopped to (3,3) is removed
    EXPECT_EQ(b.board[3][3].type, PieceType::None)
        << "Black pawn should be removed after en passant";

    DEBUG_LOG("[DEBUG] White e.p. capture success => White pawn at d6 => (2,3)");

}
