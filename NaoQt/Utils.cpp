#pragma once

#include "vdf_parser.hpp"

#include <regex>

#include <QString>
#include <QSettings>
#include <QDir>

namespace Utils {
	QString getShortSize(quint64 size, bool bits) {

		// bitshift by n * 10 to divide by 1024^n, then divide by 1024. to get a float value with 3 decimals.

		if (size > 0x1000000000000000) {
			return QString("%0 Ei%1").arg((size >> 50) / 1024., 0, 'f', 3)
				.arg(bits ? "bit" : "B");
		}
		else if (size > 0x4000000000000) {
			return QString("%0 Pi%1").arg((size >> 40) / 1024., 0, 'f', 3)
				.arg(bits ? "bit" : "B");
		}
		else if (size > 0x10000000000) {
			return QString("%0 Ti%1").arg((size >> 30) / 1024., 0, 'f', 3)
				.arg(bits ? "bit" : "B");
		}
		else if (size > 0x40000000) {
			return QString("%0 Gi%1").arg((size >> 20) / 1024., 0, 'f', 3)
				.arg(bits ? "bit" : "B");
		}
		else if (size > 0x100000) {
			return QString("%0 Mi%1").arg((size >> 10) / 1024., 0, 'f', 3)
				.arg(bits ? "bit" : "B");
		}
		else if (size > 0x400) {
			return QString("%0 Ki%1").arg(size / 1024., 0, 'f', 3)
				.arg(bits ? "bit" : "B");
		}
		else {
			return QString("%0 %1").arg(size)
				.arg(bits ? "bit" : "B");
		}
	}

	QString getShortTime(double time) {
		qint64 hours = time / 3600;
		time -= hours * 3600;

		qint64 minutes = time / 60;
		time -= minutes * 60;

		qint64 seconds = time;
		time -= seconds;

		qint64 msecs = time * 1000.;

		QString output = QString("%0:%1:%3.%4").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')).arg(msecs, 3, 10, QChar('0'));

		if (hours == 0)
			output.remove(0, 3);

		if (minutes == 0)
			output.remove(0, 3);

		return output;
	}

	QString sanitizeFileName(QString& fname) {
		// because Platinum Games thinks : is valid for filenames

		const QString illegalChars = R"(\\/:?"<>|)";

		for (QString::iterator it = fname.begin();
			it != fname.end(); ++it) {

			if (illegalChars.contains(*it))
				*it = '_';
		}

		return fname;
	}

	QString ucFirst(QString& str) {
		return QString(str.at(0).toUpper()) + str.mid(1);
		
	}

	QString cleanDirPath(QString str) {
		str = QDir::toNativeSeparators(str);

		if (!str.endsWith(QDir::separator())) {
			str = str + QDir::separator();
		}

		if (str.at(0) == QDir::separator()) {
			str = QDir::rootPath() + str.mid(1);
		}

		if (str.at(0).isLetter() && str.at(0).isLower()) {
			str = Utils::ucFirst(str);
		}

		return str;
	}

	QString cleanFilePath(QString str) {
		str = QDir::toNativeSeparators(str);

		if (str.at(0) == QDir::separator()) {
			str = QDir::rootPath() + str.mid(1);
		}

		if (str.at(0).isLetter() && str.at(0).isLower()) {
			str = Utils::ucFirst(str);
		}

		return str;
	}
}

namespace Steam {
	QString getSteamPath() {

		// Just read directly from the Windows registry

		return QSettings("HKEY_CURRENT_USER\\Software\\Valve\\Steam",
			QSettings::NativeFormat).value("SteamPath").toString();
	}

	QStringList getSteamInstallFolders() {
		// <steampath>/SteamApps/libraryfolders.vdf contains all our install folders

		std::ifstream libfolders((getSteamPath() + "/SteamApps/libraryfolders.vdf").toUtf8().constData());
		tyti::vdf::object root = tyti::vdf::read(libfolders);

		QStringList retlist = QStringList(getSteamPath());

		for (size_t i = 1; i <= root.attribs.size() - 2; i++) {

			// replace some slashes to make it readable

			retlist.append(
				std::regex_replace(
					root.attribs.at(std::to_string(i)),
					std::regex(R"d(\\\\)d"),
					"/"
				).c_str()
			);
		}

		return retlist;
	}

	QString getGamePath(QString game, QString def) {

		// Search all install folders for a given game, return def if not found

		for (QString folder : getSteamInstallFolders()) {
			QStringList entries = QDir(folder + "/SteamApps/common").entryList(QStringList(game), QDir::Dirs);

			if (entries.size() > 0 && QDir(folder + "/SteamApps/common/" + entries.at(0)).count()) {
				return Utils::cleanDirPath(folder + "/SteamApps/common/" + entries.at(0));
			}
		}

		return Utils::cleanDirPath(def);
	}
}
