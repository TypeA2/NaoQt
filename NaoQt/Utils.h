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
    QString ucFirst(const QString& str);

    // Cleanup directory path
    QString cleanDirPath(QString str);

    // Cleanup file path
    QString cleanFilePath(QString str);

    // Cleanup generic path
    QString cleanGenericPath(QString str);

    // Round up a to a multiple of b
    template <typename T, typename M>
    typename std::enable_if<std::is_integral<T>::value, T>::type roundUp(T a, M b) {
        if (b == 0) {
            return a;
        }

        T remainder = a % b;
        if (remainder == 0) {
            return a;
        }

        return a + b - remainder;
    }
}

namespace Steam {
    // Returns the Steam install folder as read from the Windows registry
    QString getSteamPath();

    // Returns a list containing all install folders, including the default
    QStringList getSteamInstallFolders();

    // Returns the path of the game folder
    QString getGamePath(const QString& game, const QString& def = QString());
}