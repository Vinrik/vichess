#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include "enginemanager.h"
#include "moves.h"
#include "board.h"

// For QSignalSpy to handle a Move type:
Q_DECLARE_METATYPE(Move);

TEST(EngineIntegrationTest, CallFirst30Moves)
{
    // If your test runner doesn't have a QCoreApplication, we may need:
    // int argc=1; char* argv[]={"test_engine_integration"};
    // QCoreApplication app(argc, argv);

    // Register the Move metatype
    qRegisterMetaType<Move>("Move");

    EngineManager engine;
    engine.setEnginePath("/usr/bin/stockfish"); // or your path
    bool started = engine.startEngine();
    ASSERT_TRUE(started) << "Engine must start successfully.";

    // We'll store a local Board to apply each best move
    Board board;
    board.init(); // standard chess start

    const int maxMoves = 30;
    for(int i=0; i<maxMoves; i++){
        // request best move
        std::string fen=board.toFEN();
        engine.requestMove(fen,6); // depth=6

        QSignalSpy spy(&engine, &EngineManager::bestMoveReady);

        // Wait up to 3s for a bestmove
        QTime limit = QTime::currentTime().addSecs(3);
        while(spy.count()==0 && QTime::currentTime()<limit){
            QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        }

        ASSERT_EQ(spy.count(), 1) << "Engine didn't respond with bestmove in time.";

        // Take the best move from the signal
        QVariantList args=spy.takeFirst();
        Move best = args[0].value<Move>();

        // Check if it's in generateLegalMoves
        auto allMoves=generateLegalMoves(board);
        bool found=false;
        for(auto &m: allMoves){
            if(m.fromRow==best.fromRow && m.fromCol==best.fromCol &&
               m.toRow==best.toRow && m.toCol==best.toCol)
            {
                found=true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Engine gave an illegal move?? " 
                           << "("<<best.fromRow<<","<<best.fromCol<<")->("
                           <<best.toRow<<","<<best.toCol<<")";

        // Actually apply the move
        bool ok=board.movePiece(best.fromRow,best.fromCol,
                                best.toRow,best.toCol,
                                best.isCapture,best.isEnPassant,
                                best.promotionPiece,best.isCastling);
        EXPECT_TRUE(ok);

        // If no more moves possible, break
        auto next = generateLegalMoves(board);
        if(next.empty()) break;
    }
}
