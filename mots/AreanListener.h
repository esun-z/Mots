#pragma once

#include "global.h"
#include "NetworkManager.h"

#define AREANSTATUS_ERROR 0
#define AREANSTATUS_OUTOFGAME 1
#define AREANSTATUS_INGAME 2

class AreanListener : public QObject
{
	Q_OBJECT

public:
	AreanListener(MotsConfig* config, QObject* p);
	~AreanListener();

	void Init(MotsConfig* config, QObject* p);

	void stopListening();
	void startListening();

	int status;
	AreanInfo arean;

	NetworkManager* networkManager = nullptr;

private:
	QObject* parent;
	MotsConfig* motsConfig;
	QTimer* timer = nullptr;
	QFile* file = nullptr;
	QDir dir;

	bool CheckDir(QDir dir);
	bool GetAreanInfo();
	void Debug_PrintAreanInfo();
	/*long long GetValue(QString line, QString key);
	QString GetString(QString line, QString key);
	QString ConvertUnicodeString(QString ustr);*/

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
