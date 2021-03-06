#include "NetworkManager.h"

NetworkManager::NetworkManager(MotsConfig* config, ArenaInfo* areanInfo, QObject *parent)
	: QObject(parent)
{
	nmId = new NM(this);
	nmOverview = new NM(this);
	nmShip = new NM(this);
	/*connect(nmId->reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(HandleIdNetworkError(QNetworkReply::NetworkError)));
	connect(nmOverview->reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(HandleOverviewNetworkError(QNetworkReply::NetworkError)));
	connect(nmShip->reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(HandleShipNetworkError(QNetworkReply::NetworkError)));*/

	arena = areanInfo;
	con = config;
	for (int i = 0; i < arena->playerCount; i++) {
		if (!arena->player[i].basicReady) {
			qDebug() << "NetworkManager: Basic info not ready.";
			emit Error();
			return;
		}
	}

	pr = new PrUtility(config, this);
	connect(pr, SIGNAL(PrListReady()), this, SLOT(HandlePrReady()));
	connect(pr, SIGNAL(PrError()), this, SLOT(HandlePrError()));
	if (pr->Init()) {
		prReady = true;
	}
	PushId();
}

NetworkManager::~NetworkManager()
{
	disconnect(this);
	if (nmId != nullptr) {
		delete nmId;
	}
	if (nmOverview != nullptr) {
		delete nmOverview;
	}
	if (nmShip != nullptr) {
		delete nmShip;
	}
	if (pr != nullptr) {
		pr->deleteLater();
		pr = nullptr;
	}
}

void NetworkManager::HandleIdNetworkError(QNetworkReply::NetworkError e) {
	lastError = "Network error " + QString::number(e);
	emit Error();
	nmId->currentPlayer++;
	PushId();
}

void NetworkManager::HandleOverviewNetworkError(QNetworkReply::NetworkError e) {
	lastError = "Network Error " + QString::number(e);
	emit Error();
	arena->player[nmOverview->currentPlayer].networkError = true;
	PushOverview();
}

void NetworkManager::HandleShipNetworkError(QNetworkReply::NetworkError e) {
	lastError = "Network Error " + QString::number(e);
	emit Error();
	arena->player[nmShip->currentPlayer].networkError = true;
	PushShip();
}

void NetworkManager::HandlePrReady() {
	qDebug() << "NM: PR ready received.";
	prReady = true;
	//pr->Debug_PrintPrList();
}

void NetworkManager::HandlePrError() {
	lastError = "Failed to get PR list.";
	emit Error();
}

bool NetworkManager::CheckFinish() {
	bool finished = true;
	qDebug() << "________________________________";
	for (int i = 0; i < arena->playerCount; i++) {
		std::cout << arena->player[i].searchProcess << "\t";
	}
	qDebug() << "--------------------------------";
	for (int i = 0; i < arena->playerCount; i++) {
		if (arena->player[i].searchProcess == SEARCHPROCESS_READY) {
			arena->player[i].networkInfoReady = true;
		}
		if (!arena->player[i].networkInfoReady && !arena->player[i].networkError) {
			finished = false;
			break;
		}
	}
	return finished;
}

bool NetworkManager::PushId() {

	//nmId->currentPlayer++;
	if (idFinished || nmId->currentPlayer >= arena->playerCount) {
		qDebug() << "Id Fin";
		idFinished = true;
		return false;
	}

	QString url = con->getIdUrl;
	url.replace(PLACEHOLDER_PLAYERNAME, arena->player[nmId->currentPlayer].name);
	if (!nmId->Get(url, false)) {
		lastError = "Network Manager: has not been initialized.";
		emit Error();
		return false;
	}

	qDebug() << "Push Id: " << nmId->currentPlayer;
	connect(nmId->reply, SIGNAL(finished()), this, SLOT(HandleIdReply()));

	return true;
}

bool NetworkManager::PushOverview() {
	if (nmOverview->reply != nullptr) {
		qDebug() << "Ov: working";
		return false;
	}
	if (CheckFinish()) {
		return false;
	}
	bool p = false;
	for (int i = 0; i < arena->playerCount; i++) {
		if (arena->player[i].searchProcess < SEARCHPROCESS_READY && arena->player[i].searchProcess >= SEARCHPROCESS_IDGET && arena->player[i].searchProcess != SEARCHPROCESS_OVERVIEWGET && !arena->player[i].networkError) {
			nmOverview->currentPlayer = i;
			p = true;
			break;
		}
	}
	if (!p) {
		return false;
	}
	qDebug() << "Ov: current = " << nmOverview->currentPlayer;
	//nmOverview->currentPlayer++;
	/*if (overviewFinished || nmOverview->currentPlayer >= arena->playerCount) {
		qDebug() << "Ov Fin";
		overviewFinished = true;
		return false;
	}

	if (arena->player[nmOverview->currentPlayer].searchProcess < SEARCHPROCESS_IDGET) {
		qDebug() << "Ov: ID not ready" << nmOverview->currentPlayer;
		return false;
	}

	if (arena->player[nmOverview->currentPlayer].searchProcess >= SEARCHPROCESS_READY) {
		qDebug() << "Ov: Already done" << nmOverview->currentPlayer;
		return false;
	}*/
	qDebug() << "Push Ov: " << nmOverview->currentPlayer;
	QString url = con->getOverviewUrl;
	url.replace(PLACEHOLDER_PLAYERID, QString::number(arena->player[nmOverview->currentPlayer].id));

	nmOverview->Get(url, true);
	connect(nmOverview->reply, SIGNAL(finished()), this, SLOT(HandleOvReply()));
	return true;
}

bool NetworkManager::PushShip() {
	if (nmShip->reply != nullptr) {
		qDebug() << "Ship: working";
		return false;
	}
	if (CheckFinish()) {
		return false;
	}
	bool p = false;
	for (int i = 0; i < arena->playerCount; i++) {
		if (arena->player[i].searchProcess < SEARCHPROCESS_READY && arena->player[i].searchProcess >= SEARCHPROCESS_IDGET && arena->player[i].searchProcess != SEARCHPROCESS_SHIPGET && !arena->player[i].networkError) {
			nmShip->currentPlayer = i;
			p = true;
			break;
		}
	}
	if (!p) {
		return false;
	}
	qDebug() << "Ship: current = " << nmShip->currentPlayer;
	//nmShip->currentPlayer++;
	/*if (shipFinished || nmShip->currentPlayer >= arena->playerCount) {
		qDebug() << "Ship Fin";
		shipFinished = true;
		return false;
	}

	if (arena->player[nmShip->currentPlayer].searchProcess < SEARCHPROCESS_IDGET) {
		qDebug() << "Ship: ID not ready." << nmShip->currentPlayer;
		return false;
	}

	if (arena->player[nmShip->currentPlayer].searchProcess >= SEARCHPROCESS_READY) {
		qDebug() << "Ship: Already done" << nmShip->currentPlayer << arena[nmShip->currentPlayer].player->searchProcess;
		return false;
	}*/
	qDebug() << "Push Ship: " << nmShip->currentPlayer;
	QString url = con->getShipUrl;
	url.replace(PLACEHOLDER_PLAYERID, QString::number(arena->player[nmShip->currentPlayer].id));

	nmShip->Get(url, true);
	connect(nmShip->reply, SIGNAL(finished()), this, SLOT(HandleShipReply()));
	return true;
}

void NetworkManager::HandleIdReply() {
	if (nmId->reply->error()) {
		arena->player[nmId->currentPlayer].networkError = true;
		lastError = nmId->reply->errorString();
		emit Error();
		nmId->currentPlayer++;
		PushId();
		return;
	}

	int statusCode = nmId->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if (statusCode >= 300 && statusCode < 400) {
		const QVariant redirectionTarget = nmId->reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
		if (redirectionTarget.isNull()) {
			lastError = "Redirection target is null.";
			emit Error();
			nmId->currentPlayer++;
			PushId();
			return;
		}

		QUrl originalUrl = nmId->request.url();
		const QUrl redirectedUrl = originalUrl.resolved(redirectionTarget.toUrl());

		if (redirectedUrl.toString().contains(QRegularExpression("https://wowsgame.cn/zh-cn/community/accounts/\\d{8,}-.+"))) {
			QStringList urlStrList = redirectedUrl.toString().split("/");
			if (urlStrList.at(urlStrList.count() - 1).isEmpty()) {
				urlStrList.takeLast();
			}
			qDebug() << urlStrList;
			QString userIdStr = urlStrList.at(urlStrList.count() - 1).split("-").at(0);
			bool* ok = new bool(true);
			arena->player[nmId->currentPlayer].id = userIdStr.toLongLong(ok);
			if (!ok) {
				lastError = "Failed to find player: " + arena->player[nmId->currentPlayer].name;
				emit Error();

				PushId();
				return;
			}
			arena->player[nmId->currentPlayer].searchProcess = SEARCHPROCESS_IDGET;
			nmId->currentPlayer++;
			PushId();
			PushOverview();
			PushShip();
		}
		else {
			nmId->Get(redirectedUrl, false);
			connect(nmId->reply, SIGNAL(finished()), this, SLOT(HandleIdReply()));
			qDebug() << "Redirected to: " << redirectedUrl;
		}
	}
	else {
		arena->player[nmId->currentPlayer].networkError = true;
		lastError = "Failed to get player info: " + arena->player[nmId->currentPlayer].name;
		emit Error();
		nmId->currentPlayer++;
		PushId();
	}
}

void NetworkManager::CalculatePr(int p) {
	if (p >= arena->playerCount) {
		return;
	}
	if (arena->player[p].onShipScore.battleCount < 5) {
		return;
	}
	double winRate = (double)arena->player[p].onShipScore.winningCount / arena->player[p].onShipScore.battleCount * 100;
	arena->player[p].onShipScore.pr =
		pr->CalcPr(arena->player[p].shipId, winRate, arena->player[p].onShipScore.averageDamage, arena->player[p].onShipScore.frag);

}

void NetworkManager::SendReady() {

	if (prReady) {
		std::sort(arena->player, arena->player + arena->playerCount, cmpByPr);
	}
	emit NetworkInfoReady();
}

void NetworkManager::HandleOvReply() {

	if (nmShip->reply->error()) {
		HandleShipNetworkError(nmShip->reply->error());
	}

	if (nmOverview->HandleRedirection(true)) {
		connect(nmOverview->reply, SIGNAL(finished()), this, SLOT(HandleOvReply()));
		return;
	}
	QString content = nmOverview->reply->readAll();
	QString strTemp = ReplyReader::GetBetween(content, QString::fromLocal8Bit("整体成绩"), QString::fromLocal8Bit("存活场次"));
	QString battleCountStr = ReplyReader::GetRight(strTemp, QString::fromLocal8Bit("战斗场次"));
	QString winningCountStr = ReplyReader::GetRight(strTemp, QString::fromLocal8Bit("胜利"));
	arena->player[nmOverview->currentPlayer].overviewScore.battleCount = ReplyReader::GetNumBetween(battleCountStr, "<span>", "</span>");
	arena->player[nmOverview->currentPlayer].overviewScore.winningCount = ReplyReader::GetNumBetween(winningCountStr, "<span>", "</span>");

	strTemp = ReplyReader::GetBetween(content, QString::fromLocal8Bit("场均分数"), QString::fromLocal8Bit("击落飞机数"));
	QString avgExpStr = ReplyReader::GetRight(strTemp, QString::fromLocal8Bit("经验"));
	QString avgDmgStr = ReplyReader::GetRight(strTemp, QString::fromLocal8Bit("造成的伤害"));
	QString avgFragStr = ReplyReader::GetRight(strTemp, QString::fromLocal8Bit("击毁战舰数"));
	arena->player[nmOverview->currentPlayer].overviewScore.averageExp = ReplyReader::GetNumBetween(avgExpStr, "<span>", "</span>");
	arena->player[nmOverview->currentPlayer].overviewScore.averageDamage = ReplyReader::GetNumBetween(avgDmgStr, "<span>", "</span>");
	arena->player[nmOverview->currentPlayer].overviewScore.frag = ReplyReader::GetNumBetween(avgFragStr, "<span>", "</span>");

	arena->player[nmOverview->currentPlayer].searchProcess *= SEARCHPROCESS_OVERVIEWGET;
	nmOverview->reply->deleteLater();
	nmOverview->reply = nullptr;
	if (!CheckFinish()) {
		PushOverview();
	}
	else {
		SendReady();
	}
}

void NetworkManager::HandleShipReply() {

	qDebug() << "HandleShipReply();";

	if (nmShip->reply->error()) {
		HandleShipNetworkError(nmShip->reply->error());
	}

	if (nmShip->HandleRedirection(true)) {
		connect(nmShip->reply, SIGNAL(finished()), this, SLOT(HandleShipReply()));
		return;
	}
	QString content = nmShip->reply->readAll();
	
	QString avgTierStr = ReplyReader::GetBetween(content, QString::fromLocal8Bit("玩家所使用战舰的平均等级</td>"), "/td>");
	arena->player[nmShip->currentPlayer].averageTier = ReplyReader::GetNumBetween(avgTierStr, ">", "<");
	
	QString shipIdStr = QString::number(arena->player[nmShip->currentPlayer].shipId);
	QString strTemp = ReplyReader::GetBetween(content, shipIdStr, shipIdStr);
	QString shipName = ReplyReader::GetBetween(strTemp, "<span class=\"_text\">", "</span></td>");
	if (shipName.isEmpty()) {
		arena->player[nmShip->currentPlayer].noShipData = true;
	}
	else {
		arena->player[nmShip->currentPlayer].noShipData = false;
		arena->player[nmShip->currentPlayer].shipName = shipName;
		strTemp = ReplyReader::GetBetween(content, "ref=\"" + shipIdStr, QString::fromLocal8Bit("最高记录"));
		QString battleCountStr = ReplyReader::GetRight(strTemp, QString::fromLocal8Bit("战斗场次"));
		QString winningCountStr = ReplyReader::GetRight(strTemp, QString::fromLocal8Bit("胜利"));
		arena->player[nmShip->currentPlayer].onShipScore.battleCount = ReplyReader::GetNumBetween(battleCountStr, "<span>", "</span>");
		arena->player[nmShip->currentPlayer].onShipScore.winningCount = ReplyReader::GetNumBetween(winningCountStr, "<span>", "</span>");

		QString avgStr = ReplyReader::GetRight(strTemp, QString::fromLocal8Bit("场均分数"));
		QString avgExpStr = ReplyReader::GetRight(avgStr, QString::fromLocal8Bit("经验"));
		QString avgDmgStr = ReplyReader::GetRight(avgStr, QString::fromLocal8Bit("造成的伤害"));
		QString avgFragStr = ReplyReader::GetRight(avgStr, QString::fromLocal8Bit("击毁战舰数"));
		arena->player[nmShip->currentPlayer].onShipScore.averageExp = ReplyReader::GetNumBetween(avgExpStr, "<span>", "</span>");
		arena->player[nmShip->currentPlayer].onShipScore.averageDamage = ReplyReader::GetNumBetween(avgDmgStr, "<span>", "</span>");
		arena->player[nmShip->currentPlayer].onShipScore.frag = ReplyReader::GetNumBetween(avgFragStr, "<span>", "</span>");

		if (prReady) {
			CalculatePr(nmShip->currentPlayer);
		}
	}
	
	arena->player[nmShip->currentPlayer].searchProcess *= SEARCHPROCESS_SHIPGET;
	nmShip->reply->deleteLater();
	nmShip->reply = nullptr;
	if (!CheckFinish()) {
		PushShip();
	}
	else {
		SendReady();
	}
}
