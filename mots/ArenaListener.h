#pragma once

#include "global.h"
#include "NetworkManager.h"

#define ARENASTATUS_ERROR 0
#define ARENASTATUS_OUTOFGAME 1
#define ARENASTATUS_INGAME 2

class ArenaListener : public QObject
{
	Q_OBJECT

public:
	ArenaListener(MotsConfig* config, QObject* p);
	~ArenaListener();

	void Init(MotsConfig* config, QObject* p);

	void stopListening();
	void startListening();

	int status;
	ArenaInfo arena;

	NetworkManager* networkManager = nullptr;

private:
	QObject* parent;
	MotsConfig* motsConfig;
	QTimer* timer = nullptr;
	QFile* file = nullptr;
	QJsonDocument* jsonDoc = nullptr;
	QDir dir;

	bool CheckDir(QDir dir);
	bool GetAreanInfo();
	void Debug_PrintAreanInfo();

private slots:
	void HandleTimeout();
	void HandleNetworkManager();
	void HandleNetworkError();

signals:
	void ErrorFindingGameDir();
	void ErrorStartingGame();
	void ErrorNetworking();
	void GameStart();
	void AllInfoReady();
	void GameEnd();

};
