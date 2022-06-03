#include "mots.h"

mots::mots(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    SendLog("Initializing.");

    networkInfoUpdateTimer = new QTimer(this);
    
    configManager = new ConfigManager(&config, this);
    connect(configManager, SIGNAL(CreateConfigFile()), this, SLOT(HandleCreateConfigFile()));
    connect(ui.pushButtonCancelChange, SIGNAL(clicked()), this, SLOT(UpdateConfig()));
    connect(ui.pushButtonConfirmConfig, SIGNAL(clicked()), this, SLOT(ChangeConfig()));
    connect(ui.pushButtonResetConfig, SIGNAL(clicked()), this, SLOT(ResetConfig()));
    UpdateConfig();
    
    CheckBot();
}

void mots::CheckBot() {
    SendLog("Checking robot permission.");
    rc = new RobotCheck(ROBOTCHECKURL, this);
    connect(rc, SIGNAL(Error()), this, SLOT(HandleRobotCheckError()));
    connect(rc, SIGNAL(RobotAllow()), this, SLOT(HandleRobotCheckAllow()));
    connect(rc, SIGNAL(RobotDisallow()), this, SLOT(HandleRobotCheckDisallow()));
}

void mots::HandleRobotCheckAllow() {
    SendLog("Robot permission confirmed.");
    arenaListener = new ArenaListener(&config, this);
    InitConnection();
    arenaListener->Init(&config, this);
    if (arenaListener->status != ARENASTATUS_ERROR) {
        SendLog("Ready. Listening.");
    }
}

void mots::HandleRobotCheckError() {
    SendLog("* Network Error: " + rc->lastError);
}

void mots::HandleRobotCheckDisallow() {
    SendLog("* Failed to get robot permission.\nrobots.txt:\n" + rc->lastError);
}

void mots::InitConnection() {
    connect(arenaListener, SIGNAL(ErrorFindingGameDir()), this, SLOT(HandleGameDirError()));
    connect(arenaListener, SIGNAL(ErrorStartingGame()), this, SLOT(HandleReadingError()));
    connect(arenaListener, SIGNAL(ErrorNetworking()), this, SLOT(HandleNetworkError()));
    connect(arenaListener, SIGNAL(GameStart()), this, SLOT(HandleGameStart()));
    connect(arenaListener, SIGNAL(AllInfoReady()), this, SLOT(HandleAllInfoReady()));
    connect(arenaListener, SIGNAL(GameEnd()), this, SLOT(HandleGameEnd()));
    connect(networkInfoUpdateTimer, SIGNAL(timeout()), this, SLOT(HandleUpdateTimer()));
    
}

void mots::SendLog(QString log) {
    ui.labelLog->setText(log);
    ui.listWidgetLog->addItem(log);
    ui.listWidgetLog->setCurrentRow(ui.listWidgetLog->count() - 1);
}

void mots::HandleGameDirError() {
    SendLog("* Error: Failed to find game folder. Please check the settings.");
    ui.tabWidget->setCurrentWidget(ui.tabConfig);
}

void mots::HandleReadingError() {
    SendLog("* Error: Failed to read arena info file. \nThis can be aroused by a file protector \nor an unexpected format change due to an update of the game.");
    ui.tabWidget->setCurrentWidget(ui.tabLog);
}

void mots::HandleNetworkError() {
    SendLog("* Network Error: " + arenaListener->networkManager->lastError);
    UpdateAreanData();
    //ui.tabWidget->setCurrentWidget(ui.tabLog);
}

void mots::HandleGameEnd() {
    SendLog("Game ended. Ready to listen next game.");
    arenaListener->Init(&config, this);
}

void mots::HandleGameStart() {
    UpdateAreanData();
    if (config.networkInterval > 0) {
        networkInfoUpdateTimer->start(config.networkInterval);
    }
    else {
        networkInfoUpdateTimer->start(1000);
    }
    SendLog("Game started. Getting players' scores...");
}

void mots::HandleAllInfoReady() {
    networkInfoUpdateTimer->stop();
    UpdateAreanData();
    SendLog("All data loaded. Enjoy your game, " + arenaListener->arena.selfPlayerName + " on " + arenaListener->arena.selfShipName + ".");
}

void mots::HandleCreateConfigFile() {
    UpdateConfig();
    SendLog("Creating new config file.");
}

void mots::HandleConfigError() {
    SendLog("* Config Error: " + configManager->lastError);
}

void mots::HandleUpdateTimer() {
    qDebug() << "Timer: UpdateData";
    UpdateAreanData();
}

void mots::UpdateConfig() {
    ui.lineEditGamePath->setText(config.gamePath);
    ui.lineEditGetIdUrl->setText(config.getIdUrl);
    ui.lineEditGetShipNameUrl->setText(config.getShipNameUrl);
    ui.lineEditGetOverviewUrl->setText(config.getOverviewUrl);
    ui.lineEditGetShipUrl->setText(config.getShipUrl);
    if (config.customApplicationId) {
        ui.lineEditApplicationId->setText(config.applicationId);
    }
    else {
        ui.lineEditApplicationId->clear();
    }
    ui.lineEditLocalInterval->setText(QString::number(config.localInterval));
    ui.lineEditNetworkInterval->setText(QString::number(config.networkInterval));
}

void mots::ChangeConfig() {
    MotsConfig c;
    c.gamePath = ui.lineEditGamePath->text();
    c.getIdUrl = ui.lineEditGetIdUrl->text();
    c.getOverviewUrl = ui.lineEditGetOverviewUrl->text();
    c.getShipNameUrl = ui.lineEditGetShipNameUrl->text();
    c.getShipUrl = ui.lineEditGetShipUrl->text();
    if (ui.lineEditApplicationId->text().isEmpty()) {
        c.customApplicationId = false;
    }
    else {
        c.customApplicationId = true;
        c.applicationId = ui.lineEditApplicationId->text();
    }
    c.localInterval = ui.lineEditLocalInterval->text().toLong();
    c.networkInterval = ui.lineEditNetworkInterval->text().toLong();

    if (configManager->ValidConfig(&c)) {
        config = c;
        arenaListener->Init(&config, this);
        if (configManager->WriteConfig()) {
            SendLog("Config saved.");
        }
        else {
            SendLog("Config changed. Failed to save config.");
        }
    }
    else {
        SendLog("* Error: Invalid config.");
    }
}

void mots::ResetConfig() {
    config = MotsConfig();
    UpdateConfig();
    configManager->WriteConfig();
    SendLog("Config reset.");
}

void mots::UpdateAreanData() {

    //SendLog("Updating data.");

    QString playerStr;
    QStringList ally, enemy;

    QColor colors1[MAXPLAYERCOUNT];
    QColor colors2[MAXPLAYERCOUNT];
    for (int i = 0; i < arenaListener->arena.playerCount; i++) {
        colors1[i] = QColor(0, 0, 0, 0);
        colors2[i] = QColor(0, 0, 0, 0);
    }

    int self = 0x7FFFFFFF;
    for (int i = 0; i < arenaListener->arena.playerCount; i++) {
        playerStr = "";
        if (arenaListener->arena.player[i].onShipScore.pr != 0) {
            colors2[enemy.count()] = PrUtility::GetColorFromPr(arenaListener->arena.player[i].onShipScore.pr);
        }
        if (arenaListener->arena.player[i].onShipScore.pr != 0) {
            colors1[ally.count()] = PrUtility::GetColorFromPr(arenaListener->arena.player[i].onShipScore.pr);
        }
        if (!arenaListener->arena.player[i].basicReady) {
            continue;
        }
        playerStr += arenaListener->arena.player[i].name + "\t";
        if (!arenaListener->arena.player[i].networkInfoReady) {
            playerStr += "\n----------------------------------------------------------------------";
            switch (arenaListener->arena.player[i].relation) {
            case RELATION_SELF:
                self = ally.count();
                ally << playerStr;
                break;
            case RELATION_ALLY:
                ally << playerStr;
                break;
            case RELATION_ENEMY:
                enemy << playerStr;
                break;
            default:
                break;
            }
            continue;
        }
        playerStr += "[" + arenaListener->arena.player[i].shipName + "]\tAvgTier: " + QString::number(arenaListener->arena.player[i].averageTier);
        if (arenaListener->arena.player[i].onShipScore.pr != 0) {
            playerStr += "\tPR: " + QString::number(arenaListener->arena.player[i].onShipScore.pr);
        }
        playerStr += "\n";
        double overviewWinningRate = (double)arenaListener->arena.player[i].overviewScore.winningCount / arenaListener->arena.player[i].overviewScore.battleCount * 100;
        double onShipWinningRate = (double)arenaListener->arena.player[i].onShipScore.winningCount / arenaListener->arena.player[i].onShipScore.battleCount * 100;
        playerStr += "ov: " + QString::number(arenaListener->arena.player[i].overviewScore.battleCount) + "\t" +
            QString::number(overviewWinningRate, 'f', 1) + "%\t" + QString::number(arenaListener->arena.player[i].overviewScore.averageExp) + "\t" +
            QString::number(arenaListener->arena.player[i].overviewScore.averageDamage) + "\t" + QString::number(arenaListener->arena.player[i].overviewScore.frag, 'f', 2);
        playerStr += "\n";
        playerStr += "cs: " + QString::number(arenaListener->arena.player[i].onShipScore.battleCount) + "\t" +
            QString::number(onShipWinningRate, 'f', 1) + "%\t" + QString::number(arenaListener->arena.player[i].onShipScore.averageExp) + "\t" +
            QString::number(arenaListener->arena.player[i].onShipScore.averageDamage) +"\t" + QString::number(arenaListener->arena.player[i].onShipScore.frag, 'f', 2);
        playerStr += "\n----------------------------------------------------------------------";
        switch (arenaListener->arena.player[i].relation) {
        case RELATION_SELF:
            self = ally.count();
            ally << playerStr;
            break;
        case RELATION_ALLY:
            ally << playerStr;
            break;
        case RELATION_ENEMY:
            enemy << playerStr;
            break;
        default:
            break;
        }
    }
    ui.listWidgetData1->clear();
    ui.listWidgetData2->clear();
    
    ui.listWidgetData1->addItems(ally);
    ui.listWidgetData2->addItems(enemy);

    for (int i = 0; i < ally.count(); i++) {
        ui.listWidgetData1->item(i)->setBackgroundColor(colors1[i]);
    }
    for (int i = 0; i < enemy.count(); i++) {
        ui.listWidgetData2->item(i)->setBackgroundColor(colors2[i]);
    }

    if (self < ui.listWidgetData1->count()) {
        ui.listWidgetData1->setCurrentRow(self);
    }
    
}
