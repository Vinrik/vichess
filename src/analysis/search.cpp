#include "search.h"
#include "evaluation.h"
#include <limits>
#include <algorithm>
#include <iostream>

extern std::vector<Move> generateLegalMoves(const Board &);

int minimax(Board board, int depth, int alpha, int beta, bool maximizingPlayer){
    auto moves=generateLegalMoves(board);

    // base case
    if(depth==0 || moves.empty()){
        if(moves.empty()){
            if(board.isKingInCheck(board.sideToMove)){
                // checkmate
                return maximizingPlayer? -1000000 : 1000000;
            } else {
                // stalemate
                return 0;
            }
        }
        return evaluate(board);
    }

    if(maximizingPlayer){
        int maxEval=std::numeric_limits<int>::min();
        for(auto &m: moves){
            Board child=board;
            bool ok=child.movePiece(m.fromRow, m.fromCol, m.toRow, m.toCol,
                                    m.isCapture, m.isEnPassant,
                                    m.promotionPiece, m.isCastling);
            if(!ok) continue;
            int eval=minimax(child, depth-1, alpha, beta, false);
            maxEval=std::max(maxEval, eval);
            alpha=std::max(alpha, eval);
            if(beta<=alpha) break;
        }
        return maxEval;
    } else {
        int minEval=std::numeric_limits<int>::max();
        for(auto &m: moves){
            Board child=board;
            bool ok=child.movePiece(m.fromRow, m.fromCol, m.toRow, m.toCol,
                                    m.isCapture, m.isEnPassant,
                                    m.promotionPiece, m.isCastling);
            if(!ok) continue;
            int eval=minimax(child, depth-1, alpha, beta, true);
            minEval=std::min(minEval, eval);
            beta=std::min(beta, eval);
            if(beta<=alpha) break;
        }
        return minEval;
    }
}

Move findBestMove(Board board, int maxDepth){
    Move best;
    int bestEval=std::numeric_limits<int>::min();

    auto moves=generateLegalMoves(board);
    if(moves.empty()) return best; // no moves

    for(int d=1; d<=maxDepth; d++){
        int localBestEval=std::numeric_limits<int>::min();
        Move localBestMove;
        for(auto &m: moves){
            Board child=board;
            bool ok=child.movePiece(m.fromRow, m.fromCol, m.toRow, m.toCol,
                                    m.isCapture, m.isEnPassant,
                                    m.promotionPiece, m.isCastling);
            if(!ok) continue;
            int eval=minimax(child, d-1, std::numeric_limits<int>::min(),
                             std::numeric_limits<int>::max(), false);
            if(eval>localBestEval){
                localBestEval=eval;
                localBestMove=m;
            }
        }
        bestEval=localBestEval;
        best=localBestMove;
        std::cout<<"[Depth="<<d<<"] bestEval="<<bestEval<<"\n";
    }
    return best;
}
