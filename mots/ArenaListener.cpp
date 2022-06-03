#include "ArenaListener.h"

ArenaListener::ArenaListener(MotsConfig* config, QObject *p)
	: QObject(p)
{
    Init(config, p);
}

ArenaListener::~ArenaListener()
{
    if (timer != nullptr) {
        timer->deleteLater();
        timer = nullptr;
    }
    if (file != nullptr) {
        if (file->isOpen()) {
            file->close();
        }
        file->deleteLater();
        file = nullptr;
    }
}

bool ArenaListener::CheckDir(QDir dir) {
    QDir d = dir.path() + "/replays";
    qDebug() << d;
    return d.exists();
}

void ArenaListener::Init(MotsConfig* config, QObject* p) {
    parent = p;
    motsConfig = config;
    dir = config->gamePath;
    if (timer != nullptr) {
        timer->deleteLater();
    }
    timer = new QTimer(this);
    if (!CheckDir(config->gamePath)) {
        qDebug() << "* Error Finding Game Dir";
        status = ARENASTATUS_ERROR;
        emit ErrorFindingGameDir();
        return;
    }
    status = ARENASTATUS_OUTOFGAME;
    if (file != nullptr) {
        file->deleteLater();
    }
    file = new QFile(dir.path() + "/replays/tempArenaInfo.json", this);
    connect(timer, SIGNAL(timeout()), this, SLOT(HandleTimeout()));
    timer->start(config->getLocalInterval());
}

bool ArenaListener::GetAreanInfo() {
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QByteArray content = file->readAll();
    QJsonParseError* err;
    err = new QJsonParseError();
    jsonDoc = new QJsonDocument(QJsonDocument::fromJson(content,err));
    if (err->error) {
        return false;
    }
    QJsonObject obj = jsonDoc->object();

    arena.selfPlayerName = obj.value("playerName").toString();
    QString selfShipName = obj.value("playerVehicle").toString();
    QStringList selfShipList = selfShipName.split("-");
    if (selfShipName.count() > 1) {
        arena.selfShipName = selfShipList.at(1);
    }
    else {
        arena.selfShipName = selfShipName;
    }

    QJsonArray players = obj.value("vehicles").toArray();
    arena.playerCount = players.count();
    for (int i = 0; i < arena.playerCount; i++) {
        QJsonObject player = players.at(i).toObject();
        arena.player[i].shipId = player.value("shipId").toDouble(-1);
        arena.player[i].name = ConvertUnicodeString(player.value("name").toString());
        arena.player[i].relation = player.value("relation").toInt(-1);
        arena.player[i].basicReady = true;
    }

    Debug_PrintAreanInfo();
    file->close();
    return true;
}

void ArenaListener::HandleTimeout() {
    
    //Update status
    if (!dir.exists() || file == nullptr) {
        status = ARENASTATUS_ERROR;
        timer->stop();
        emit ErrorFindingGameDir();
        return;
    }
    
    if (file->exists()) {
        if (status != ARENASTATUS_INGAME) {
            status = ARENASTATUS_INGAME;
            if (networkManager != nullptr) {
                networkManager->~NetworkManager();
                networkManager = nullptr;
            }
            arena = ArenaInfo();
            if (GetAreanInfo()) {
                networkManager = new NetworkManager(motsConfig, &arena, this);
                connect(networkManager, SIGNAL(NetworkInfoReady()), this, SLOT(HandleNetworkManager()));
                connect(networkManager, SIGNAL(Error()), this, SLOT(HandleNetworkError()));
                emit GameStart();
            }
            else {
                emit ErrorStartingGame();
            }
        }
    }
    else {
        if (status != ARENASTATUS_OUTOFGAME) {
            status = ARENASTATUS_OUTOFGAME;
            emit GameEnd();
        }
    }
    //qDebug() << status;
}

void ArenaListener::HandleNetworkManager() {
    Debug_PrintAreanInfo();
    emit AllInfoReady();
}

void ArenaListener::HandleNetworkError() {
    qDebug() << "____*Error____";
    emit ErrorNetworking();
    Debug_PrintAreanInfo();
}

void ArenaListener::Debug_PrintAreanInfo() {
    qDebug() << "------ArenaInfo------";
    qDebug() << "PlayerCount = " << arena.playerCount;
    qDebug() << "SelfName = " << arena.selfPlayerName;
    qDebug() << "SelfShipName = " << arena.selfShipName;
    qDebug() << "------PlayerInfo------";
    for (int i = 0; i < arena.playerCount; i++) {
        qDebug() << arena.player[i].id << "\t" << arena.player[i].name << "\t" << arena.player[i].relation
            << "\t" << arena.player[i].shipId << "\t" << arena.player[i].shipName;
    }
    qDebug() << "------InfoEnd------";
}

void ArenaListener::stopListening() {
    timer->stop();
}

void ArenaListener::startListening() {
    if (timer->interval() == 0) {
        timer->setInterval(1000);
    }
    timer->start();
}