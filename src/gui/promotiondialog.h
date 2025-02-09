#pragma once
#include <QDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include "board.h"

class PromotionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PromotionDialog(QWidget *parent=nullptr);
    PieceType chosenPiece() const { return piece_; }

private:
    PieceType piece_=PieceType::Queen;
};
