#include "Containers/StringsReader.h"

// --===-- Getters --===--

QVector<QString> StringsReader::strings() const {
    return m_strings;
}

QString StringsReader::formatted() const {
    return m_formatted;
}

StringsReader::SuccessState StringsReader::state() const {
    return static_cast<SuccessState>(m_state);
}

QString StringsReader::error() const {
    return m_error;
}
