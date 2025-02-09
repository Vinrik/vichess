#include "promotiondialog.h"

PromotionDialog::PromotionDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Promote Pawn");
    setModal(true);

    QHBoxLayout *layout=new QHBoxLayout(this);

    QPushButton *queenBtn=new QPushButton("Queen");
    QPushButton *rookBtn=new QPushButton("Rook");
    QPushButton *bishopBtn=new QPushButton("Bishop");
    QPushButton *knightBtn=new QPushButton("Knight");

    layout->addWidget(queenBtn);
    layout->addWidget(rookBtn);
    layout->addWidget(bishopBtn);
    layout->addWidget(knightBtn);

    connect(queenBtn,&QPushButton::clicked,this,[this](){piece_=PieceType::Queen;accept();});
    connect(rookBtn,&QPushButton::clicked,this,[this](){piece_=PieceType::Rook;accept();});
    connect(bishopBtn,&QPushButton::clicked,this,[this](){piece_=PieceType::Bishop;accept();});
    connect(knightBtn,&QPushButton::clicked,this,[this](){piece_=PieceType::Knight;accept();});

    setLayout(layout);
    resize(300,80);
}
