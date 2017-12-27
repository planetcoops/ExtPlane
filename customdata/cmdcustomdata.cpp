#include "cmdcustomdata.h"

XPLMCommandRef CmdCustomData::commandRef = NULL;
int CmdCustomData::_value = 0;

int CmdCustomData::IntCallback(void *inRefcon) {
    Q_UNUSED(inRefcon);
    return _value;
}

int CmdCustomData::CmdHandler(XPLMCommandRef cmd, XPLMCommandPhase phase, void *refcon) {
    Q_UNUSED(cmd);
    Q_UNUSED(refcon);
    if (phase == xplm_CommandBegin) {
        _value = 1 - _value;
    }
    return (1);
}