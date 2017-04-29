#include "speccursor.h"

SpecCursor::SpecCursor()
{
    cursorEnabled = false;
}

QCPItemLine *SpecCursor::getVLine1() const
{
    return vLine1;
}

void SpecCursor::setVLine1(QCPItemLine *value)
{
    vLine1 = value;
}

QCPItemLine *SpecCursor::getVLine2() const
{
    return vLine2;
}

void SpecCursor::setVLine2(QCPItemLine *value)
{
    vLine2 = value;
}

bool SpecCursor::getCursorEnabled() const
{
    return cursorEnabled;
}

void SpecCursor::setCursorEnabled(bool value)
{
    cursorEnabled = value;
}
