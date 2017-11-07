#ifndef IntArrayDataRef_H
#define IntArrayDataRef_H

#include "dataref.h"
#include <QVector>

class IntArrayDataRef : public DataRef {
    Q_OBJECT

public:
    IntArrayDataRef(QObject *parent, QString name, XPLMDataRef ref);
    ~IntArrayDataRef();
    QVector<int> & value();
    virtual bool updateValue();
    virtual QString valueString();
    virtual void setValue(QString &newValue);
private:
    QVector<int> _values; // Actual values in a vector
    int _length; // Length of dataref, as given by X-Plane
    int * _valueArray; // Array for reading & writing values. Allocated dynamically.
};

#endif // IntArrayDataRef_H
