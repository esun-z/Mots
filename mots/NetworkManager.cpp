#include "NetworkManager.h"

NetworkManager::NetworkManager(MotsConfig* config, AreanInfo* areanInfo, QObject *parent)
	: QObject(parent)
{
	nm = new NM(this);
	//timer = new QTimer(this);
	//timer->setSingleShot(true);
	//timer->setInterval(config->networkInterval);
	arean = areanInfo;
	con = config;
	for (int i = 0; i < arean->playerCount; i++) {
		if (!arean->player[i].basicReady) {
			qDebug() << "* NetworkManager: Basic info not ready.";
			emit Error();
			return;
		}
	}
	PushRequest();
}

NetworkManager::~NetworkManager()
{
	disconnect(this);
	delete nm;
}

bool NetworkManager::PushRequest() {
	
	/*for (int i = 0; i < arean->playerCount; ++i) {
		qDebug() << arean->player[i].name;
	}*/
	currentPlayer++;
	if (currentPlayer >= arean->playerCount) {
		emit NetworkInfoReady();
		return false;
	}

	qDebug() << currentPlayer;
	QString idUrl = con->getIdUrl;
	
	if (arean->player[currentPlayer].searchProcess == SEARCHPROCESS_NOTREADY) {
		QString url = idUrl.replace(PLACEHOLDER_PLAYERNAME, arean->player[currentPlayer].name);
		//QString url = "https://wowsgame.cn/zh-cn/community/accounts/search/?search=esunz&pjax=1";
		qDebug() << "- newUrl = " + url;
		nm->request = QNetworkRequest();
		nm->request.setUrl(QUrl(url));
		nm->searchProcess = SEARCHPROCESS_NOTREADY;
		nm->reply = nm->manager->get(nm->request);
		connect(nm->reply, SIGNAL(finished()), this, SLOT(HandleReply()));
		return true;
	}
	else {
		emit Error();
		return false;
	}
}

void NetworkManager::HandleReply() {
	
	if (nm->reply->error()) {
		qDebug() << "Error: " + nm->reply->errorString();
		lastError = nm->reply->errorString();
		nm->reply->deleteLater();
		emit Error();
		PushRequest();
		return;
	}

	int statusCode = nm->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	qDebug() << "Status code: " << statusCode;

	if (statusCode >= 300 && statusCode < 400) {
		const QVariant redirectionTarget = nm->reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
		if (!redirectionTarget.isNull()) {
			QUrl url1 = nm->request.url();
			const QUrl redirectedUrl = url1.resolved(redirectionTarget.toUrl());

			nm->reply->deleteLater();
			nm->reply = nullptr;

			nm->request = QNetworkRequest();
			nm->request.setUrl(QUrl(redirectedUrl));
			if (nm->searchProcess == SEARCHPROCESS_OVERVIEWGET || nm->searchProcess == SEARCHPROCESS_SHIPGET) {
				nm->request.setRawHeader("X-Requested-With", "XMLHttpRequest");
			}
			nm->reply = nm->manager->get(nm->request);
			connect(nm->reply, SIGNAL(finished()), this, SLOT(HandleReply()));
			qDebug() << "Redirect to: " + redirectedUrl.toString();
		}
		return;
	}


	QString url;
	/*QTextCodec* codec = QTextCodec::codecForName("utf8");
	QString content = codec->toUnicode(nm->reply->readAll());*/
	QString content = nm->reply->readAll();
	switch (nm->searchProcess) {
	case SEARCHPROCESS_NOTREADY: {
		
		QString overviewUrl = GetContent(content, "og:url");
		
		QStringList l = overviewUrl.split("/");
		arean->player[currentPlayer].id = l.at(l.count() - 2).split("-").at(0).toLongLong();
		qDebug() << arean->player[currentPlayer].id;
		nm->reply->deleteLater();

		if (arean->player[currentPlayer].id == 0) {
			lastError = "Failed to find player: " + arean->player[currentPlayer].name;
			emit Error();
			PushRequest();
			break;
		}

		QString shipUrl = con->getShipNameUrl;
		if (con->customApplicationId) {
			shipUrl = shipUrl.replace(PLACEHOLDER_APPLICATIONID, APPLICATIONID);
		}
		else {
			shipUrl = shipUrl.replace(PLACEHOLDER_APPLICATIONID, con->applicationId);
		}
		QString nextUrl = shipUrl.replace(PLACEHOLDER_SHIPID, QString::number(arean->player[currentPlayer].shipId));
		nm->request.setUrl(nextUrl);

		nm->searchProcess = SEARCHPROCESS_IDGET;

		nm->reply = nm->manager->get(nm->request);
		connect(nm->reply, SIGNAL(finished()), this, SLOT(HandleReply()));
		break;
	}
	case SEARCHPROCESS_IDGET: {
		
		if (!content.contains("\"name\":\"")) {
			lastError = "Failed to find ship name through Wargaming public API. This is probably aroused by a new ship released in the latest version.";
			emit Error();
			PushRequest();
			break;
		}

		int pHead = content.indexOf("\"name\":\"", 0, Qt::CaseSensitive) + QString::fromStdString("\"name\": ").length();
		int pTail = content.indexOf("\"}", pHead, Qt::CaseSensitive);
		QString name = content.mid(pHead, pTail - pHead);
		qDebug() << name;
		arean->player[currentPlayer].shipName = name;

		nm->reply->deleteLater();
		QString overviewUrl = con->getOverviewUrl;
		QString nextUrl = overviewUrl.replace(PLACEHOLDER_PLAYERID, QString::number(arean->player[currentPlayer].id));
		nm->request.setUrl(nextUrl);
		nm->request.setRawHeader("X-Requested-With", "XMLHttpRequest");

		nm->searchProcess = SEARCHPROCESS_SHIPGET;
		
		nm->reply = nm->manager->get(nm->request);
		connect(nm->reply, SIGNAL(finished()), this, SLOT(HandleReply()));
		break;
	}
	case SEARCHPROCESS_SHIPGET: {

		int pHead = content.indexOf(OVERVIEWSEARCHKEY, 0, Qt::CaseSensitive) + OVERVIEWSEARCHKEY.length();
		int pTail = content.indexOf(OVERVIEWENDKEY, pHead, Qt::CaseSensitive);
		QString nums = content.mid(pHead, pTail - pHead);
		QStringList values;
		int sp, ep;
		for (int i = 0; i < 4; i++) {
			sp = nums.indexOf("<div>", 0, Qt::CaseSensitive) + QString::fromStdString("<div>").length();
			ep = nums.indexOf("<", sp, Qt::CaseSensitive);
			values << nums.mid(sp, ep - sp);
			nums = nums.mid(sp, nums.length() - sp);
			qDebug() << values.at(i);
		}
		arean->player[currentPlayer].overviewScore.battleCount = values.at(0).toInt();
		values[1] = values.at(1).left(values.length() - 1);
		arean->player[currentPlayer].overviewScore.winningCount = arean->player[currentPlayer].overviewScore.battleCount * values.at(1).toDouble() / 100;
		arean->player[currentPlayer].overviewScore.averageExp = values.at(2).toDouble();
		arean->player[currentPlayer].overviewScore.averageDamage = values.at(3).toDouble();

		nm->reply->deleteLater();
		QString shipUrl = con->getShipUrl;
		QString nextUrl = shipUrl.replace(PLACEHOLDER_PLAYERID, QString::number(arean->player[currentPlayer].id));
		nm->request = QNetworkRequest();
		nm->request.setUrl(nextUrl);
		nm->request.setRawHeader("X-Requested-With", "XMLHttpRequest");
		
		nm->searchProcess = SEARCHPROCESS_OVERVIEWGET;
		
		nm->reply = nm->manager->get(nm->request);
		connect(nm->reply, SIGNAL(finished()), this, SLOT(HandleReply()));
		break;
	}
	case SEARCHPROCESS_OVERVIEWGET: {
		//qDebug() << content.contains(codec->toUnicode("ƽ"));
		int ph = content.indexOf(AVERAGETIERSEARCHKEY, 0, Qt::CaseSensitive) + AVERAGETIERSEARCHKEY.length();
		qDebug() << "ph = " << ph;
		int pt = content.indexOf("/td>", ph, Qt::CaseSensitive);
		qDebug() << "pt = " << pt;
		QString avgTier = content.mid(ph, pt - ph);
		//qDebug() << avgTier;
		int sp = avgTier.indexOf(">", 0, Qt::CaseSensitive) + QString::fromStdString(">").length();
		int ep = avgTier.indexOf("<", sp, Qt::CaseSensitive);
		QString avgTierValue = avgTier.mid(sp, ep - sp);
		arean->player[currentPlayer].averageTier = avgTierValue.toDouble();
		qDebug() << avgTierValue;

		QString searchKey = SHIPNAMESEARCHKEY;
		searchKey = searchKey.replace(PLACEHOLDER_SHIPNAME, arean->player[currentPlayer].shipName);
		ph = content.indexOf(searchKey, 0, Qt::CaseSensitive) + searchKey.length();
		pt = content.indexOf("</tr>", ph, Qt::CaseSensitive);
		QString valueStr = content.mid(ph, pt - ph);
		QStringList values;
		for (int i = 0; i < 3; i++) {
			sp = valueStr.indexOf("<td class=\"_value\">", 0, Qt::CaseSensitive) + QString::fromStdString("<td class=\"_value\">").length();
			ep = valueStr.indexOf("</td>", sp, Qt::CaseSensitive);
			values << valueStr.mid(sp, ep - sp);
			valueStr = valueStr.mid(sp, valueStr.length() - sp);
			qDebug() << values.at(i);
		}
		arean->player[currentPlayer].onShipScore.battleCount = values.at(0).toInt();
		arean->player[currentPlayer].onShipScore.winningCount = values.at(1).toInt();
		arean->player[currentPlayer].onShipScore.averageExp = values.at(2).toDouble();

		if (arean->player[currentPlayer].onShipScore.battleCount != 0) {
			int pHead = content.indexOf(AVERAGEDAMAGESEARCHKEY1, ph, Qt::CaseSensitive) + AVERAGEDAMAGESEARCHKEY1.length();
			pHead = content.indexOf(AVERAGEDAMAGESEARCHKEY2, pHead, Qt::CaseSensitive) + AVERAGEDAMAGESEARCHKEY2.length();
			int pTail = content.indexOf("</tr>", pHead, Qt::CaseSensitive);
			QString num = content.mid(pHead, pTail - pHead);
			sp = num.indexOf("<span>", 0, Qt::CaseSensitive) + QString::fromStdString("<span>").length();
			ep = num.indexOf("</span>", sp, Qt::CaseSensitive);
			QString value = num.mid(sp, ep - sp);
			arean->player[currentPlayer].onShipScore.averageDamage = value.toDouble();
		}
		else {
			arean->player[currentPlayer].onShipScore.averageDamage = 0;
		}

		arean->player[currentPlayer].searchProcess = SEARCHPROCESS_READY;
		arean->player[currentPlayer].networkInfoReady = true;
		
		PushRequest();

		break;
	}
	default:
		break;
	}
}

QString NetworkManager::GetContent(QString source, QString key) {
	int startPoint = source.indexOf(key, 0, Qt::CaseSensitive) + key.length() + QString("\" ").length();
	int endPoint = source.indexOf("/>", startPoint, Qt::CaseSensitive);
	QString content = source.mid(startPoint, endPoint - startPoint);
	int sp = content.indexOf("content=\"", 0, Qt::CaseSensitive) + QString("content=\"").length();
	int ep = content.indexOf("\"", sp, Qt::CaseSensitive);
	return content.mid(sp, ep - sp);
}

void NetworkManager::Stop() {
	this->~NetworkManager();
}