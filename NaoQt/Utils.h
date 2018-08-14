#pragma once

class QString;

namespace Utils {
	// Readable filesizes
	QString getShortSize(quint64 size, bool bits = false);

	// Readable time
	QString getShortTime(double time);

	// Sanitize filenames
	QString sanitizeFileName(QString& fname);

	// Uppercase first letter
	QString ucFirst(QString& str);

	// Cleanup directory path
	QString cleanDirPath(QString str);
}

namespace Steam {
	// Returns the Steam install folder as read from the Windows registry
	QString getSteamPath();

	// Returns a list containing all install folders, including the default
	QStringList getSteamInstallFolders();

	// Returns the path of the game folder
	QString getGamePath(QString game, QString def = QString());
}