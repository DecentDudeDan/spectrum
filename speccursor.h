#ifndef SPECCURSOR_H
#define SPECCURSOR_H

#include <qcustomplot.h>

class SpecCursor
{

public:
    SpecCursor();

    QCPItemLine *getVLine1() const;
    void setVLine1(QCPItemLine *value);

    QCPItemLine *getVLine2() const;
    void setVLine2(QCPItemLine *value);

    bool getCursorEnabled() const;
    void setCursorEnabled(bool value);

private:
    QCPItemLine *vLine1;
    QCPItemLine *vLine2;

    bool cursorEnabled;
};

#endif // SPECCURSOR_H
