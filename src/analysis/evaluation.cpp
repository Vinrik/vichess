#include "evaluation.h"

int evaluate(const Board &b){
    int score=0;

    const int pawnVal=100;
    const int knightVal=320;
    const int bishopVal=330;
    const int rookVal=500;
    const int queenVal=900;
    const int kingVal=20000;

    for(int r=0; r<8; r++){
        for(int c=0; c<8; c++){
            const Piece &p=b.board[r][c];
            if(p.type==PieceType::None) continue;
            int val=0;
            switch(p.type){
                case PieceType::Pawn:   val=pawnVal; break;
                case PieceType::Knight: val=knightVal; break;
                case PieceType::Bishop: val=bishopVal; break;
                case PieceType::Rook:   val=rookVal; break;
                case PieceType::Queen:  val=queenVal; break;
                case PieceType::King:   val=kingVal; break;
                default: break;
            }
            if(p.color==Color::White) score+=val;
            else score-=val;
        }
    }

    return score;
}
