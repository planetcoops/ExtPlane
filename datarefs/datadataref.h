#ifndef DATADATAREF_H
#define DATADATAREF_H

#include "dataref.h"

#include <QByteArray>
#include <QTime>

/**
  * Represents a Binary DataRef (type b).
  * The accuracy for binary datarefs represents how often ExtPlane
  * should check for updates in milliseconds. When the accuracy
  * is set to 0, ExtPlane will check for updates as soon as possible.
  */
class DataDataRef : public DataRef {
    Q_OBJECT

public:
    DataDataRef(QObject *parent, QString name, XPLMDataRef ref);
    QByteArray &value();
    void setValue(QByteArray &newValue);
    virtual bool updateValue();
    virtual QString valueString();
    virtual void setValue(QString &newValue);

private:
    int _length; // Length of dataref, as given by X-Plane
    QByteArray _value; // Value of dataref is stored here once retrieved from XPLM
    QByteArray _newValue; // Temp variable used while checking changes. Here to avoid resizing.
    QTime _lastUpdate; // Timer used for tracking the last update time
};

#endif // DATADATAREF_H
