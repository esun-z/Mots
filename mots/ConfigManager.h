#pragma once

#include "global.h"

class ConfigManager : public QObject
{
	Q_OBJECT

public:
	ConfigManager(MotsConfig* c, QObject *p);
	~ConfigManager();

	MotsConfig* config = nullptr;
	QObject* parent = nullptr;
	QFile* file = nullptr;

	
	bool ValidConfig(MotsConfig* c);

	QString lastError;

	bool WriteConfig();
	bool ReadConfig();

signals:
	void Error();
	void CreateConfigFile();
};
