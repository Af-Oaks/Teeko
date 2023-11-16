#include "Teeko.h"
#include "ui_Teeko.h"

#include <QDebug>
#include <QMessageBox>
#include <QActionGroup>
#include <QSignalMapper>

Teeko::Teeko(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::Teeko),
      m_player(Player::player(Player::Red)),
      m_phase(Teeko::DropPhase) {

    ui->setupUi(this);

    QObject::connect(ui->actionNew, SIGNAL(triggered(bool)), this, SLOT(reset()));
    QObject::connect(ui->actionQuit, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
    QObject::connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(showAbout()));
    numBlue=0;numRed=0;selected=0,lastMove=-1;
    QSignalMapper* map = new QSignalMapper(this);
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 5; ++col) {
            QString holeName = QString("hole%1%2").arg(row).arg(col);
            Hole* hole = this->findChild<Hole*>(holeName);
            Q_ASSERT(hole != nullptr);

            m_board[row][col] = hole;

            int id = row * 5 + col;
            map->setMapping(hole, id);
            QObject::connect(hole, SIGNAL(clicked(bool)), map, SLOT(map()));
        }
    }
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QObject::connect(map, SIGNAL(mapped(int)), this, SLOT(play(int)));
#else
    QObject::connect(map, SIGNAL(mappedInt(int)), this, SLOT(play(int)));
#endif

    // When the turn ends, switch the player.
    QObject::connect(this, SIGNAL(turnEnded()), this, SLOT(switchPlayer()));

    this->reset();

    this->adjustSize();
    this->setFixedSize(this->size());
}

Teeko::~Teeko() {
    delete ui;
}

void Teeko::setPhase(Teeko::Phase phase) {
    if (m_phase != phase) {
        m_phase = phase;
        emit phaseChanged(phase);
    }
}

void Teeko::play(int id) {//funcionamento se da entorno dessa função
    Hole* hole = m_board[id / 5][id % 5];
    if(numBlue <4 || numRed <4){
        if (hole->isEmpty()) {
            hole->setPlayer(m_player);
            if(m_player->type() ==0){
                numRed++;
            }
            else{
                numBlue++;
            }
            CheckWinCondition(m_player->type());
            emit turnEnded();
        }
    }
    else{
        m_phase = Teeko::MovePhase;
        //apos todos os jogadores botarem as 4 peças verifico se o hole selecionado é valido para o respectivo jogador
        if(hole->isUsed() && hole->player()->type() == m_player->type() && !selected){
            //deixar botao selecionado highlighted
            lastMove = id;selected = 1;
            hole->setState(hole->Selected);
            //mostrar as possibilidades em highlighted
            checkPossibility(id);
            //n trocar de turno
        }
        else if(lastMove == id && selected){
            //retirar highligthed do botao e das possiveis possibilidades
                printf("entrou aqqqqqqq\n");
                lastMove =-1;selected =0;
                hole->setState(hole->Used);
                decheckPossibility();
            //n trocar de turno
        }
        else if(hole->isEmpty() && lastMove !=-1 && selected){
            int diflin = lastMove/5 - id/5;
            int difcol = lastMove%5 - id%5;
            //checo se o movimento feito está detro das possibilidades
            if(abs(diflin) <=1 && abs(difcol) <=1){

                Hole* lastHole = m_board[lastMove/ 5][lastMove%5];
                decheckPossibility();
                lastHole->reset();
                hole->setPlayer(m_player);
                CheckWinCondition(m_player->type());

                //encerro a jogada e passo
                lastMove =-1;
                selected =0;
                emit turnEnded();
            }
            else{
                //do nothing??
                //printf("to no ultimo\n");
            }

        }
        else{
            //printf("entrei em lugar nehum \n");
        }

    }

}

void Teeko::decheckPossibility(){
    Hole* hole;
    for(int x=0;x<25;x++){
        hole =  m_board[x/5][x%5];
        if( hole->isEmpty() ){
            hole->reset();
        }
    }

}

void Teeko::checkPossibility(int play){
    int play_col =play/5,play_row = play%5;
    int che_col,che_row;

    for(int x=0;x<25;x++){
        che_col = x/5; che_row = x%5;
        Hole* hole2 =  m_board[che_col/5][che_row%5];
        printf("x=%d | lin=%d | col=%d |vaz=%d\n",x,che_col,che_row,hole2->isEmpty());
        if(hole2->isEmpty()){
            //printf(" vazio x=%d ",x);
            if( abs(play_col-che_col) <=1 && abs(play_row - che_row) <=1 ){
                hole2->setState(hole2->Playable);
                //printf("play =%d || lin =%d || col =%d || diflin=%d ||difCol =%d\n",play,che_col,che_row,abs(play_col-che_col),abs(play_row - che_row));
            }
        }
    }
}

void Teeko::switchPlayer() {
    // Switch the player.
    m_player = m_player->other();

    // Finally, update the status bar.
    this->updateStatusBar();
}

void Teeko::reset() {
    // Reset board.
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            Hole* hole = m_board[row][col];
            hole->reset();
        }
    }
    numBlue =0; numRed=0;selected =0;lastMove=-1;
    // Reset the player.
    m_player = Player::player(Player::Red);

    // Reset to drop phase.
    m_phase = Teeko::DropPhase;

    // Finally, update the status bar.
    this->updateStatusBar();
}

void Teeko::showAbout() {
    QMessageBox::information(this, tr("About"), tr("Teeko\n\nAndrei Rimsa Alvares - andrei@cefetmg.br"));
}

void Teeko::updateStatusBar() {
    QString phase(m_phase == Teeko::DropPhase ? tr("colocar") : tr("mover"));

    ui->statusbar->showMessage(tr("Fase de %1: vez do %2")
        .arg(phase)
        .arg(m_player->name()));
}

void Teeko::CheckWinCondition(int player){
    // percorro a matriz de hole e guardo em 1 vetor de pares
    int cond1 = 0 ,cond2 = 0,tam =0;
    std::vector<std::pair<int,int>> posicoes;
    for(int x =0;x<5;x++){
        for(int z =0;z<5;z++){
            Hole* hole =m_board[x][z];
            if(hole->isUsed() && hole->player()->type() == player){
                posicoes.push_back(std::make_pair(x,z));
                tam++;
            }
        }
    }
    //condição para vitoria em linha ou coluna
    //se exites em um ou outro 4 numeros iguais então há a chance de ter vencido o game
    // precisa so checar no outro vetor se os numeros da posicoes sao sequenciais
    std::sort(posicoes.begin(),posicoes.end());

    for(int x=0;x<tam-1;x++){
        if(posicoes[x].first == (posicoes[x+1].first) && posicoes[x].second +1 == (posicoes[x+1].second)  ){//condicao linha
            cond1++;
        }
        if(posicoes[x].second== (posicoes[x+1].second) && posicoes[x].first +1 == (posicoes[x+1].first) ){//condicao coluna
            cond2++;
        }
    }
    if(cond1 ==3 || cond2 ==3){
        goto end;
    }
    else{
        cond1 =0; cond2=0;
    }
    //condição para diagonais
    //pego a posicao de row e col em um vetor de pares, ordeno em forma crescente pela linha. se for crescente sequencial na coluna é diagona principal
    //da mesma forma anterior,mas ordeno de forma descrescente pela linha, se for crescente sequencial na coluna é diagonal secundaria
    for(int x=0;x<tam-1;x++){
        if( (posicoes[x].first+1 )== (posicoes[x+1].first) && (posicoes[x].second+1) == (posicoes[x+1].second)  ){//diagonal principal ok
            cond1++;
        }
        if( (posicoes[x].second -1) == (posicoes[x+1].second) && (posicoes[x].first+1) == (posicoes[x+1].first) ){//diagonal secundaria
            cond2++;
        }
    }
    if(cond1 ==3 || cond2 ==3){
        goto end;
    }
    else{
        cond1=0; cond2=0;
    }

    //condição de quadrado
    //checo manualmente o espaçamento das bolinhas
    if(posicoes[0].first == posicoes[1].first && posicoes[2].first == posicoes[3].first && posicoes[0].first +1 == posicoes[2].first){
        if(posicoes[0].second == posicoes[2].second && posicoes[1].second == posicoes[3].second && posicoes[0].second +1 == posicoes[1].second){
            cond1 =3;
        }
    }
    //fim
    end:
    if(cond1 ==3 || cond2 ==3){
        vitoriaPlayer(player);
    }

}
void Teeko::vitoriaPlayer(int player){
    // sinalizo vitoria do player vencedor abrindo um popUP, ao clicar em reinciar(reinicia o jogo),caso contrario deixe o tabuleiro travado para modificao
    //e so será novo jogo ao iniciar novo jogo manualmente.
    //TODO: mensagens já implementada, verificar como fazer o reinicio ao fechar a aba e trancar o tabuleiro.
    if(player ==0)
        QMessageBox::information(this, tr("VENCEDOR"), tr("Jogador Vermelho ganhou"));
    else
        QMessageBox::information(this, tr("VENCEDOR"), tr("Jogador Azul ganhou"));
    reset();
}
