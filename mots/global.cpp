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