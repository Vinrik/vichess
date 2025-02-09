#include "enginemanager.h"
#include <QDebug>
#include <QStringList>

EngineManager::EngineManager(QObject *parent)
    : QObject(parent)
{
    connect(&engineProcess, &QProcess::readyReadStandardOutput,
            this, &EngineManager::onReadyRead);
    connect(&engineProcess, &QProcess::stateChanged,
            this, &EngineManager::onStateChanged);
}

void EngineManager::setEnginePath(const QString &path){
    enginePath = path;
}

bool EngineManager::startEngine(){
    if(enginePath.isEmpty()){
        qWarning() << "Engine path not set!";
        return false;
    }
    engineProcess.start(enginePath);
    if(!engineProcess.waitForStarted(3000)){
        qWarning() << "Failed to start engine at" << enginePath;
        return false;
    }
    engineProcess.write("uci\n");
    engineProcess.waitForBytesWritten(100);  // optional
    return true;
}

void EngineManager::requestMove(const std::string &fen, int depth){
    if(!engineReady){
        qWarning() << "Engine not ready or not in UCI mode.";
        return;
    }

    QString cmd = QString("position fen %1\n").arg(QString::fromStdString(fen));
    engineProcess.write(cmd.toUtf8());

    QString goCmd = QString("go depth %1\n").arg(depth);
    engineProcess.write(goCmd.toUtf8());

    engineProcess.waitForBytesWritten();  // optional
}

void EngineManager::onReadyRead(){
    QString output = engineProcess.readAllStandardOutput();
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for(const QString &line: lines){
        if(line.startsWith("uciok")){
            engineReady = true;
        }
        else if(line.startsWith("bestmove")){
            QStringList parts = line.split(' ');
            if(parts.size()>=2){
                Move m = parseBestMove(parts.at(1));
                emit bestMoveReady(m);
            }
        }
    }
}

void EngineManager::onStateChanged(QProcess::ProcessState newState){
    qDebug()<<"Engine process state changed:"<<newState;
}

Move EngineManager::parseBestMove(const QString &bmStr){
    if(bmStr.length()<4) return Move();
    int fromCol = bmStr[0].unicode()-'a';
    int fromRow = 8 - (bmStr[1].unicode()-'0');
    int toCol   = bmStr[2].unicode()-'a';
    int toRow   = 8 - (bmStr[3].unicode()-'0');
    Move mv(fromRow, fromCol, toRow, toCol);

    if(bmStr.length()==5){
        char promoC = bmStr[4].toLatin1();
        switch(promoC){
            case 'q': mv.promotionPiece=PieceType::Queen;  break;
            case 'r': mv.promotionPiece=PieceType::Rook;   break;
            case 'b': mv.promotionPiece=PieceType::Bishop; break;
            case 'n': mv.promotionPiece=PieceType::Knight; break;
            default: break;
        }
    }
    return mv;
}
