#include "ReplyReader.h"

QString ReplyReader::GetBetween(QString str, QString sk, QString ek) {
	int pHead = str.indexOf(sk, 0, Qt::CaseSensitive);
	if (pHead == -1) {
		qDebug() << "* Cannot find start key: " << sk;
		return "";
	}
	pHead += sk.length();
	int pTail = str.indexOf(ek, pHead, Qt::CaseSensitive);
	if (pTail == -1) {
		qDebug() << "* Cannot find end key: " << ek;
		return "";
	}
	return str.mid(pHead, pTail - pHead);
}

QString ReplyReader::GetRight(QString str, QString sk) {
	int pHead = str.indexOf(sk, 0, Qt::CaseSensitive);
	if (pHead == -1) {
		qDebug() << "* Cannot find start key: " << sk;
		return "";
	}
	pHead += sk.length();
	return str.right(str.length() - pHead);
}

double ReplyReader::GetNumBetween(QString str, QString sk, QString ek) {
	return GetBetween(str, sk, ek).toDouble();
}

long long ReplyReader::GetllNumBetween(QString str, QString sk, QString ek) {
	return GetBetween(str, sk, ek).toLongLong();
}

//QString ReplyReader::GetString(QString line, QString key) {
//	line += ",";
//	QString k = "\"" + key + "\": ";
//	int pHead = line.indexOf(k, 0, Qt::CaseSensitive);
//	if (pHead == -1) {
//		qDebug() << "*Head";
//
//		return "";
//	}
//	pHead += k.length();
//	int pTail = line.indexOf(",", pHead, Qt::CaseSensitive);
//	if (pTail == -1) {
//		qDebug() << "*Tail";
//
//		return "";
//	}
//
//	QString str = line.mid(pHead, pTail - pHead);
//
//	if (str.contains("\\u")) {
//		str = ConvertUnicodeString(str);
//	}
//
//	if (str.startsWith("\"")) {
//		str = str.right(str.length() - 1);
//	}
//	if (str.endsWith("\"")) {
//		str = str.left(str.length() - 1);
//	}
//
//	return str;
//}
//
//long long ReplyReader::GetValue(QString line, QString key) {
//
//	QString value = GetString(line, key);
//	return value.toLongLong();
//}