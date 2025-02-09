#include <gtest/gtest.h>
#include "search.h"
#include "evaluation.h"

TEST(AnalysisTest, EvaluateEmptyBoard){
    Board b;
    for(int r=0;r<8;r++){
        for(int c=0;c<8;c++){
            b.board[r][c].type=PieceType::None;
            b.board[r][c].color=Color::None;
        }
    }
    int sc=evaluate(b);
    EXPECT_EQ(sc,0);
}

TEST(AnalysisTest, MinimaxShallow){
    Board b;
    b.init();
    int val=minimax(b,1,-999999,999999,true);
    // Just check no crash + plausible range
    EXPECT_GE(val,-20000);
    EXPECT_LE(val,20000);
}

TEST(AnalysisTest, FindBestMoveInitial){
    Board b; b.init();
    Move m=findBestMove(b,1);
    bool moveOk=b.movePiece(m.fromRow,m.fromCol,m.toRow,m.toCol,
                            m.isCapture,m.isEnPassant,m.promotionPiece,m.isCastling);
    EXPECT_TRUE(moveOk);
}
