#include <gtest/gtest.h>
#include "board.h"
#include "moves.h"

TEST(VisualizerTest, PawnMoveForward) {
    Board board;
    board.init();
    // White Pawn e2->e4 => e2 is row=6,col=4
    bool moveOk=board.movePiece(6,4,4,4,false,false,PieceType::None,false);
    EXPECT_TRUE(moveOk);
    EXPECT_EQ(board.board[4][4].type, PieceType::Pawn);
    EXPECT_EQ(board.board[6][4].type, PieceType::None);
}

TEST(VisualizerTest, PawnDoubleMoveBlocked){
    Board board;
    board.init();
    // block e3
    board.board[5][4] = Piece(PieceType::Rook, Color::White);

    bool moveOk=board.movePiece(6,4,4,4,false,false,PieceType::None,false);
    EXPECT_FALSE(moveOk);
}

TEST(VisualizerTest, KnightMove){
    Board board; board.init();
    // White Knight g1->f3 => g1=(7,6), f3=(5,5)
    bool moveOk=board.movePiece(7,6,5,5,false,false,PieceType::None,false);
    EXPECT_TRUE(moveOk);
    EXPECT_EQ(board.board[5][5].type, PieceType::Knight);
    EXPECT_EQ(board.board[7][6].type, PieceType::None);
}

TEST(VisualizerTest, EnPassant){
    Board board; board.init();
    // e2->e4
    board.movePiece(6,4,4,4,false,false,PieceType::None,false);
    // e7->e6
    board.movePiece(1,4,2,4,false,false,PieceType::None,false);
    // e4->e5
    board.movePiece(4,4,3,4,false,false,PieceType::None,false);
    // d7->d5
    board.movePiece(1,3,3,3,false,false,PieceType::None,false);

    // e5->d4 en passant
    bool moveOk=board.movePiece(3,4,2,3,true,true,PieceType::None,false);
    EXPECT_TRUE(moveOk);
    EXPECT_EQ(board.board[2][3].type, PieceType::Pawn);
    EXPECT_EQ(board.board[3][3].type, PieceType::None);
}

TEST(VisualizerTest, CastlingKingside){
    Board board; board.init();
    // Clear f1,g1
    board.board[7][5]=Piece();
    board.board[7][6]=Piece();

    bool moveOk=board.movePiece(7,4,7,6,false,false,PieceType::None,true);
    EXPECT_TRUE(moveOk);
    EXPECT_EQ(board.board[7][6].type, PieceType::King);
    EXPECT_EQ(board.board[7][5].type, PieceType::Rook);
}

TEST(VisualizerTest, GenerateLegalMoves_StalematePosition){
    Board board;
    // clear everything
    for(int r=0;r<8;r++){
        for(int c=0;c<8;c++){
            board.board[r][c]=Piece();
        }
    }
    // black king in corner a8 => row=0,col=0
    board.board[0][0]=Piece(PieceType::King, Color::Black);
    // white king at b6 => row=2,col=1
    board.board[2][1]=Piece(PieceType::King, Color::White);
    // white pawn c6 => row=1,col=2
    board.board[1][2]=Piece(PieceType::Pawn, Color::White);

    board.sideToMove=Color::Black;
    EXPECT_TRUE(isStalemate(board));
}
