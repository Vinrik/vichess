#pragma once
#include <QObject>
#include <QProcess>
#include <QString>
#include "moves.h"

class EngineManager : public QObject
{
    Q_OBJECT

public:
    explicit EngineManager(QObject *parent=nullptr);

    void setEnginePath(const QString &path);
    bool startEngine();

    // request best move from a FEN
    void requestMove(const std::string &fen, int depth=8);

signals:
    void bestMoveReady(const Move &move);

private slots:
    void onReadyRead();
    void onStateChanged(QProcess::ProcessState newState);

private:
    QProcess engineProcess;
    QString enginePath;
    bool engineReady=false;

    Move parseBestMove(const QString &bmStr);
};
