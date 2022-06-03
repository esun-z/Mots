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
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QRegularExpression>
#include <QColor>
#include <iostream>
#include <map>
#include <algorithm>

#define MAXPLAYERCOUNT 100
//#define MAXVERSIONCOUNT 10
#define MAXSHIPCOUNT 1000
#define PLACEHOLDER_PLAYERNAME "{playerName}"
#define PLACEHOLDER_PLAYERID "{playerId}"
#define PLACEHOLDER_SHIPID "{shipId}"
#define PLACEHOLDER_SHIPNAME "{shipName}"
#define PLACEHOLDER_APPLICATIONID "{WGApplicationId}"
#define APPLICATIONID "somethingStupid"

#define ROBOTCHECKURL "https://wowsgame.cn/"
#define PRLISTURL "http://proxy.wows.shinoaki.com:7152/public/ship/pr/list"

#define SEARCHPROCESS_WAITING 0
#define SEARCHPROCESS_IDGET 1
#define SEARCHPROCESS_OVERVIEWGET 2
#define SEARCHPROCESS_SHIPGET 4
#define SEARCHPROCESS_READY 8

#define RELATION_SELF 0
#define RELATION_ALLY 1
#define RELATION_ENEMY 2

#define CURRENTCONFIGVERSION 0

struct PlayerScore {
	int battleCount;
	int winningCount;
	int averageExp;
	int averageDamage;
	double frag;
	int pr;

	PlayerScore() {
		battleCount = 0;
		winningCount = 0;
		averageExp = 0;
		averageDamage = 0;
		frag = 0;
		pr = 0;
	}
};

struct PlayerData {
	long long id;
	long long shipId;
	QString name;
	QString shipName;
	int relation;
	//bool ready;
	bool basicReady;
	//bool shipNameReady;
	bool networkInfoReady;
	bool networkError;
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
		searchProcess = SEARCHPROCESS_WAITING;
		overviewScore = PlayerScore();
		onShipScore = PlayerScore();
		averageTier = 0;
		networkError = false;
	}
};

struct ArenaInfo {
	PlayerData player[MAXPLAYERCOUNT];
	int playerCount;
	QString selfPlayerName;
	QString selfShipName;

	ArenaInfo() {
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

	int getNetworkInterval() {
		if (networkInterval <= 0) {
			networkInterval = 1;
		}
		return networkInterval;
	}

	int getLocalInterval() {
		if (localInterval <= 0) {
			localInterval = 1;
		}
		return localInterval;
	}
};

QString ConvertUnicodeString(QString ustr);