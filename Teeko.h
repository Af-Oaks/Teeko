#ifndef TEEKO_H
#define TEEKO_H

#include <QMainWindow>
#include <QMap>
#include <QSet>
#include "Hole.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class Teeko;
}
QT_END_NAMESPACE

class Teeko : public QMainWindow {
    Q_OBJECT

public:
    enum Phase {
        DropPhase,
        MovePhase
    };
    Q_ENUM(Phase)

    Teeko(QWidget *parent = nullptr);
    virtual ~Teeko();

    Teeko::Phase phase() const { return m_phase; }

signals:
    void phaseChanged(Teeko::Phase phase);
    void turnEnded();
    void updateBoard();

private:
    Ui::Teeko *ui;
    Player* m_player;
    Phase m_phase;
    Hole* m_board[5][5];
    int numBlue,numRed,selected,lastMove;

private slots:
    void setPhase(Teeko::Phase phase);
    void play(int id);
    void switchPlayer();
    void reset();
    void CheckWinCondition(int player);
    void vitoriaPlayer(int player);
    void checkPossibility(int id);
    void decheckPossibility();
    void showAbout();
    void updateStatusBar();

};

#endif // TEEKO_H
