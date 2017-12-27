#ifndef CMDCUSTOMDATA_H
#define CMDCUSTOMDATA_H

#include <QObject>
#include "XPLMUtilities.h"

/**
  *
  */
class CmdCustomData : public QObject
{
    Q_OBJECT
public:
    static XPLMCommandRef commandRef;
    static int IntCallback(void *inRefcon);
    static int CmdHandler(XPLMCommandRef cmd, XPLMCommandPhase phase, void *refcon);

signals:

public slots:

private:
    static int _value;
};

#endif // CMDCUSTOMDATA_H
