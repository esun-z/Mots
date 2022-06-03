#include "PrUtility.h"

PrUtility::PrUtility(MotsConfig *config, QObject *parent)
	: QObject(parent)
{
	con = config;
	qDebug() << "PU: start init.";
	if (!Init()) {
		qDebug() << "PU: init error.";
		emit PrError();
	}
	else {
		qDebug() << "PU: init finished.";
	}
}

PrUtility::~PrUtility()
{
}

bool PrUtility::Init() {
	if (!CheckDate()) {
		if (manager != nullptr) {
			manager->deleteLater();
		}
		if (reply != nullptr) {
			reply->deleteLater();
		}
		manager = new QNetworkAccessManager(this);
		
		request.setUrl(QUrl(PRLISTURL));
		reply = manager->get(request);
		connect(reply, SIGNAL(finished()), this, SLOT(HandleNetworkReply()));
	}
	else {
		QFile f("prList.json");
		if (!f.open(QIODevice::ReadOnly)) {
			qDebug() << "PU: Failed to open local file";
			return false;
		}
		QByteArray ba = f.readAll();
		f.close();
		if (ReadJson(ba)) {
			ready = true;
			emit PrListReady();
			qDebug() << "PU: Pr List Ready Emitted.";
			return true;
		}
		else {
			qDebug() << "PU: Failed to read local file";
			return false;
		}
	}
	return true;
}

bool PrUtility::ReadJson(QByteArray ba) {
	QJsonParseError* err;
	err = new QJsonParseError();
	jsonDoc = new QJsonDocument(QJsonDocument::fromJson(ba, err));
	if (err->error) {
		return false;
	}
	QJsonObject obj = jsonDoc->object();
	QString suc = obj.value("message").toString();
	if (!suc.contains("success")) {
		return false;
	}
	QJsonArray data = obj.value("data").toArray();
	for (int i = 0; i < data.count(); i++) {
		QJsonObject v = data.at(i).toObject();
		SHIPDATA s = { v.value("winRate").toDouble(),v.value("averageDamageDealt").toDouble(),v.value("averageFrags").toDouble() };
		long long shipId = v.value("idCode").toDouble();
		//qDebug() << shipId;
		prList.insert(std::pair<long long, SHIPDATA>(shipId, s));
	}
	WriteJson(ba);
	return true;
}

bool PrUtility::WriteJson(QByteArray ba) {
	QFile f("prList.json");
	if (!f.open(QIODevice::WriteOnly)) {
		return false;
	}
	f.write(jsonDoc->toJson());
	f.close();
	return true;
}

bool PrUtility::CheckDate() {
	bool inDate = true;
	QFile f("prConfig.cfg");

	do {
		if (!f.exists()) {
			inDate = false;
			break;
		}
		if (!f.open(QIODevice::ReadOnly)) {
			inDate = false;
			break;
		}
		QDateTime time;
		QDataStream s(&f);
		s >> time;
		qDebug() << "PU: Pr time: " << time;
		f.close();
		if (QDateTime::currentSecsSinceEpoch() - time.toSecsSinceEpoch() > 60 * 60 * 24) {
			inDate = false;
		}
		break;
	} while (false);
	qDebug() << "PU: Pr in Date: " << inDate;
	if (!inDate) {
		f.open(QIODevice::WriteOnly);
		QDataStream sw(&f);
		sw << QDateTime::currentDateTime();
		f.close();
	}

	return inDate;
	
}

bool PrUtility::HandleRedirection() {
	
	if (reply == nullptr) {
		return false;
	}
	int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	if (statusCode < 300 || statusCode >= 400) {
		return false;
	}

	const QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
	if (redirectionTarget.isNull()) {
		return false;
	}
	QUrl originalUrl = request.url();
	const QUrl redirectedUrl = originalUrl.resolved(redirectionTarget.toUrl());
	
		
	if(manager == nullptr) {
		return false;
	}
	if (reply != nullptr) {
		reply->deleteLater();
	}
	request = QNetworkRequest();
	request.setUrl(redirectedUrl);
	
	reply = manager->get(request);
	return true;
}

void PrUtility::HandleNetworkReply() {
	if (reply->error()) {
		lastError = reply->errorString();
		emit PrError();
		return;
	}

	if (HandleRedirection()) {
		connect(reply, SIGNAL(finished()), this, SLOT(HandleNetworkReply()));
		return;
	}

	QByteArray ba = reply->readAll();
	if (!ReadJson(ba)) {
		emit PrError();
	}
	else {
		ready = true;
		emit PrListReady();
	}
}

int PrUtility::CalcPr(long long shipId, double winRate, double avgDmg, double avgFrag) {
	double eWinRate = prList.at(shipId).avgWinRate;
	double eDmg = prList.at(shipId).avgDmg;
	double eFrag = prList.at(shipId).avgFrag;

	if (eWinRate <= 0 || eDmg <= 0 || eFrag <= 0) {
		return 0;
	}

	double rWinRate = winRate / eWinRate;
	double rDmg = avgDmg / eDmg;
	double rFrag = avgFrag / eFrag;

	double nWinRate, nDmg, nFrag;
	nWinRate = std::max((double)0, (rWinRate - 0.7) / (1 - 0.7));
	nDmg = std::max((double)0, (rDmg - 0.4) / (1 - 0.4));
	nFrag = std::max((double)0, (rFrag - 0.1) / (1 - 0.1));

	qDebug() << shipId << " " << rWinRate << " " << rDmg << " " << rFrag << " " << winRate << " " << eWinRate;

	return 700 * nDmg + 300 * nFrag + 150 * nWinRate;

}

QColor PrUtility::GetColorFromPr(int pr) {
	if (pr == 0) {
		return QColor(0, 0, 0, 0);
	}
	if (pr < 750) {
		return QColor(254, 14, 0, 50);
	}
	if (pr < 1100) {
		return QColor(254, 121, 3, 50);
	}
	if (pr < 1350) {
		return QColor(255, 199, 31, 50);
	}
	if (pr < 1550) {
		return QColor(68, 179, 0, 50);
	}
	if (pr < 1750) {
		return QColor(49, 128, 0, 50);
	}
	if (pr < 2100) {
		return QColor(2, 201, 179, 50);
	}
	if (pr < 2450) {
		return QColor(208, 66, 243, 50);
	}
	return QColor(160, 13, 197, 50);
}

void PrUtility::Debug_PrintPrList() {
	for (auto it : prList) {
		std::cout << it.first << " " << it.second.avgDmg << std::endl;
	}
}
