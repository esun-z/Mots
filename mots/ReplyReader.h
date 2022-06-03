#pragma once

#include "global.h"

class ReplyReader
{
	
	/*QString GetString(QString line, QString key);
	long long GetValue(QString line, QString key);*/
public:
	static QString GetBetween(QString content, QString startKey, QString endKey);
	static QString GetRight(QString content, QString startKey);
	static double GetNumBetween(QString content, QString startKey, QString endKey);
	static long long GetllNumBetween(QString content, QString startKey, QString endKey);

};

