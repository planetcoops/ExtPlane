#include "xplaneplugin.h"
#include "datarefs/dataref.h"
#include "datarefs/floatdataref.h"
#include "datarefs/floatarraydataref.h"
#include "datarefs/intdataref.h"
#include "datarefs/intarraydataref.h"
#include "datarefs/doubledataref.h"
#include "datarefs/datadataref.h"
#include "util/console.h"
#include "customdata/navcustomdata.h"
#include "customdata/atccustomdata.h"
#include "customdata/cmdcustomdata.h"
#include <clocale>
#include <XPLMUtilities.h>

#if defined(_MSC_VER) && _MSC_VER < 1900

#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf

__inline int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

__inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(outBuf, size, format, ap);
    va_end(ap);

    return count;
}

#endif

static char __log_printf_buffer[4096];
#define log_printf(fmt, ...) snprintf(__log_printf_buffer, 4096, "ExtPlane-Plugin: " fmt, __VA_ARGS__), XPLMDebugString(__log_printf_buffer)

XPlanePlugin::XPlanePlugin(QObject *parent) :
    QObject(parent), argc(0), argv(0), app(0), server(0), flightLoopInterval(0.31f) { // Default to 30hz
}

XPlanePlugin::~XPlanePlugin() {
    DEBUG << Q_FUNC_INFO;
}

float XPlanePlugin::flightLoop(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop,
                               int inCounter, void *inRefcon) {
    Q_UNUSED(inElapsedSinceLastCall);
    Q_UNUSED(inElapsedTimeSinceLastFlightLoop);
    Q_UNUSED(inCounter);
    Q_UNUSED(inRefcon);
    // Tell each dataref to update its value through the XPLM api
    foreach(DataRef *ref, refs) ref->updateValue();
    // Tell Qt to process it's own runloop
    app->processEvents();
    return flightLoopInterval;
}

int XPlanePlugin::pluginStart(char * outName, char * outSig, char *outDesc) {
    // Set plugin info
    INFO << "Plugin started";
    strcpy(outName, "ExtPlane");
    strcpy(outSig, "com.planetcoops.extplaneplugin");
    strcpy(outDesc, "Read and write X-Plane datarefs from external programs using TCP sockets.");

    // Init application and server
    app = new QCoreApplication(argc, &argv);
    setlocale(LC_NUMERIC, "C"); // See http://stackoverflow.com/questions/25661295/why-does-qcoreapplication-call-setlocalelc-all-by-default-on-unix-linux

    server = new TcpServer(this, this);
    connect(server, SIGNAL(setFlightLoopInterval(float)), this, SLOT(setFlightLoopInterval(float)));

    // Log version information
    log_printf("Listening on TCP port %d using protocol %s compiled %s %s\n", EXTPLANE_PORT, EXTPLANE_PROTOCOL, __DATE__, __TIME__);

    // Register the nav custom data accessors
    XPLMRegisterDataAccessor("extplane/navdata/5km",
                                                 xplmType_Data,                                 // The types we support
                                                 0,                                             // Writable
                                                 NULL, NULL,                                    // Integer accessors
                                                 NULL, NULL,                                    // Float accessors
                                                 NULL, NULL,                                    // Doubles accessors
                                                 NULL, NULL,                                    // Int array accessors
                                                 NULL, NULL,                                    // Float array accessors
                                                 NavCustomData::DataCallback_5km, NULL,         // Raw data accessors
                                                 NULL, NULL);                                   // Refcons not used
    XPLMRegisterDataAccessor("extplane/navdata/20km",
                                                 xplmType_Data,                                 // The types we support
                                                 0,                                             // Writable
                                                 NULL, NULL,                                    // Integer accessors
                                                 NULL, NULL,                                    // Float accessors
                                                 NULL, NULL,                                    // Doubles accessors
                                                 NULL, NULL,                                    // Int array accessors
                                                 NULL, NULL,                                    // Float array accessors
                                                 NavCustomData::DataCallback_20km, NULL,        // Raw data accessors
                                                 NULL, NULL);                                   // Refcons not used
    XPLMRegisterDataAccessor("extplane/navdata/100km",
                                                 xplmType_Data,                                 // The types we support
                                                 0,                                             // Writable
                                                 NULL, NULL,                                    // Integer accessors
                                                 NULL, NULL,                                    // Float accessors
                                                 NULL, NULL,                                    // Doubles accessors
                                                 NULL, NULL,                                    // Int array accessors
                                                 NULL, NULL,                                    // Float array accessors
                                                 NavCustomData::DataCallback_100km, NULL,       // Raw data accessors
                                                 NULL, NULL);                                   // Refcons not used
    XPLMRegisterDataAccessor("extplane/atc/124thatc/latest",
                                                 xplmType_Data,                                 // The types we support
                                                 0,                                             // Writable
                                                 NULL, NULL,                                    // Integer accessors
                                                 NULL, NULL,                                    // Float accessors
                                                 NULL, NULL,                                    // Doubles accessors
                                                 NULL, NULL,                                    // Int array accessors
                                                 NULL, NULL,                                    // Float array accessors
                                                 ATCCustomData::DataCallback, NULL,             // Raw data accessors
                                                 NULL, NULL);
    XPLMRegisterDataAccessor("extplane/command_state",
                                                 xplmType_Int,                                  // The types we support
                                                 0,                                             // Writable
                                                 CmdCustomData::IntCallback, NULL,              // Integer accessors
                                                 NULL, NULL,                                    // Float accessors
                                                 NULL, NULL,                                    // Doubles accessors
                                                 NULL, NULL,                                    // Int array accessors
                                                 NULL, NULL,                                    // Float array accessors
                                                 NULL, NULL,                                    // Raw data accessors
                                                 NULL, NULL);
    // Register custom command
    CmdCustomData::commandRef = XPLMCreateCommand("ExtPlane/command", "ExtPlane custom command, e.g. Push to Talk.");
    XPLMRegisterCommandHandler(CmdCustomData::commandRef, CmdCustomData::CmdHandler, 0, NULL);

    app->processEvents();
    return 1;
}

DataRef* XPlanePlugin::subscribeRef(QString name) {
    DEBUG << name;

    // Search in list of already subscribed datarefs - if found return that
    foreach(DataRef *ref, refs) {
        if(ref->name()==name) {
            DEBUG << "Already subscribed to " << name;
            ref->setSubscribers(ref->subscribers() + 1);
            return ref;
        }
    }

    // Not yet subscribed - create a new dataref
    XPLMDataRef ref = XPLMFindDataRef(name.toLatin1());
    if(ref) {
        XPLMDataTypeID refType = XPLMGetDataRefTypes(ref);
        DataRef *dr = 0;
        if(refType & xplmType_Double) {
            dr = new DoubleDataRef(this, name, ref);
        } else if(refType & xplmType_Float) {
            dr = new FloatDataRef(this, name, ref);
        } else if(refType & xplmType_Int) {
            dr = new IntDataRef(this, name, ref);
        } else if (refType & xplmType_FloatArray) {
            dr = new FloatArrayDataRef(this, name, ref);
        } else if (refType & xplmType_IntArray) {
            dr = new IntArrayDataRef(this, name, ref);
        } else if (refType & xplmType_Data) {
            dr = new DataDataRef(this, name, ref);
        }
        if(dr) {
            dr->setSubscribers(1);
            dr->setWritable(XPLMCanWriteDataRef(ref) != 0);
            DEBUG << "Subscribed to ref " << dr->name() << ", type: " << dr->typeString() << ", writable:" << dr->isWritable();
            refs.append(dr);
            return dr;
        } else {
            INFO << "Dataref type " << refType << "not supported";
        }
    } else {
        INFO << "Can't find dataref " << name;
    }
    return 0;
}

void XPlanePlugin::unsubscribeRef(DataRef *ref) {
    Q_ASSERT(refs.contains(ref));
    DEBUG << ref->name() << ref->subscribers();
    ref->setSubscribers(ref->subscribers() - 1);
    if(ref->subscribers() == 0) {
        refs.removeOne(ref);
        DEBUG << "Ref " << ref->name() << " not subscribed by anyone - removing.";
        ref->deleteLater();
    }
}

void XPlanePlugin::keyStroke(int keyid) {
    DEBUG << keyid;
    XPLMCommandKeyStroke(keyid);
}

void XPlanePlugin::buttonPress(int buttonid) {
    DEBUG << buttonid;
    XPLMCommandButtonPress(buttonid);
}

void XPlanePlugin::buttonRelease(int buttonid) {
    DEBUG << buttonid;
    XPLMCommandButtonRelease(buttonid);
}

void XPlanePlugin::setFlightLoopInterval(float newInterval) {
    if(newInterval > 0) {
        flightLoopInterval = newInterval;
        DEBUG << "New interval" << flightLoopInterval;
    } else {
        DEBUG << "Invalid interval " << newInterval;
    }
}

void XPlanePlugin::pluginStop() {
    // Unregister custom command
    XPLMUnregisterCommandHandler(CmdCustomData::commandRef, CmdCustomData::CmdHandler, 0, NULL);
    DEBUG;
    app->processEvents();
    delete server;
    server = 0;
    app->quit();
    app->processEvents();
    delete app;
    app = 0;
    qDeleteAll(refs);
    refs.clear();
}

void XPlanePlugin::receiveMessage(XPLMPluginID inFromWho, long inMessage, void *inParam) {
    Q_UNUSED(inParam);

    DEBUG << inFromWho << inMessage;
}
