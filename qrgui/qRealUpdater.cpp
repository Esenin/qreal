#include "qRealUpdater.h"

using namespace qReal;

QRealUpdater::QRealUpdater(char *applicationPath)
	: QObject()
	, mHasNewUpdates(false)
	, mUpdaterProcess(NULL)
	, mQRealPath(applicationPath)

{
	mUpdaterPath = QFileInfo(mQRealPath).absolutePath() + "/qrUpdater/";
}

bool QRealUpdater::hasUpdates()
{
	return mHasNewUpdates;
}

void QRealUpdater::startUpdater()
{
	if (!hasUpdatePermission()) {
		mHasNewUpdates = false;
		return;
	}
	executeUpdater();
	transferInfo();
}

void QRealUpdater::executeUpdater()
{
	QString const programPath = mUpdaterPath + "qrUpdater";
	QStringList arguments;
	arguments << "-unit" << "qrealBP-windows"
			<< "-version" << SettingsManager::value("version").toString()
			<< "-url" << "https://dl.dropboxusercontent.com/s/ukc0tn6fiueg7u9/updates.xml?token_hash=AAFS8VRpQl0WKfITMZA5k4bMI9k_1MXDZJxgEQ9EcOMDxA&dl=1";

	mUpdaterProcess = new QProcess();
	mUpdaterProcess->setWorkingDirectory(mUpdaterPath);

	mUpdaterProcess->start(programPath, arguments);
}

void QRealUpdater::transferInfo()
{
	mUpdaterProcess->write(QString(mQRealPath + "\n").toUtf8());
	mUpdaterProcess->waitForReadyRead(updaterTimeout);
	readAnswer();
}

bool QRealUpdater::hasUpdatePermission()
{
	SettingsManager::instance()->load();
	return !SettingsManager::value("version", "").toString().isEmpty()
			&& SettingsManager::value("updaterActive", true).toBool();
}

void QRealUpdater::prepareForClose()
{
	SettingsManager::setValue("version", "");  // 'couz I cant just get version from inside
	SettingsManager::instance()->saveData();
}

void QRealUpdater::readAnswer()
{
	QString input(mUpdaterProcess->readAll());
	mHasNewUpdates = input.contains("Terminate!");
	if (mHasNewUpdates) {
		prepareForClose();
	}
}
