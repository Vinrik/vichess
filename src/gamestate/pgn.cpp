#include "pgn.h"
#include <sstream>
#include <iostream>
#include "notation.h"
#include "moves.h"

// Very simplified PGN parse: doesn't handle variations, just linear moves
namespace PGN {

bool importPGN(const std::string &pgnText, GameState &game){
    // Clear current game
    game.newGame();

    // For simplicity, skip PGN headers, just parse moves
    // e.g. "1. e4 e5 2. Nf3 Nc6 ..."
    std::istringstream iss(pgnText);
    std::string token;

    while(iss >> token){
        // skip numbers like "1.", "2.", etc.
        if(token.find('.')!=std::string::npos) continue;
        if(token=="1-0"||token=="0-1"||token=="1/2-1/2") break; // game end

        // We have a SAN move. We'll do a naive approach:
        // 1) generate all legal moves from current position
        // 2) find which move matches this SAN
        const Board &b = game.currentBoard();
        auto moves = generateLegalMoves(b);
        bool found=false;
        for(const auto &m: moves){
            // build SAN
            Board temp=b;
            std::string sanBefore=temp.toFEN();
            temp.movePiece(m.fromRow,m.fromCol,m.toRow,m.toCol,
                           m.isCapture,m.isEnPassant,m.promotionPiece,m.isCastling);
            // to see SAN we must do: buildSAN(boardBefore,m)
            Board bBefore; 
            bBefore.loadFEN(sanBefore);
            std::string guess=buildSAN(bBefore,m);
            if(guess==token){
                // apply to actual game
                game.makeMove(m);
                found=true;
                break;
            }
        }
        if(!found){
            std::cerr<<"Could not find SAN move '"<<token<<"' in PGN import.\n";
            return false;
        }
    }
    return true;
}

std::string exportPGN(const GameState &game){
    // We'll add a minimal header
    std::ostringstream oss;
    oss << "[Event \"Local Game\"]\n";
    oss << "[Site \"Local\"]\n";
    oss << "[Date \"????.??.??\"]\n";
    oss << "[White \"" << game.whitePlayer.name << "\"]\n";
    oss << "[Black \"" << game.blackPlayer.name << "\"]\n\n";

    auto &hist = game.moveHistory();
    // print moves in pairs
    int fullMoveNumber = 1;
	for (size_t i = 0; i < hist.size(); /* manual increment */) {
	    // Print move number
	    oss << fullMoveNumber << ". ";

	    // White move
	    oss << hist[i].notation << " ";
	    i++; 
	    // If there is a black move, do it
	    if (i < hist.size()) {
		oss << hist[i].notation << " ";
		i++;
	    }

	    oss << "\n";  // new line after each White+Black pair
	    fullMoveNumber++;
	}
    oss << "\n";

    return oss.str();
}

} // namespace PGN
