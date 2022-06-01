#include "ConfigManager.h"

ConfigManager::ConfigManager(MotsConfig* c, QObject *p)
	: QObject(p)
{
	config = c;
	parent = p;
	
	file = new QFile(QCoreApplication::applicationDirPath() + "/mots.cfg", this);
	
	if (!file->exists()) {
		if (!WriteConfig()) {
			emit Error();
		}
		else {
			emit CreateConfigFile();
		}
	}
	else {
		if (!ReadConfig()) {
			config = new MotsConfig();
			emit Error();
		}
	}
}

ConfigManager::~ConfigManager()
{
}

bool ConfigManager::WriteConfig() {
	if (!file->open(QIODevice::WriteOnly)) {
		lastError = "Failed to open config file.";
		return false;
	}

	QDataStream s(file);
	s << CURRENTCONFIGVERSION;
	s << config->gamePath
		<< config->getIdUrl
		<< config->getShipNameUrl
		<< config->getOverviewUrl
		<< config->getShipUrl;
	s << config->customApplicationId
		<< config->applicationId;
	s << config->localInterval << config->networkInterval;

	file->close();

	return true;
}

bool ConfigManager::ReadConfig() {
	if (!file->open(QIODevice::ReadOnly)) {
		lastError = "Failed to open config file.";
		return false;
	}

	QDataStream s(file);
	int ver = -1;
	s >> ver;
	switch (ver)
	{
	case 0:

		s >> config->gamePath
			>> config->getIdUrl
			>> config->getShipNameUrl
			>> config->getOverviewUrl
			>> config->getShipUrl;
		s >> config->customApplicationId
			>> config->applicationId;
		s >> config->localInterval >> config->networkInterval;

		break;

	default:
		return false;
	}

	file->close();

	if (ValidConfig(config)) {
		return true;
	}
	else {
		lastError = "Invalid config.";
		return false;
	}
}

bool ConfigManager::ValidConfig(MotsConfig* c) {
	bool valid = true;
	if (c->gamePath.isEmpty() ||
		!c->getIdUrl.contains(PLACEHOLDER_PLAYERNAME) ||
		!c->getOverviewUrl.contains(PLACEHOLDER_PLAYERID) ||
		!c->getShipNameUrl.contains(PLACEHOLDER_APPLICATIONID) ||
		!c->getShipNameUrl.contains(PLACEHOLDER_SHIPID) ||
		!c->getShipUrl.contains(PLACEHOLDER_PLAYERID)) {
		valid = false;
		/*qDebug() << c->gamePath.isEmpty()
			<< !c->getIdUrl.contains(PLACEHOLDER_PLAYERNAME)
			<< !c->getOverviewUrl.contains(PLACEHOLDER_PLAYERID)
			<< !c->getShipNameUrl.contains(PLACEHOLDER_APPLICATIONID)
			<< !c->getShipNameUrl.contains(PLACEHOLDER_SHIPID)
			<< !c->getShipUrl.contains(PLACEHOLDER_PLAYERID);*/
	}
	if (c->customApplicationId) {
		if (c->applicationId.isEmpty()) {
			valid = false;
		}
		qDebug() << 2;
	}
	if (c->networkInterval <= 0 || c->localInterval <= 0) {
		valid = false;
		qDebug() << 3;
	}
	
	return valid;
}
