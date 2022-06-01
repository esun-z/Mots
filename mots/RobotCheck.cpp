#include "RobotCheck.h"

RobotCheck::RobotCheck(QString url, QObject *parent)
	: QObject(parent)
{
	if (!url.endsWith("/")) {
		url += "/";
	}
	req.setUrl(url + "robots.txt");
	nm = new QNetworkAccessManager(this);
	rep = nm->get(req);
	connect(rep, SIGNAL(finished()), this, SLOT(HandleReply()));
}

RobotCheck::~RobotCheck()
{
}

void RobotCheck::HandleReply() {
	if (rep->error()) {
		lastError = rep->errorString();
		rep->deleteLater();
		rep = nullptr;
		emit Error();
		return;
	}

	int statusCode = rep->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	qDebug() << "Status code: " << statusCode;

	if (statusCode >= 300 && statusCode < 400) {
		const QVariant redirectionTarget = rep->attribute(QNetworkRequest::RedirectionTargetAttribute);
		if (!redirectionTarget.isNull()) {
			QUrl url1 = req.url();
			const QUrl redirectedUrl = url1.resolved(redirectionTarget.toUrl());

			rep->deleteLater();
			rep = nullptr;

			req = QNetworkRequest();
			req.setUrl(QUrl(redirectedUrl));
			
			rep = nm->get(req);
			connect(rep, SIGNAL(finished()), this, SLOT(HandleReply()));
			qDebug() << "Redirect to: " + redirectedUrl.toString();
		}
		return;
	}

	QString content = rep->readAll();
	bool allow = false;
	if (content.contains("\nAllow: /community/\n"),Qt::CaseSensitive) allow = true;
	if (allow) {
		emit RobotAllow();
	}
	else {
		lastError = content;
		emit RobotDisallow();
	}
}