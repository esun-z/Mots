#include "global.h"

QString ConvertUnicodeString(QString ustr) {
	do {
		int idx = ustr.indexOf("\\u");
		QString strHex = ustr.mid(idx, 6);
		strHex = strHex.replace("\\u", QString());
		int nHex = strHex.toInt(0, 16);
		ustr.replace(idx, 6, QChar(nHex));
	} while (ustr.indexOf("\\u") != -1);
	return ustr;
}

QString GetString(QString line, QString key) {
	line += ",";
	QString k = "\"" + key + "\": ";
	int pHead = line.indexOf(k, 0, Qt::CaseSensitive);
	if (pHead == -1) {
		qDebug() << "*Head";
		
		return "";
	}
	pHead += k.length();
	int pTail = line.indexOf(",", pHead, Qt::CaseSensitive);
	if (pTail == -1) {
		qDebug() << "*Tail";

		return "";
	}
	
	QString str = line.mid(pHead, pTail - pHead);

	if (str.contains("\\u")) {
		str = ConvertUnicodeString(str);
	}

	if (str.startsWith("\"")) {
		str = str.mid(1, str.length() - 1);
	}
	if (str.endsWith("\"")) {
		str = str.mid(0, str.length() - 1);
	}

	return str;
}

long long GetValue(QString line, QString key) {

	QString value = GetString(line, key);
	return value.toLongLong();
}