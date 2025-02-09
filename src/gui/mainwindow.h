#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QToolBar>
#include <QAction>
#include <QTimer>
#include <QDockWidget>
#include <QListWidget>
#include <QMenu>
#include "gamestate.h"
#include "enginemanager.h"
#include "moves.h"

// If you had a separate promotiondialog.h, you might not need it now
// if you're using a QMenu approach. Keep or remove as needed.

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow();
public slots:
    void onToggleAnalysisMode(bool checked);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
	
    void onNewGame();
    void onEngineMove();
    void handleEngineBestMove(const Move &m);
    void onGoStart();
    void onPrevMove();
    void onNextMove();
    void onGoEnd();
    void onStartEngineVsEngine();
    void onSavePGN();

private:
    void setupUI();
    void drawBoard();
    void updateMoveListDisplay();
    bool tryClickMove(int fromRow, int fromCol, int toRow, int toCol);
    void applyMoveAndRefresh(const Move &m);
    void highlightPossibleMoves(int row, int col);

    // NEW: For non-blocking promotion approach
    void showPromotionChoices(int boardRow, int boardCol);
    void finalizePromotion(PieceType chosen);

    GameState gameState;
    EngineManager *engineManager;

    QGraphicsScene *scene;
    QGraphicsView  *view;
    QToolBar       *toolbar;
    QListWidget    *moveList;

    bool pieceSelected;
    int selectedRow, selectedCol;
    std::vector<QPair<int,int>> highlightSquares;

    // For replay
    int replayIndex;

    // For engine vs engine
    QTimer *engineTimer;
    bool   engineVsEngineActive;

    // (A) Store a pointer to the QMenu for promotion
    QMenu *promotionMenu;             

    // (B) Store the pending Move if a promotion is chosen but piece not picked yet

	Move pendingPromotionMove;
};

