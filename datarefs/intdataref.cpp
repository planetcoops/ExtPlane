#include "intdataref.h"
#include "../util/console.h"

IntDataRef::IntDataRef(QObject *parent, QString name, XPLMDataRef ref) : DataRef(parent, name, ref), _value(-999999)
{
    _typeString = "i";
    _type = xplmType_Int;
}

int IntDataRef::value() {
    return _value;
}

bool IntDataRef::updateValue() {
   int newValue = XPLMGetDatai(_ref);
    if(_value != newValue) {
        _value = newValue;
        emit changed(this);
        return true;
    }
    return false;
}

void IntDataRef::setValue(int newValue) {
    if(!isWritable()) {
        INFO << "Tried to write read-only dataref" << name();
        return;
    }
    DEBUG << name() << newValue;
    _value = newValue;
    XPLMSetDatai(ref(), _value);
    emit changed(this);
}

QString IntDataRef::valueString() {
    return QString::number(value());
}

void IntDataRef::setValue(QString &newValue) {
    bool ok = false;
    int value = newValue.toInt(&ok);
    if(ok) {
        setValue(value);
    } else {
        INFO << "Cannot set value " << newValue;
    }
}
