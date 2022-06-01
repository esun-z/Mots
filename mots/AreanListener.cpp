#include "AreanListener.h"

AreanListener::AreanListener(MotsConfig* config, QObject *p)
	: QObject(p)
{
    Init(config, p);
}

AreanListener::~AreanListener()
{
    timer->deleteLater();
    timer = nullptr;
    if (file->isOpen()) {
        file->close();
    }
    file->deleteLater();
    file = nullptr;
}

bool AreanListener::CheckDir(QDir dir) {
    dir = dir.path() + "/replays";
    qDebug() << dir;
    return dir.exists();
}

void AreanListener::Init(MotsConfig* config, QObject* p) {
    parent = p;
    motsConfig = config;
    dir = config->gamePath;
    if (timer != nullptr) {
        timer->deleteLater();
    }
    timer = new QTimer(this);
    if (!CheckDir(config->gamePath)) {
        qDebug() << "* Error Finding Game Dir";
        status = AREANSTATUS_ERROR;
        emit ErrorFindingGameDir();
        return;
    }
    status = AREANSTATUS_OUTOFGAME;
    if (file != nullptr) {
        file->deleteLater();
    }
    file = new QFile(dir.path() + "/replays/tempArenaInfo.json", this);
    connect(timer, SIGNAL(timeout()), this, SLOT(HandleTimeout()));
    timer->start(config->localInterval);
}

//QString AreanListener::ConvertUnicodeString(QString ustr) {
//    do {
//        int idx = ustr.indexOf("\\u");
//        QString strHex = ustr.mid(idx, 6);
//        strHex = strHex.replace("\\u", QString());
//        int nHex = strHex.toInt(0, 16);
//        ustr.replace(idx, 6, QChar(nHex));
//    } while (ustr.indexOf("\\u") != -1);
//    return ustr;
//}
//
//QString AreanListener::GetString(QString line, QString key) {
//    line += ",";
//    QString k = "\"" + key + "\": ";
//    int pHead = line.indexOf(k, 0, Qt::CaseSensitive);
//    if (pHead == -1) {
//        qDebug() << "*Head";
//        qDebug() << k;
//        qDebug() << line;
//        return "";
//    }
//    pHead += k.length();
//    int pTail = line.indexOf(",", pHead, Qt::CaseSensitive);
//    if (pTail == -1) {
//        qDebug() << "*Tail";
//        
//        return "";
//    }
//
//    QString str = line.mid(pHead, pTail - pHead);
//
//    if (str.contains("\\u")) {
//        str = ConvertUnicodeString(str);
//    }
//
//    if (str.startsWith("\"")) {
//        str = str.mid(1, str.length() - 1);
//    }
//    if (str.endsWith("\"")) {
//        str = str.mid(0, str.length() - 1);
//    }
//
//    return str;
//}
//
//long long AreanListener::GetValue(QString line, QString key) {
//    
//    QString value = GetString(line, key);
//    return value.toLongLong();
//}

bool AreanListener::GetAreanInfo() {
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QString fileContent = file->readAll();
    int pHead = fileContent.indexOf("\"vehicles\": [", 0);
    if (pHead == -1) {
        return false;
    }
    pHead += QString::fromStdString("\"vehicles\": [").length();
    int pTail = fileContent.indexOf("]", pHead);
    if (pTail == -1) {
        return false;
    }
    
    QString playerInfoStr = fileContent.mid(pHead, pTail - pHead);
    playerInfoStr = playerInfoStr.mid(1, playerInfoStr.length() - 2);
    QStringList playerInfoList = playerInfoStr.split("}, {");
    
    arean.playerCount = playerInfoList.count();
    for (int i = 0; i < playerInfoList.count(); i++) {
        
        arean.player[i].shipId = GetValue(playerInfoList.at(i), "shipId");
        arean.player[i].relation = GetValue(playerInfoList.at(i), "relation");
        arean.player[i].name = GetString(playerInfoList.at(i), "name");
        arean.player[i].basicReady = true;

        if (arean.player[i].relation == 0) {
            arean.selfPlayerName = arean.player[i].name;
        }
    }
    QString selfShip = GetString(fileContent, "playerVehicle");
    QStringList selfShipList = selfShip.split("-");
    qDebug() << "SelfShipName = " << selfShipList;
    if (selfShipList.count() > 1) {
        arean.selfShipName = selfShipList.at(1);
    }
    else {
        arean.selfShipName = selfShip;
    }
    Debug_PrintAreanInfo();
    file->close();
    return true;
}

void AreanListener::HandleTimeout() {
    
    //Update status
    if (!dir.exists() || file == nullptr) {
        status = AREANSTATUS_ERROR;
        timer->stop();
        emit ErrorFindingGameDir();
        return;
    }
    
    if (file->exists()) {
        if (status != AREANSTATUS_INGAME) {
            status = AREANSTATUS_INGAME;
            if (networkManager != nullptr) {
                networkManager->Stop();
            }
            arean = AreanInfo();
            if (GetAreanInfo()) {
                networkManager = new NetworkManager(motsConfig, &arean, this);
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
        if (status != AREANSTATUS_OUTOFGAME) {
            status = AREANSTATUS_OUTOFGAME;
            emit GameEnd();
        }
    }
    //qDebug() << status;
}

void AreanListener::HandleNetworkManager() {
    Debug_PrintAreanInfo();
    emit AllInfoReady();
}

void AreanListener::HandleNetworkError() {
    qDebug() << "____*Error____";
    emit ErrorNetworking();
    Debug_PrintAreanInfo();
}

void AreanListener::Debug_PrintAreanInfo() {
    qDebug() << "------AreanInfo------";
    qDebug() << "PlayerCount = " << arean.playerCount;
    qDebug() << "SelfName = " << arean.selfPlayerName;
    qDebug() << "SelfShipName = " << arean.selfShipName;
    qDebug() << "------PlayerInfo------";
    for (int i = 0; i < arean.playerCount; i++) {
        qDebug() << arean.player[i].id << "\t" << arean.player[i].name << "\t" << arean.player[i].relation
            << "\t" << arean.player[i].shipId << "\t" << arean.player[i].shipName;
    }
    qDebug() << "------InfoEnd------";
}

void AreanListener::stopListening() {
    timer->stop();
}

void AreanListener::startListening() {
    if (timer->interval() ==0) {
        timer->setInterval(1000);
    }
    timer->start();
}

//QString AreanListener::FindVersionDir(QDir sourceDir) {
//    
//    QString resultDir = "";
//    QStringList dirList;
//    
//    if (!sourceDir.exists())
//    {
//        return resultDir;
//    }
//
//    QFileInfoList fileInfoList = sourceDir.entryInfoList();
//
//    foreach(QFileInfo fileInfo, fileInfoList)
//    {
//        if (fileInfo.fileName().contains("."))
//        {
//            continue;
//        }
//        if (fileInfo.isDir())
//        {
//            dirList << fileInfo.fileName();
//        }
//    }
//    
//    int version[MAXVERSIONCOUNT];
//    
//    for (int i = 0; i < dirList.count(); i++) {
//        version[i] = dirList.at(i).toInt();
//    }
//
//    std::sort(version, version + dirList.count());
//
//    resultDir = sourceDir.path() + QString::number(version[dirList.count() - 1]);
//    qDebug() << resultDir;
//    return resultDir;
//}
