#pragma once

#include <stdexcept>

class CPKException : public std::logic_error {
    public:
    CPKException(const char* what) : std::logic_error(what) {}
};

class UTFReader;
class QIODevice;
class CPKReader {
    public:
    CPKReader(QIODevice* input);

    private:

    void init();

    QIODevice* m_input;

    UTFReader* m_cpkReader;
};

