#pragma once

#include "global.h"

class RobotCheck : public QObject
{
	Q_OBJECT

public:
	RobotCheck(QString url, QObject *parent);
	~RobotCheck();

	QString lastError;

private:
	QNetworkAccessManager* nmId = nullptr;
	QNetworkRequest req;
	QNetworkReply* rep = nullptr;

signals:
	void RobotAllow();
	void RobotDisallow();
	void Error();

private slots:
	void HandleReply();
};
