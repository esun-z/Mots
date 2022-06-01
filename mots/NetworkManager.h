#pragma once

#include "global.h"

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
		int searchProcess;

		NM(QObject* p) {
			manager = new QNetworkAccessManager(p);
			parent = p;
			searchProcess = SEARCHPROCESS_NOTREADY;
			reply = nullptr;
			request.setRawHeader("Accept", "*/*");
		}

		/*void push() {
			reply = manager->get(request);
			
		}*/
	};

	NetworkManager(MotsConfig* config, AreanInfo* arean, QObject *parent);
	~NetworkManager();
	QString lastError;
	void Stop();

private slots:
	void HandleReply();
private:
	
	NM* nm;
	AreanInfo* arean;
	MotsConfig* con;
	int currentPlayer = -1;
	bool PushRequest();
	QString GetContent(QString source, QString key);


	const QString OVERVIEWSEARCHKEY = "class=\"_values\">";
	const QString OVERVIEWENDKEY = "/div>*\n*</div>";
	const QString AVERAGETIERSEARCHKEY = QString::fromLocal8Bit("玩家所使用战舰的平均等级</td>");
	const QString AVERAGEDAMAGESEARCHKEY1 = QString::fromLocal8Bit("<h3 class=\"_title\">场均分数</h3>");
	const QString AVERAGEDAMAGESEARCHKEY2 = QString::fromLocal8Bit("<span>造成的伤害</span>");
	const QString SHIPNAMESEARCHKEY = "class=\"_text\">{shipName}</span></td>";

signals:
	void NetworkInfoReady();
	void Error();
};
