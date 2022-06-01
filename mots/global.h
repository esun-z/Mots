#pragma once
#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QDataStream>
#include <QTimer>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>

#define MAXPLAYERCOUNT 100
#define MAXVERSIONCOUNT 10
#define PLACEHOLDER_PLAYERNAME "{playerName}"
#define PLACEHOLDER_PLAYERID "{playerId}"
#define PLACEHOLDER_SHIPID "{shipId}"
#define PLACEHOLDER_SHIPNAME "{shipName}"
#define PLACEHOLDER_APPLICATIONID "{WGApplicationId}"
#define APPLICATIONID "b9bcc3fe33f38c063d31c721f6433f83"

#define ROBOTCHECKURL "https://wowsgame.cn/"

#define SEARCHPROCESS_NOTREADY 0
#define SEARCHPROCESS_READY 1
#define SEARCHPROCESS_IDGET 2
#define SEARCHPROCESS_OVERVIEWGET 3
#define SEARCHPROCESS_SHIPGET 4

#define RELATION_SELF 0
#define RELATION_ALLY 1
#define RELATION_ENEMY 2

#define CURRENTCONFIGVERSION 0

struct PlayerScore {
	int battleCount;
	int winningCount;
	int averageExp;
	int averageDamage;

	PlayerScore() {
		battleCount = 0;
		winningCount = 0;
		averageExp = 0;
		averageDamage = 0;
	}
};

struct PlayerData {
	long long id;
	long long shipId;
	QString name;
	QString shipName;/*Not available for now because of lack of database.*/
	int relation;
	//bool ready;
	bool basicReady;
	//bool shipNameReady;
	bool networkInfoReady;
	int searchProcess;
	PlayerScore overviewScore;
	PlayerScore onShipScore;
	double averageTier;

	PlayerData() {
		id = -1;
		shipId = -1;
		relation = -1;
		//ready = false;
		basicReady = false;
		networkInfoReady = false;
		//shipNameReady = false;
		searchProcess = SEARCHPROCESS_NOTREADY;
		overviewScore = PlayerScore();
		onShipScore = PlayerScore();
		averageTier = 0;
	}
};

struct AreanInfo {
	PlayerData player[MAXPLAYERCOUNT];
	int playerCount;
	QString selfPlayerName;
	QString selfShipName;

	AreanInfo() {
		playerCount = 0;
		for (int i = 0; i < MAXPLAYERCOUNT; i++) {
			player[i] = PlayerData();
		}
	}
};

struct MotsConfig {
	int networkInterval;
	int localInterval;
	QString gamePath;
	QString getIdUrl;
	QString getOverviewUrl;
	QString getShipUrl;
	QString getShipNameUrl;
	QString applicationId;
	bool customApplicationId;

	MotsConfig() {
		networkInterval = 500;
		localInterval = 1000;
		gamePath = "C:/Games/World_of_Warships_CN360/";
		getIdUrl = QString::fromStdString("https://wowsgame.cn/community/accounts/search/?search=") + PLACEHOLDER_PLAYERNAME + "&pjax=1";
		getOverviewUrl = QString::fromStdString("https://wowsgame.cn/community/accounts/tab/pvp/overview/") + PLACEHOLDER_PLAYERID;
		getShipUrl = QString::fromStdString("https://wowsgame.cn/community/accounts/tab/pvp/ships/") + PLACEHOLDER_PLAYERID;
		getShipNameUrl = QString::fromStdString("https://api.worldofwarships.asia/wows/encyclopedia/ships/?application_id=") + PLACEHOLDER_APPLICATIONID + "&fields=name&language=zh-cn&ship_id=" + PLACEHOLDER_SHIPID;
		applicationId = APPLICATIONID;
		customApplicationId = false;
	}
};

QString ConvertUnicodeString(QString ustr);
QString GetString(QString line, QString key);
long long GetValue(QString line, QString key);