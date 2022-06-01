#pragma once

#include "global.h"

//#define OVERVIEWSEARCHKEY "<div class=\"_names\">\n            <div>��ս����</div>\n            <div>��ʤ����/ս������</div>\n            <div class=\"_average-exp\">��������</div>\n            <div>�����˺�</div>\n            <div>��ɱ/������</div>\n        </div>";



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
	const QString AVERAGETIERSEARCHKEY = QString::fromLocal8Bit("�����ʹ��ս����ƽ���ȼ�</td>");
	const QString AVERAGEDAMAGESEARCHKEY1 = QString::fromLocal8Bit("<h3 class=\"_title\">��������</h3>");
	const QString AVERAGEDAMAGESEARCHKEY2 = QString::fromLocal8Bit("<span>��ɵ��˺�</span>");
	const QString SHIPNAMESEARCHKEY = "class=\"_text\">{shipName}</span></td>";

signals:
	void NetworkInfoReady();
	void Error();
};
