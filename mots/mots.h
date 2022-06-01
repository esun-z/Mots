#pragma once

#include <QtWidgets/QWidget>
#include "ui_mots.h"
#include "global.h"
#include "AreanListener.h"
#include "ConfigManager.h"
#include "RobotCheck.h"

class mots : public QWidget
{
    Q_OBJECT

public:
    mots(QWidget *parent = Q_NULLPTR);
    MotsConfig config = MotsConfig();
    AreanListener* areanListener;


private slots:
    void HandleGameDirError();
    void HandleReadingError();
    void HandleNetworkError();
    void HandleGameEnd();
    void UpdateAreanData();
    void HandleAllInfoReady();
    void HandleGameStart();
    void HandleCreateConfigFile();
    void UpdateConfig();
    void HandleConfigError();
    void ChangeConfig();
    void ResetConfig();

    void HandleUpdateTimer();

    void HandleRobotCheckError();
    void HandleRobotCheckAllow();
    void HandleRobotCheckDisallow();

    

private:
    Ui::motsClass ui;
    QTimer* networkInfoUpdateTimer = nullptr;
    ConfigManager* configManager = nullptr;
    RobotCheck* rc = nullptr;

    void InitConnection();

    void SendLog(QString log);

    void CheckBot();
    
};
