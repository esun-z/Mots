#pragma once

#include "global.h"
#include "ReplyReader.h"
#include "PrUtility.h"
//#define OVERVIEWSEARCHKEY "<div class=\"_names\">\n            <div>参战场次</div>\n            <div>获胜场次/战斗场次</div>\n            <div class=\"_average-exp\">场均经验</div>\n            <div>场均伤害</div>\n            <div>击杀/死亡比</div>\n        </div>";



class NetworkManager : public QObject
{
	Q_OBJECT


public:

	struct NM {
		QNetworkAccessManager* manager;
		QNetworkRequest request;
		QNetworkReply* reply;
		QObject* parent;
		int currentPlayer;
		//int redirectionCount;
		//int searchProcess;

		NM(QObject* p) {
			manager = new QNetworkAccessManager(p);
			parent = p;
			//searchProcess = SEARCHPROCESS_WAITING;
			reply = nullptr;
			request.setRawHeader("Accept", "*/*");
			currentPlayer = 0;
			//redirectionCount = 0;
		}

		bool Get(QUrl url, bool isXml) {
			if (manager == nullptr) {
				return false;
			}
			if (reply != nullptr) {
				reply->deleteLater();
			}
			request = QNetworkRequest();
			request.setUrl(url);
			if (isXml) {
				request.setRawHeader("X-Requested-With", "XMLHttpRequest");
			}
			//redirectionCount = 0;
			reply = manager->get(request);
			return true;
		}

		bool HandleRedirection(bool isXml) {

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
			return Get(redirectedUrl, isXml);

		}
	};

	NetworkManager(MotsConfig* config, ArenaInfo* arena, QObject *parent);
	~NetworkManager();
	QString lastError;
	bool prReady = false;
	//void Stop();

private slots:
	void HandleIdReply();
	void HandleOvReply();
	void HandleShipReply();

	void HandlePrReady();
	void HandlePrError();
private:
	
	NM *nmId = nullptr, *nmOverview = nullptr, *nmShip = nullptr;
	PrUtility* pr = nullptr;
	ArenaInfo* arena;
	MotsConfig* con;
	//int currentPlayer = -1;
	//bool PushRequest();
	//QString GetContent(QString source, QString key);
	//int redirectionCount = 0;
	bool idFinished = false;

	bool PushId();
	bool PushOverview();
	bool PushShip();
	bool CheckFinish();

	void SendReady();
	void CalculatePr(int p);

	static bool cmpByPr(PlayerData a, PlayerData b) {
		return a.onShipScore.pr > b.onShipScore.pr;
	}
signals:
	void NetworkInfoReady();
	void Error();
};
