#include "mainwindow.h"
#include <QVBoxLayout>
#include <QAction>
#include <QGraphicsPixmapItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QTimer>
#include <fstream>
#include "moves.h"
#include "pgn.h"  // for PGN::exportPGN etc.

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      engineManager(new EngineManager(this)),
      pieceSelected(false),
      selectedRow(-1),
      selectedCol(-1),
      replayIndex(0),
      engineTimer(new QTimer(this)),
      engineVsEngineActive(false),
      promotionMenu(nullptr) // Initialize to nullptr
{
    setupUI();

    connect(engineManager, &EngineManager::bestMoveReady,
            this, &MainWindow::handleEngineBestMove);

    engineManager->setEnginePath("/usr/bin/stockfish");
    engineManager->startEngine();

    // Default: new game => both humans
    gameState.newGame();

    // Timer logic for engine vs engine
    connect(engineTimer, &QTimer::timeout, this, [this](){
        if(!engineVsEngineActive) return;

        Board b = gameState.currentBoard();
        auto possible=generateLegalMoves(b);
        if(possible.empty()){
            // game ended
            engineTimer->stop();
            engineVsEngineActive=false;
            return;
        }
        Color stm=b.sideToMove;
        if(stm==Color::White && gameState.whitePlayer.type==PlayerType::Engine){
            onEngineMove();
        }
        else if(stm==Color::Black && gameState.blackPlayer.type==PlayerType::Engine){
            onEngineMove();
        }
    });

    drawBoard();
    updateMoveListDisplay();
}

MainWindow::~MainWindow(){
    if(promotionMenu) {
        delete promotionMenu;
        promotionMenu = nullptr;
    }
}

// ---------------------------------------------------
// UI SETUP
// ---------------------------------------------------
void MainWindow::setupUI(){
    toolbar = addToolBar("Main Toolbar");
    QAction *newGameAct = toolbar->addAction("New Game");
    connect(newGameAct, &QAction::triggered, this, &MainWindow::onNewGame);

    QAction *engineMoveAct = toolbar->addAction("Engine Move");
    connect(engineMoveAct, &QAction::triggered, this, &MainWindow::onEngineMove);

    QAction *startAct = toolbar->addAction("|<<");
    connect(startAct, &QAction::triggered, this, &MainWindow::onGoStart);

    QAction *prevAct = toolbar->addAction("<<");
    connect(prevAct, &QAction::triggered, this, &MainWindow::onPrevMove);

    QAction *nextAct = toolbar->addAction(">>");
    connect(nextAct, &QAction::triggered, this, &MainWindow::onNextMove);

    QAction *endAct = toolbar->addAction(">>|");
    connect(endAct, &QAction::triggered, this, &MainWindow::onGoEnd);

    // Start engine vs engine
    QAction *engineVsEngineAct = toolbar->addAction("Engine vs Engine");
    connect(engineVsEngineAct, &QAction::triggered, this, &MainWindow::onStartEngineVsEngine);

    // Save PGN
    QAction *savePGNAct = toolbar->addAction("Save PGN");
    connect(savePGNAct, &QAction::triggered, this, &MainWindow::onSavePGN);

    scene = new QGraphicsScene(this);
    view  = new QGraphicsView(scene,this);
    setCentralWidget(view);

    QDockWidget *dock = new QDockWidget("Moves",this);
    moveList = new QListWidget(dock);
    dock->setWidget(moveList);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    view->viewport()->installEventFilter(this);
}

// ---------------------------------------------------
// EVENT FILTER: Handling Clicks on the Board
// ---------------------------------------------------
bool MainWindow::eventFilter(QObject *obj, QEvent *event){
    if(obj==view->viewport() && event->type()==QEvent::MouseButtonPress){
        QMouseEvent *me=(QMouseEvent*)event;
        QPointF scenePos = view->mapToScene(me->pos());
        int col=int(scenePos.x()/64);
        int row=int(scenePos.y()/64);
        if(row<0||row>7||col<0||col>7) return false;

        // If it's the engine's turn => ignore user clicks
        Board b = gameState.currentBoard();
        Color stm = b.sideToMove;
        if((stm==Color::White && gameState.whitePlayer.type==PlayerType::Engine) ||
           (stm==Color::Black && gameState.blackPlayer.type==PlayerType::Engine))
        {
            return false;
        }

        Piece clickedPiece = b.board[row][col];

        // Cancel any open promotion menu if user clicks somewhere else
        if(promotionMenu){
            promotionMenu->close(); 
            delete promotionMenu;
            promotionMenu=nullptr;
            pendingPromotionMove = Move(); // discard
        }

        // --- If no piece selected yet ---
        if (!pieceSelected)
        {
            if (clickedPiece.type!=PieceType::None && clickedPiece.color==stm)
            {
                pieceSelected=true;
                selectedRow=row;
                selectedCol=col;
                highlightPossibleMoves(row,col);
                drawBoard();
            }
            // else do nothing
        }
        else
        {
            // A piece is selected
            // If user clicks another of their own pieces => switch selection
            if (clickedPiece.type!=PieceType::None && clickedPiece.color==stm)
            {
                pieceSelected=true;
                selectedRow=row;
                selectedCol=col;
                highlightPossibleMoves(row,col);
                drawBoard();
            }
            else
            {
                // Attempt move if (row,col) is in highlightSquares
                bool validDest=false;
                for(auto &sq : highlightSquares){
                    if(sq.first==row && sq.second==col){
                        validDest=true;
                        break;
                    }
                }
                if(validDest){
                    bool success = tryClickMove(selectedRow, selectedCol, row, col);
                    // No popup on failure; just ignore
                }
                // else invalid => just clear selection

                pieceSelected=false;
                highlightSquares.clear();
                drawBoard();
            }
        }
        return true;
    }
    return QMainWindow::eventFilter(obj,event);
}

// ---------------------------------------------------
// tryClickMove: create a Move, test if promotion needed
// ---------------------------------------------------

bool analysisMode = false;

bool MainWindow::tryClickMove(int fromRow, int fromCol, int toRow, int toCol)
{
    if (!analysisMode) {
        // if we are not in analysis mode, ensure we're at the end of the history
        // so we do NOT rewrite moves
        if (replayIndex < (int)gameState.moveHistory().size()) {
            // user is in the middle => do not allow new moves
            // quietly ignore or show a small message "You cannot alter the past in game mode."
            return false;
        }
    }
	{
    Board b = gameState.currentBoard();
    // Check in the legal moves
    auto allMoves = generateLegalMoves(b);

    // find the matching move in that set
    for(const auto &m : allMoves){
        if(m.fromRow==fromRow && m.fromCol==fromCol &&
           m.toRow==toRow   && m.toCol==toCol)
        {
            // If this move is a promotion => show a small pop-up near the target
            // so user can pick which piece to promote to
            // if user picks nothing => we let them do something else
            Piece mover = b.board[fromRow][fromCol];
            bool isPawn = (mover.type == PieceType::Pawn);
            bool finalRank = false;
            if(isPawn && finalRank){
                if((mover.color==Color::White && toRow==0) ||
                   (mover.color==Color::Black && toRow==7))
                {
                // store it in pending
                pendingPromotionMove = m;
                showPromotionChoices(toRow, toCol);
                return true;
                }
            }


            else {
                // normal move => apply it now
                applyMoveAndRefresh(m);
                return true;
            }
        }
    }
    // not found => invalid
    return false;
}
    // Then proceed with your normal logic
    // ...
}

void MainWindow::onToggleAnalysisMode(bool checked)
{
    analysisMode = checked;
}

// ---------------------------------------------------
// Show a pop-up menu at the board square for promotion
// ---------------------------------------------------
void MainWindow::showPromotionChoices(int boardRow, int boardCol)
{

    // If there's already a menu, close it
    if(promotionMenu){
        promotionMenu->close();
        delete promotionMenu;
        promotionMenu=nullptr;
    }

    promotionMenu = new QMenu(this);

    QAction *queenAct  = promotionMenu->addAction("Promote to Queen");
    QAction *rookAct   = promotionMenu->addAction("Promote to Rook");
    QAction *bishopAct = promotionMenu->addAction("Promote to Bishop");
    QAction *knightAct = promotionMenu->addAction("Promote to Knight");

    // When user picks an action => finalize
    connect(queenAct,  &QAction::triggered, this, [this](){ finalizePromotion(PieceType::Queen);  });
    connect(rookAct,   &QAction::triggered, this, [this](){ finalizePromotion(PieceType::Rook);   });
    connect(bishopAct, &QAction::triggered, this, [this](){ finalizePromotion(PieceType::Bishop); });
    connect(knightAct, &QAction::triggered, this, [this](){ finalizePromotion(PieceType::Knight); });

    // If user clicks away => menu closes => we do aboutToHide => discard
    connect(promotionMenu, &QMenu::aboutToHide, this, [this](){
        // If we get here w/o picking a piece => user canceled promotion
        // We do not finalize. Just reset pendingPromotionMove
        pendingPromotionMove = Move();
    });

    int squareSize=64;
    int x = boardCol * squareSize;
    int y = boardRow * squareSize;
    QPoint globalPos = view->mapToGlobal(QPoint(x,y));

    promotionMenu->popup(globalPos); 
}

// ---------------------------------------------------
// finalizePromotion => set the piece and apply the move
// ---------------------------------------------------
void MainWindow::finalizePromotion(PieceType chosen)
{
    if(pendingPromotionMove.fromRow<0) {
        // no pending promotion => do nothing
        return;
    }
    pendingPromotionMove.promotionPiece = chosen;

    // close & delete the QMenu
    if(promotionMenu){
        promotionMenu->close();
        delete promotionMenu;
        promotionMenu=nullptr;
    }

    // Now apply the move
    applyMoveAndRefresh(pendingPromotionMove);

    // reset
    pendingPromotionMove = Move();
}
// ---------------------------------------------------
// highlightPossibleMoves
// ---------------------------------------------------
void MainWindow::highlightPossibleMoves(int row, int col){
    highlightSquares.clear();
    Board b = gameState.currentBoard();
    auto all = generateLegalMoves(b);
    for(auto &m : all){
        if(m.fromRow==row && m.fromCol==col){
            highlightSquares.push_back({m.toRow,m.toCol});
        }
    }
}

// ---------------------------------------------------
// drawBoard
// ---------------------------------------------------
void MainWindow::drawBoard(){
    scene->clear();
    int squareSize=64;
    Board b=gameState.currentBoard();

    for(int r=0; r<8; r++){
        for(int c=0; c<8; c++){
            bool light=((r+c)%2==0);
            QColor color= light ? QColor("#f0d9b5") : QColor("#b58863");
            scene->addRect(c*squareSize, r*squareSize, squareSize, squareSize,
                           Qt::NoPen, color);

            Piece p=b.board[r][c];
            if(p.type!=PieceType::None){
                QString colorStr=(p.color==Color::White)? "white":"black";
                QString pieceStr;
                switch(p.type){
                    case PieceType::Pawn:   pieceStr="pawn";   break;
                    case PieceType::Rook:   pieceStr="rook";   break;
                    case PieceType::Knight: pieceStr="knight"; break;
                    case PieceType::Bishop: pieceStr="bishop"; break;
                    case PieceType::Queen:  pieceStr="queen";  break;
                    case PieceType::King:   pieceStr="king";   break;
                    default: break;
                }
                QString path=QString(":/images/default/%1_%2.png")
                             .arg(colorStr).arg(pieceStr);
                QPixmap pm(path);
                pm=pm.scaled(squareSize,squareSize,Qt::KeepAspectRatio,Qt::SmoothTransformation);
                auto pix=scene->addPixmap(pm);
                pix->setPos(c*squareSize,r*squareSize);
            }
        }
    }

    // highlight
    for(auto &sq : highlightSquares){
        scene->addRect(sq.second*squareSize, sq.first*squareSize,
                       squareSize, squareSize,
                       QPen(Qt::yellow,2),
                       QBrush(QColor(255,255,0,80)));
    }
}

// ---------------------------------------------------
// applyMoveAndRefresh
// ---------------------------------------------------
void MainWindow::applyMoveAndRefresh(const Move &m){
    bool ok=gameState.makeMove(m);
    if(!ok){
        // Remove invalid-move popup. Instead, do nothing special
        // (We could show a "blink" or something, but no blocking)
        return;
    }
    // after a valid move => we are at the new end
    replayIndex=(int)gameState.moveHistory().size();

    drawBoard();
    updateMoveListDisplay();

    // check game ending
    Board b=gameState.currentBoard();
    auto possible=generateLegalMoves(b);
    if(possible.empty()){
        if(b.isKingInCheck(b.sideToMove)){
            // checkmate
            Color winner=(b.sideToMove==Color::White)? Color::Black:Color::White;
            QString ws=(winner==Color::White)?"White":"Black";
            QMessageBox::information(this,"Checkmate",ws+" wins!");
        } else {
            QMessageBox::information(this,"Stalemate","It's a draw!");
        }
        // stop engine vs engine if running
        engineTimer->stop();
        engineVsEngineActive=false;
    }
}

// ---------------------------------------------------
// updateMoveListDisplay
// ---------------------------------------------------
void MainWindow::updateMoveListDisplay()
{
    moveList->clear();
    const auto &hist = gameState.moveHistory();

    // We iterate in steps of 2 so that each iteration 
    // prints one White move + possibly one Black move
    for (size_t i = 0; i < hist.size(); i += 2) {
        size_t whiteIdx = i;
        size_t blackIdx = i + 1;

        int moveNumber = (int)i/2 + 1; // e.g. i=0 => move#=1, i=2 => move#=2, etc.

        std::string whiteMove = hist[whiteIdx].notation;
        std::string blackMove;
        if (blackIdx < hist.size()) {
            blackMove = hist[blackIdx].notation;
        }

        // Build one line of text:
        // e.g. "1. e4 e5"
        QString line = QString("%1. %2 %3")
            .arg(moveNumber)
            .arg(QString::fromStdString(whiteMove))
            .arg(QString::fromStdString(blackMove));

        moveList->addItem(line);
    }
}

// ---------------------------------------------------
// SLOTS
// ---------------------------------------------------
void MainWindow::onNewGame(){
    // both human
    gameState.newGame(PlayerType::Human,"White",PlayerType::Human,"Black");

    pieceSelected=false;
    highlightSquares.clear();
    replayIndex=0;
    engineVsEngineActive=false;
    engineTimer->stop();

    // discard any open menu
    if(promotionMenu){
        promotionMenu->close();
        delete promotionMenu; 
        promotionMenu=nullptr;
    }
    pendingPromotionMove = Move();

    drawBoard();
    updateMoveListDisplay();
}

void MainWindow::onEngineMove(){
    std::string fen=gameState.currentBoard().toFEN();
    engineManager->requestMove(fen,8);
}

void MainWindow::handleEngineBestMove(const Move &m){
    applyMoveAndRefresh(m);
}

void MainWindow::onGoStart(){
    replayIndex=0;
    gameState.jumpToMoveIndex(replayIndex);
    pieceSelected=false;
    highlightSquares.clear();

    // discard promotion
    if(promotionMenu){
        promotionMenu->close();
        delete promotionMenu; 
        promotionMenu=nullptr;
    }
    pendingPromotionMove = Move();

    drawBoard();
    updateMoveListDisplay();
}

void MainWindow::onPrevMove(){
    if(replayIndex>0){
        replayIndex--;
        gameState.jumpToMoveIndex(replayIndex);
        pieceSelected=false;
        highlightSquares.clear();

        if(promotionMenu){
            promotionMenu->close();
            delete promotionMenu; 
            promotionMenu=nullptr;
        }
        pendingPromotionMove = Move();

        drawBoard();
        updateMoveListDisplay();
    }
}

void MainWindow::onNextMove(){
    auto &hist=gameState.moveHistory();
    if(replayIndex < (int)hist.size()){
        replayIndex++;
        gameState.jumpToMoveIndex(replayIndex);
        pieceSelected=false;
        highlightSquares.clear();

        if(promotionMenu){
            promotionMenu->close();
            delete promotionMenu; 
            promotionMenu=nullptr;
        }
        pendingPromotionMove = Move();

        drawBoard();
        updateMoveListDisplay();
    }
}

void MainWindow::onGoEnd(){
    replayIndex=(int)gameState.moveHistory().size();
    gameState.jumpToMoveIndex(replayIndex);
    pieceSelected=false;
    highlightSquares.clear();

    if(promotionMenu){
        promotionMenu->close();
        delete promotionMenu; 
        promotionMenu=nullptr;
    }
    pendingPromotionMove = Move();

    drawBoard();
    updateMoveListDisplay();
}

void MainWindow::onStartEngineVsEngine(){
    // White=Engine, Black=Engine
    gameState.newGame(PlayerType::Engine,"Engine1",
                      PlayerType::Engine,"Engine2");

    pieceSelected=false;
    highlightSquares.clear();
    replayIndex=0;

    if(promotionMenu){
        promotionMenu->close();
        delete promotionMenu; 
        promotionMenu=nullptr;
    }
    pendingPromotionMove = Move();

    drawBoard();
    updateMoveListDisplay();

    engineVsEngineActive=true;
    engineTimer->start(1000); // 1 second intervals
}

void MainWindow::onSavePGN(){
    QString fileName=QFileDialog::getSaveFileName(this,"Save PGN","","PGN Files (*.pgn)");
    if(fileName.isEmpty()) return;

    std::string pgnText=PGN::exportPGN(gameState);
    std::ofstream out(fileName.toStdString());
    if(!out){
        QMessageBox::warning(this,"Error","Cannot open file for writing!");
        return;
    }
    out << pgnText;
    out.close();
    QMessageBox::information(this,"Saved","PGN saved successfully!");
}
