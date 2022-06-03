#pragma once

#include "global.h"

class PrUtility : public QObject
{
	Q_OBJECT

public:
	PrUtility(MotsConfig *config, QObject *parent);
	~PrUtility();
	bool Init();
	bool CheckDate();

	QString lastError;
	bool ready;
	bool ReadJson(QByteArray ba);
	bool WriteJson(QByteArray ba);

	struct SHIPDATA {
		double avgWinRate;
		double avgDmg;
		double avgFrag;
	};

	std::map<long long, SHIPDATA> prList;

	void Debug_PrintPrList();

	int CalcPr(long long shipId, double winRate, double avgDmg, double avgFrag);

	static QColor GetColorFromPr(int pr);

private:
	MotsConfig* con = nullptr;
	QNetworkAccessManager* manager = nullptr;
	QNetworkRequest request;
	QNetworkReply* reply = nullptr;
	QJsonDocument* jsonDoc = nullptr;

	bool HandleRedirection();

private slots:
	void HandleNetworkReply();

signals:
	void PrError();
	void PrListReady();
};
