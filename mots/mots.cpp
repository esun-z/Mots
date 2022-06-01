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
    areanListener = new AreanListener(&config, this);
    InitConnection();
    areanListener->Init(&config, this);
    if (areanListener->status != AREANSTATUS_ERROR) {
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
    connect(areanListener, SIGNAL(ErrorFindingGameDir()), this, SLOT(HandleGameDirError()));
    connect(areanListener, SIGNAL(ErrorStartingGame()), this, SLOT(HandleReadingError()));
    connect(areanListener, SIGNAL(ErrorNetworking()), this, SLOT(HandleNetworkError()));
    connect(areanListener, SIGNAL(GameStart()), this, SLOT(HandleGameStart()));
    connect(areanListener, SIGNAL(AllInfoReady()), this, SLOT(HandleAllInfoReady()));
    connect(areanListener, SIGNAL(GameEnd()), this, SLOT(HandleGameEnd()));
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
    SendLog("* Network Error: " + areanListener->networkManager->lastError);
    UpdateAreanData();
    ui.tabWidget->setCurrentWidget(ui.tabLog);
}

void mots::HandleGameEnd() {
    SendLog("Game ended. Ready to listen next game.");
    areanListener->Init(&config, this);
}

void mots::HandleGameStart() {
    UpdateAreanData();
    if (config.networkInterval > 0) {
        networkInfoUpdateTimer->start(config.networkInterval);
    }
    else {
        networkInfoUpdateTimer->start(1000);
    }
    SendLog("Game started. Getting players' scores.");
}

void mots::HandleAllInfoReady() {
    networkInfoUpdateTimer->stop();
    UpdateAreanData();
    SendLog("All data loaded. Enjoy your game, " + areanListener->arean.selfPlayerName + " on " + areanListener->arean.selfShipName + ".");
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
        areanListener->Init(&config, this);
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

    SendLog("Updating data.");

    QString playerStr;
    QStringList ally, enemy;
    int self = 0x7FFFFFFF;
    for (int i = 0; i < areanListener->arean.playerCount; i++) {
        playerStr = "";
        if (!areanListener->arean.player[i].basicReady) {
            continue;
        }
        playerStr += areanListener->arean.player[i].name + "\t";
        if (!areanListener->arean.player[i].networkInfoReady) {
            playerStr += "\n------------------------------------------------------------";
            switch (areanListener->arean.player[i].relation) {
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
        playerStr += "[" + areanListener->arean.player[i].shipName + "]\tAvgTier: " + QString::number(areanListener->arean.player[i].averageTier);
        playerStr += "\n";
        double overviewWinningRate = (double)areanListener->arean.player[i].overviewScore.winningCount / areanListener->arean.player[i].overviewScore.battleCount * 100;
        double onShipWinningRate = (double)areanListener->arean.player[i].onShipScore.winningCount / areanListener->arean.player[i].onShipScore.battleCount * 100;
        playerStr += "ov: " + QString::number(areanListener->arean.player[i].overviewScore.battleCount) + "\t" +
            QString::number(overviewWinningRate, 'f', 1) + "%\t" + QString::number(areanListener->arean.player[i].overviewScore.averageExp) + "\t" +
            QString::number(areanListener->arean.player[i].overviewScore.averageDamage);
        playerStr += "\n";
        playerStr += "cs: " + QString::number(areanListener->arean.player[i].onShipScore.battleCount) + "\t" +
            QString::number(onShipWinningRate, 'f', 1) + "%\t" + QString::number(areanListener->arean.player[i].onShipScore.averageExp) + "\t" +
            QString::number(areanListener->arean.player[i].onShipScore.averageDamage);
        playerStr += "\n------------------------------------------------------------";
        switch (areanListener->arean.player[i].relation) {
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
    if (self < ui.listWidgetData1->count()) {
        ui.listWidgetData1->setCurrentRow(self);
    }
}
