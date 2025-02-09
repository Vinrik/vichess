#include "board.h"
#include <iostream>
#include <cctype>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <vector>
#include "debug_log.h"

Piece::Piece() : type(PieceType::None), color(Color::None), hasMoved(false) {}
Piece::Piece(PieceType t, Color c) : type(t), color(c), hasMoved(false) {}

Board::Board() {
    init();
}

void Board::init() {
    // Clear board
    for(int r=0; r<8; r++){
        for(int c=0; c<8; c++){
            board[r][c] = Piece();
        }
    }

    // Black pieces
    board[0][0] = Piece(PieceType::Rook, Color::Black);
    board[0][1] = Piece(PieceType::Knight, Color::Black);
    board[0][2] = Piece(PieceType::Bishop, Color::Black);
    board[0][3] = Piece(PieceType::Queen, Color::Black);
    board[0][4] = Piece(PieceType::King, Color::Black);
    board[0][5] = Piece(PieceType::Bishop, Color::Black);
    board[0][6] = Piece(PieceType::Knight, Color::Black);
    board[0][7] = Piece(PieceType::Rook, Color::Black);
    for(int c=0; c<8; c++){
        board[1][c] = Piece(PieceType::Pawn, Color::Black);
    }

    // White pieces
    board[7][0] = Piece(PieceType::Rook, Color::White);
    board[7][1] = Piece(PieceType::Knight, Color::White);
    board[7][2] = Piece(PieceType::Bishop, Color::White);
    board[7][3] = Piece(PieceType::Queen, Color::White);
    board[7][4] = Piece(PieceType::King, Color::White);
    board[7][5] = Piece(PieceType::Bishop, Color::White);
    board[7][6] = Piece(PieceType::Knight, Color::White);
    board[7][7] = Piece(PieceType::Rook, Color::White);
    for(int c=0; c<8; c++){
        board[6][c] = Piece(PieceType::Pawn, Color::White);
    }

    sideToMove = Color::White;
    whiteKingMoved = whiteRookKingsideMoved = whiteRookQueensideMoved = false;
    blackKingMoved = blackRookKingsideMoved = blackRookQueensideMoved = false;
    enPassantRow = enPassantCol = -1;
    moveCounter = 0;
    DRAW_RULE_LIMIT = 50;
    boardHistory.clear();
    updateBoardHistory();
}

void Board::printBoard() {
    std::cout << "  A B C D E F G H\n";
    for (int r = 0; r < 8; r++) {
        std::cout << (8-r) << " ";
        for (int c = 0; c < 8; c++) {
            Piece p = board[r][c];
            char symbol = '.';
            if (p.type != PieceType::None) {
                switch(p.type){
                    case PieceType::King:   symbol='K'; break;
                    case PieceType::Queen:  symbol='Q'; break;
                    case PieceType::Rook:   symbol='R'; break;
                    case PieceType::Bishop: symbol='B'; break;
                    case PieceType::Knight: symbol='N'; break;
                    case PieceType::Pawn:   symbol='P'; break;
                    default: break;
                }
                if(p.color==Color::Black) symbol = std::tolower(symbol);
            }
            std::cout << symbol << " ";
        }
        std::cout << (8-r) << "\n";
    }
    std::cout << "  A B C D E F G H\n";
}

bool Board::isPathClear(int fromRow, int fromCol, int toRow, int toCol) const {
    int rowDir = (toRow - fromRow)==0 ? 0 : (toRow - fromRow)/std::abs(toRow - fromRow);
    int colDir = (toCol - fromCol)==0 ? 0 : (toCol - fromCol)/std::abs(toCol - fromCol);
    int r = fromRow + rowDir;
    int c = fromCol + colDir;

    while(r!=toRow || c!=toCol){
        if(board[r][c].type!=PieceType::None) return false;
        r += rowDir;
        c += colDir;
    }
    return true;
}

bool Board::canPieceAttack(int fromRow, int fromCol, int toRow, int toCol) const {
    Piece p = board[fromRow][fromCol];
    if(p.type==PieceType::None) return false;
    int dRow = toRow - fromRow;
    int dCol = toCol - fromCol;

    switch(p.type){
        case PieceType::Pawn:{
            int forward = (p.color==Color::White) ? -1 : 1;
            // Pawn capture is diagonal
            return (dRow==forward && std::abs(dCol)==1);
        }
        case PieceType::Knight:
            return ((std::abs(dRow)==2 && std::abs(dCol)==1) ||
                    (std::abs(dRow)==1 && std::abs(dCol)==2));
        case PieceType::Bishop:
            return (std::abs(dRow)==std::abs(dCol)) && isPathClear(fromRow,fromCol,toRow,toCol);
        case PieceType::Rook:
            return ((dRow==0 || dCol==0)) && isPathClear(fromRow,fromCol,toRow,toCol);
        case PieceType::Queen:
            return (((std::abs(dRow)==std::abs(dCol)) || (dRow==0||dCol==0))
                    && isPathClear(fromRow,fromCol,toRow,toCol));
        case PieceType::King:
            return (std::abs(dRow)<=1 && std::abs(dCol)<=1);
        default:
            return false;
    }
}

bool Board::isKingInCheck(Color color) const {
    // find king
    int kr=-1, kc=-1;
    for(int r=0; r<8; r++){
        for(int c=0; c<8; c++){
            if(board[r][c].type==PieceType::King && board[r][c].color==color){
                kr=r; kc=c;
                break;
            }
        }
    }
    if(kr<0) return false; // no king found, unusual scenario

    // check if any opponent piece can attack that square
    Color opp = (color==Color::White)? Color::Black : Color::White;
    for(int r=0; r<8; r++){
        for(int c=0; c<8; c++){
            if(board[r][c].color==opp){
                if(canPieceAttack(r,c,kr,kc)){
                    return true;
                }
            }
        }
    }
    return false;
}

bool Board::validateMove(int fromRow, int fromCol, int toRow, int toCol,
                         bool isCapture, bool isEnPassant,
                         PieceType promotionPiece, bool isCastling)
{
    if(fromRow<0||fromRow>7||fromCol<0||fromCol>7 ||
       toRow<0||toRow>7||toCol<0||toCol>7) return false;

    Piece moving = board[fromRow][fromCol];
    if(moving.type==PieceType::None || moving.color!=sideToMove) return false;

    Piece target = board[toRow][toCol];
    if(!isCastling){
        if(isCapture){
            if(!isEnPassant){
                // normal capture => target must be an enemy piece
                if(target.type==PieceType::None || target.color==moving.color) return false;
            }
        } else {
            // no capture => target must be empty
            if(target.type!=PieceType::None) return false;
        }
    }

    int dRow = toRow - fromRow;
    int dCol = toCol - fromCol;

    switch(moving.type){
        case PieceType::Pawn:{
            int forward = (moving.color==Color::White)? -1 : 1;
            int startRow= (moving.color==Color::White)? 6 : 1;
            if(isCapture || isEnPassant){
                if(dRow==forward && std::abs(dCol)==1){
                    if(isEnPassant){
                        // must match enPassant coords
                        if(toRow!=enPassantRow || toCol!=enPassantCol) return false;
                    }
                    return true;
                }
                return false;
            } else {
                // normal pawn move
                if(dCol==0){
                    if(dRow==forward && target.type==PieceType::None){
                        return true;
                    }
                    else if(dRow==2*forward && fromRow==startRow){
                        int mid = fromRow+forward;
                        if(board[mid][fromCol].type==PieceType::None &&
                           target.type==PieceType::None){
                            return true;
                        }
                    }
                }
                return false;
            }
        }
        case PieceType::Knight:
            return ((std::abs(dRow)==2 && std::abs(dCol)==1) ||
                    (std::abs(dRow)==1 && std::abs(dCol)==2));
        case PieceType::Bishop:
            return (std::abs(dRow)==std::abs(dCol)) && isPathClear(fromRow,fromCol,toRow,toCol);
        case PieceType::Rook:
            return ((dRow==0||dCol==0)) && isPathClear(fromRow,fromCol,toRow,toCol);
        case PieceType::Queen:
            return (((std::abs(dRow)==std::abs(dCol))||(dRow==0||dCol==0))
                    && isPathClear(fromRow,fromCol,toRow,toCol));
        case PieceType::King:{
            if(isCastling){
                // castling checks
                if(moving.hasMoved || isKingInCheck(moving.color)) return false;
                if(dRow!=0) return false;
                // kingside or queenside
                if(dCol==2){
                    // kingside
                    int row=fromRow;
                    // check rook at col=7
                    if(board[row][7].type!=PieceType::Rook || board[row][7].color!=moving.color)
                        return false;
                    if(!isPathClear(fromRow,fromCol,row,7)) return false;
                    // no passing through check
                    for(int cc=fromCol+1; cc<=fromCol+2; cc++){
                        Board temp = *this;
                        temp.board[row][cc] = temp.board[row][fromCol];
                        temp.board[row][fromCol] = Piece();
                        if(temp.isKingInCheck(moving.color)) return false;
                    }
                    return true;
                } else if(dCol==-2){
                    // queenside
                    int row=fromRow;
                    if(board[row][0].type!=PieceType::Rook || board[row][0].color!=moving.color)
                        return false;
                    if(!isPathClear(fromRow,fromCol,row,0)) return false;
                    for(int cc=fromCol-1; cc>=fromCol-2; cc--){
                        Board temp = *this;
                        temp.board[row][cc] = temp.board[row][fromCol];
                        temp.board[row][fromCol] = Piece();
                        if(temp.isKingInCheck(moving.color)) return false;
                    }
                    return true;
                }
                return false;
            } else {
                // normal king move
                return (std::abs(dRow)<=1 && std::abs(dCol)<=1);
            }
        }
        default:
            return false;
    }
}

bool Board::movePiece(int fromRow, int fromCol, int toRow, int toCol,
                      bool isCapture, bool isEnPassant,
                      PieceType promotionPiece, bool isCastling)
{
    // Debug: show parameters
    DEBUG_LOG("[movePiece] sideToMove="
              << " from(" << fromRow << "," << fromCol 
              << ") to("   << toRow << "," << toCol   << ")"
              << " isCapture=" << (isCapture?"true":"false")
              << " isEnPassant=" << (isEnPassant?"true":"false")
              << " promotionPiece=" << (int)promotionPiece
              << " isCastling=" << (isCastling?"true":"false")
              << std::endl;
    );
    // First, validate the move
    if(!validateMove(fromRow, fromCol, toRow, toCol,
                     isCapture, isEnPassant, promotionPiece, isCastling))
    {
    DEBUG_LOG("[movePiece] validateMove FAILED\n");

        return false;
    }

    // We'll copy the board so we can apply the move
    Board temp = *this;

    // The piece that is moving
    Piece movedPiece = temp.board[fromRow][fromCol];

    // ---------------------
    // 1) Castling case
    // ---------------------
    if(isCastling && movedPiece.type == PieceType::King) {
        temp.board[toRow][toCol] = movedPiece;
        temp.board[fromRow][fromCol] = Piece(); // empty original

        // kingside
        if(toCol == fromCol + 2) {
            temp.board[fromRow][toCol - 1] = temp.board[fromRow][7];
            temp.board[fromRow][7] = Piece();
        }
        // queenside
        else if(toCol == fromCol - 2) {
            temp.board[fromRow][toCol + 1] = temp.board[fromRow][0];
            temp.board[fromRow][0] = Piece();
        }
    }
    // ---------------------
    // 2) Normal / en passant
    // ---------------------
    else {
        // Move the piece
        temp.board[toRow][toCol] = movedPiece;
        temp.board[fromRow][fromCol] = Piece();

        // If it's en passant, remove the captured pawn behind
        if(isEnPassant) {
            int capRow = (sideToMove == Color::White) ? (toRow + 1) : (toRow - 1);
            temp.board[capRow][toCol] = Piece();
		DEBUG_LOG("[movePiece] EnPassant: removing captured pawn at ("
			  << capRow << "," << toCol << ")");

        }
    }

    // ---------------------
    // 3) Pawn promotion
    // ---------------------
    if(temp.board[toRow][toCol].type == PieceType::Pawn) {
        int promoRow = (temp.board[toRow][toCol].color == Color::White) ? 0 : 7;
        if(toRow == promoRow) {
            if(promotionPiece == PieceType::None) {
                promotionPiece = PieceType::Queen; // default to queen
            }
            temp.board[toRow][toCol].type = promotionPiece;
	    DEBUG_LOG("[movePiece] Pawn promoted to " << (int)promotionPiece);

        }
    }

    // ---------------------
    // 4) Check if results in self-check
    // ---------------------
    if(temp.isKingInCheck(sideToMove)) {
	DEBUG_LOG("[movePiece] STILL in check after applying => FAIL");

        return false;
    }

    // ---------------------
    // 5) If the moved pawn advanced 2 squares, set enPassantRow
    //    Otherwise, reset to -1.
    // ---------------------
    if(movedPiece.type == PieceType::Pawn && std::abs(toRow - fromRow) == 2) {
        temp.enPassantRow = (fromRow + toRow)/2;
        temp.enPassantCol = fromCol;
	DEBUG_LOG("[movePiece] Setting enPassant => row="
		  << temp.enPassantRow << " col=" << temp.enPassantCol);

    } else {
        temp.enPassantRow = temp.enPassantCol = -1;
	DEBUG_LOG("[movePiece] Resetting enPassant => -1");

    }

    // Copy temp changes back to 'this'
    *this = temp;

    // ---------------------
    // 6) Update castling flags, 50-move, etc.
    // ---------------------
    if(movedPiece.type == PieceType::King) {
        if(movedPiece.color == Color::White) whiteKingMoved = true;
        else blackKingMoved = true;
    }
    if(movedPiece.type == PieceType::Rook) {
        if(movedPiece.color == Color::White) {
            if(fromRow == 7 && fromCol == 0) whiteRookQueensideMoved = true;
            if(fromRow == 7 && fromCol == 7) whiteRookKingsideMoved  = true;
        } else {
            if(fromRow == 0 && fromCol == 0) blackRookQueensideMoved = true;
            if(fromRow == 0 && fromCol == 7) blackRookKingsideMoved  = true;
        }
    }

    if(movedPiece.type == PieceType::Pawn || isCapture || isEnPassant) {
        moveCounter = 0;
    } else {
        moveCounter++;
    }

    // Flip side
	DEBUG_LOG("[movePiece] SUCCESS => flipping side from "
		  << ((sideToMove==Color::White)?"White":"Black")
		  << " to "
		  << ((sideToMove==Color::White)?"White":"Black");
                  << ((sideToMove==Color::White)?"White":"Black"));
    sideToMove = (sideToMove==Color::White) ? Color::Black : Color::White;


    // Finally, update board history & check draws
    updateBoardHistory();
    checkDrawConditions();
    return true;
}

std::string Board::toFEN() const {
    std::stringstream fen;
    // piece placement
    for(int row=0; row<8; row++){
        int emptyCount=0;
        for(int col=0; col<8; col++){
            const Piece &p = board[row][col];
            if(p.type==PieceType::None){
                emptyCount++;
            } else {
                if(emptyCount>0){
                    fen << emptyCount;
                    emptyCount=0;
                }
                char symbol='?';
                switch(p.type){
                    case PieceType::King:   symbol='K'; break;
                    case PieceType::Queen:  symbol='Q'; break;
                    case PieceType::Rook:   symbol='R'; break;
                    case PieceType::Bishop: symbol='B'; break;
                    case PieceType::Knight: symbol='N'; break;
                    case PieceType::Pawn:   symbol='P'; break;
                    default: symbol='?';    break;
                }
                if(p.color==Color::Black){
                    symbol=std::tolower(symbol);
                }
                fen << symbol;
            }
        }
        if(emptyCount>0) fen<<emptyCount;
        if(row<7) fen<<'/';
    }

    // side to move
    fen<<" ";
    fen<<( (sideToMove==Color::White)? 'w':'b');

    // castling
    fen<<" ";
    bool anyCastling=false;
    if(!whiteKingMoved && !whiteRookKingsideMoved) {fen<<"K"; anyCastling=true;}
    if(!whiteKingMoved && !whiteRookQueensideMoved){fen<<"Q"; anyCastling=true;}
    if(!blackKingMoved && !blackRookKingsideMoved) {fen<<"k"; anyCastling=true;}
    if(!blackKingMoved && !blackRookQueensideMoved){fen<<"q"; anyCastling=true;}
    if(!anyCastling) fen<<"-";

    // enPassant
    fen<<" ";
    if(enPassantRow==-1) fen<<"-";
    else {
        char fileChar = 'a'+enPassantCol;
        int rank=8-enPassantRow;
        fen<<fileChar<<rank;
    }

    // halfmove clock
    fen<<" "<<moveCounter;
    // fullmove number (not fully tracked, so we put 1)
    fen<<" 1";

    return fen.str();
}

bool Board::loadFEN(const std::string &fen) {
    // minimal parse
    std::istringstream iss(fen);
    std::string piecePlacement, activeColor, castling, enPassant, halfmove, fullmove;
    iss >> piecePlacement >> activeColor >> castling >> enPassant >> halfmove >> fullmove;

    // parse piecePlacement
    std::vector<std::string> ranks;
    {
        std::stringstream pp(piecePlacement);
        std::string rank;
        while(std::getline(pp, rank, '/')){
            ranks.push_back(rank);
        }
        if(ranks.size()!=8){
            std::cerr << "FEN parse error\n";
            return false;
        }
    }

    for(int r=0; r<8; r++){
        int col=0;
        for(char c: ranks[r]){
            if(std::isdigit(c)){
                int empties=c-'0';
                for(int e=0; e<empties; e++){
                    board[r][col]=Piece();
                    col++;
                }
            } else {
                Color colr = std::isupper(c)? Color::White: Color::Black;
                char uc=std::toupper(c);
                PieceType pt=PieceType::None;
                switch(uc){
                    case 'K': pt=PieceType::King; break;
                    case 'Q': pt=PieceType::Queen; break;
                    case 'R': pt=PieceType::Rook; break;
                    case 'B': pt=PieceType::Bishop; break;
                    case 'N': pt=PieceType::Knight; break;
                    case 'P': pt=PieceType::Pawn; break;
                    default: pt=PieceType::None; break;
                }
                board[r][col]= Piece(pt,colr);
                col++;
            }
        }
    }

    sideToMove=(activeColor=="w"? Color::White:Color::Black);

    // castling
    whiteKingMoved=whiteRookKingsideMoved=whiteRookQueensideMoved=false;
    blackKingMoved=blackRookKingsideMoved=blackRookQueensideMoved=false;
    if(castling.find('K')==std::string::npos){
        whiteKingMoved=true;
    }
    if(castling.find('Q')==std::string::npos){
        whiteKingMoved=true;
    }
    if(castling.find('k')==std::string::npos){
        blackKingMoved=true;
    }
    if(castling.find('q')==std::string::npos){
        blackKingMoved=true;
    }

    enPassantRow=enPassantCol=-1;
    if(enPassant!="-" && enPassant.size()>=2){
        char fileChar=enPassant[0];
        int rank=enPassant[1]-'0';
        enPassantCol=fileChar-'a';
        enPassantRow=8-rank;
    }

    if(!halfmove.empty()){
        moveCounter=std::stoi(halfmove);
    } else {
        moveCounter=0;
    }

    updateBoardHistory();
    return true;
}

void Board::updateBoardHistory(){
    // track threefold
    std::stringstream ss;
    ss<<toFEN();
    boardHistory[ss.str()]++;
}

bool Board::checkDrawConditions(){
    if(moveCounter>=DRAW_RULE_LIMIT){
        std::cout<<"Draw by "<<DRAW_RULE_LIMIT<<"-move rule.\n";
        return true;
    }
    std::string fen=toFEN();
    if(boardHistory[fen]>=3){
        std::cout<<"Draw by threefold repetition.\n";
        return true;
    }
    return false;
}
