#pragma once

#include <QVector>

class QIODevice;

class StringsReader {
    public:

    // -- Enums --
    enum SuccessState : quint64 {
        SUCCESS,
        FAIL,
        CUSTOM
    };

    // -- Destructor --
    virtual ~StringsReader() = default;

    // -- Getters --
    virtual QVector<QString> strings() const;
    virtual QString formatted() const;
    virtual SuccessState state() const;
    virtual QString error() const;

    // -- Reader --
    virtual SuccessState read() = 0;

    protected:
    // -- Constructor --
    StringsReader() = default;

    // -- Member variables --
    quint64 m_state;
    QString m_error;

    QIODevice* m_input;

    QVector<QString> m_strings;
    QString m_formatted;
};
