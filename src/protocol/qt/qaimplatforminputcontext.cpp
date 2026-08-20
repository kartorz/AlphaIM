
/*
 * Debug:
 * export QT_DEBUG_PLUGINS=1
 * export QT_IM_MODULE=alphaim
 * featherpad, kontact
 */

#include "qaimplatforminputcontext.h"

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QTextStream>
#include <QtCore/QtDebug>
#include <QtGui/QTextCharFormat>
#include <QGuiApplication>
#include <QDBusVariant>
#include <qwindow.h>
#include <qevent.h>

#include <qpa/qplatformcursor.h>
#include <qpa/qplatformscreen.h>
#include <qpa/qwindowsysteminterface.h>

#include "qaimproxy.h"
#include "qaiminputcontextproxy.h"
#include "../../aim.h"

#include <sys/types.h>
#include <signal.h>
#include <syslog.h>

#include <QtDBus>

QT_BEGIN_NAMESPACE

#if AIM_DEBUG
#define QDBG(expr)  qDebug() << expr
#else
#define QDBG(expr) ((void)0)
#endif

/*
 * One IC per win, commit str to focus obj.
 * ---------------------------------------
 * g_im_active:
 *   0: don't forward input events.
 *   1: forward all events.
 */
static int g_im_active = 0;

class QAimPlatformInputContextPrivate
{
public:
    QAimPlatformInputContextPrivate();
    ~QAimPlatformInputContextPrivate()
    {
        destroyBusProxy();
        if (connection) {
            delete connection;
            connection = nullptr;
        }
        QDBG("~QAimPlatformInputContextPrivate\n");
    }

    bool createBusProxy();
    void destroyBusProxy();

    QDBusConnection *connection;
    QAimProxy *bus;
    QAimInputContextProxy *context;

    bool valid;
    bool busConnected;
    bool needsSurroundingText;
    QLocale locale;
    uint icid;
};

/** A IC per window, commit to focus input.
  * The IC should belong to the main GUI thread.
  */
QAimPlatformInputContext::QAimPlatformInputContext ()
    : d(new QAimPlatformInputContextPrivate())
{
    if (d->connection == NULL) {
        syslog(LOG_ERR, "%s: Error can't connect to session bus\n", IM_NAME);
        return;
    }

    QDBusServiceWatcher* watherReg = new QDBusServiceWatcher(QLatin1String(AIM_SRV_NAME), *(d->connection), QDBusServiceWatcher::WatchForRegistration, this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    connect(watherReg, &QDBusServiceWatcher::serviceRegistered,
            this, &QAimPlatformInputContext::serviceRegistered);
#else
    connect(watherReg, SIGNAL(serviceRegistered(QString)), this, SLOT(serviceRegistered(QString)))
#endif
    QDBusServiceWatcher* watherUnreg = new QDBusServiceWatcher(QLatin1String(AIM_SRV_NAME), *(d->connection), QDBusServiceWatcher::WatchForUnregistration, this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    connect(watherUnreg, &QDBusServiceWatcher::serviceUnregistered,
            this, &QAimPlatformInputContext::serviceUnregistered);
#else
    connect(watherUnreg, SIGNAL(serviceUnregistered(QString)), this, SLOT(serviceUnregistered(QString)));
#endif
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(connectToBus()));
    m_timer.start(100);

    QInputMethod *p = qApp->inputMethod();
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    connect(p, &QInputMethod::cursorRectangleChanged,
            this,&QAimPlatformInputContext::cursorRectChanged);
#else
    connect(p, SIGNAL(cursorRectangleChanged()), this, SLOT(cursorRectChanged()));
#endif

    m_eventFilterUseSynchronousMode = false;
    if (qEnvironmentVariableIsSet("AIM_ENABLE_SYNC_MODE")) {
        bool ok;
        int enableSync = qEnvironmentVariableIntValue("AIM_ENABLE_SYNC_MODE", &ok);
        if (ok && enableSync == 1)
            m_eventFilterUseSynchronousMode = true;
    }
}

QAimPlatformInputContext::~QAimPlatformInputContext (void)
{
    if (d->busConnected && d->context)
        d->context->Destroy(d->icid);
    QDBG("~QAimPlatformInputContext()\n");
    delete d;
}

bool QAimPlatformInputContext::isTriggerKey(const QKeyEvent *event)
{
    if (event->key() != Qt::Key_Space)
        return false;

    Qt::KeyboardModifiers mods = event->modifiers();

    return (mods == Qt::ShiftModifier) ||
           (mods == Qt::ControlModifier) ||
           (mods == Qt::MetaModifier);
}

// false: ~QAimPlatformInputContext()
bool QAimPlatformInputContext::isValid() const
{
    return d->valid && d->connection != NULL;
}

bool QAimPlatformInputContext::hasCapability(Capability capability) const
{
    switch (capability) {
    case QPlatformInputContext::HiddenTextCapability:
        return false; // QTBUG-40691, do not show IME on desktop for password entry fields.
    default:
        break;
    }
    return true;
}

void QAimPlatformInputContext::invokeAction(QInputMethod::Action a, int)
{
    if (!d->busConnected)
        return;
    QDBG("invokeAction()\n");
}

void QAimPlatformInputContext::commit()
{
    if (!d->busConnected)
        return;
    QDBG("commit()\n");
}

void QAimPlatformInputContext::reset()
{
    QPlatformInputContext::reset();

    if (!d->busConnected | !g_im_active)
        return;

    d->context->Reset(d->icid);
    QDBG("reset\n");
}


void QAimPlatformInputContext::update(Qt::InputMethodQueries q)
{
    QObject *input = qApp->focusObject();

    if (d->needsSurroundingText && input
            && (q.testFlag(Qt::ImSurroundingText)
                || q.testFlag(Qt::ImCursorPosition)
                || q.testFlag(Qt::ImAnchorPosition))) {
        QInputMethodQueryEvent srrndTextQuery(Qt::ImSurroundingText);
        QInputMethodQueryEvent cursorPosQuery(Qt::ImCursorPosition);
        QInputMethodQueryEvent anchorPosQuery(Qt::ImAnchorPosition);

        QCoreApplication::sendEvent(input, &srrndTextQuery);
        QCoreApplication::sendEvent(input, &cursorPosQuery);
        QCoreApplication::sendEvent(input, &anchorPosQuery);

        QString surroundingText = srrndTextQuery.value(Qt::ImSurroundingText).toString();
        uint cursorPosition = cursorPosQuery.value(Qt::ImCursorPosition).toUInt();
        uint anchorPosition = anchorPosQuery.value(Qt::ImAnchorPosition).toUInt();
    }
    QPlatformInputContext::update(q);
}

/* The first message when click or open a win.
 *
 */
void QAimPlatformInputContext::cursorRectChanged()
{
    if (!d->busConnected || !g_im_active)
        return;

    QRect r = qApp->inputMethod()->cursorRectangle().toRect();
    if(!r.isValid())
        return;
    QWindow *inputWindow = qApp->focusWindow();
    if (!inputWindow)
        return;
    r.moveTopLeft(inputWindow->mapToGlobal(r.topLeft()));
    d->context->SetCursorLocation(d->icid, r.x(), r.y(), r.width(), r.height());
    //QDBG("cursorRectChanged" << r << "ic:" << d->icid);
}

void QAimPlatformInputContext::setFocusObject(QObject *object)
{
    if (!d->busConnected)
        return;

    QDBG("setFocusObject, ic:" << d->icid << "," << object);
    if (object) {
        QDBusReply<int> reply = d->context->FocusIn(d->icid);
        if (reply.isValid())
            g_im_active = reply.value();
        return;
    }

    if (g_im_active) {
        d->context->FocusOut(d->icid);
        return;
    }
}

void QAimPlatformInputContext::commitText(const QString &text)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    QObject *input = qApp->focusObject();
    if (!input || text.isEmpty())
        return;

    /*[send/post]Event at a cross thread will crash.
      commitText() is guaranteed to run in the GUI thread.
    */
    QInputMethodEvent event;
    event.setCommitString(text);
    QCoreApplication::sendEvent(input, &event);
    QDBG("commitText" << text);
}

bool QAimPlatformInputContext::filterEvent(const QEvent *event)
{
    const QKeyEvent *keyEvent = static_cast<const QKeyEvent *>(event);
    quint32 sym = keyEvent->nativeVirtualKey();
    quint32 code = keyEvent->nativeScanCode();
    quint32 state = keyEvent->nativeModifiers();
    quint32 aimState = state;
    bool    triggerKey = false;

    if (keyEvent->type() == QEvent::KeyRelease) {
        aimState |= AIM_RELEASE_MASK;
        triggerKey = isTriggerKey(keyEvent);
    }
    //QDBG("filterEvent, sym:" << sym <<"code:" << code <<"state:" <<aimState);
    if (!d->busConnected || !inputMethodAccepted() || (!triggerKey && !g_im_active))
        return false;

    QDBusPendingReply<int> reply = d->context->ProcessKeyEvent(d->icid, sym, code - 8, aimState);
    if (m_eventFilterUseSynchronousMode || reply.isFinished()) {
        int retval = reply.value();
        if (!retval) return false;
        g_im_active = retval - 1;
        return true;
    }

    Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
    const int qtcode = keyEvent->key();
    // From QKeyEvent::modifiers()
    switch (qtcode) {
    case Qt::Key_Shift:
        modifiers ^= Qt::ShiftModifier;
        break;
    case Qt::Key_Control:
        modifiers ^= Qt::ControlModifier;
        break;
    case Qt::Key_Alt:
        modifiers ^= Qt::AltModifier;
        break;
    case Qt::Key_Meta:
        modifiers ^= Qt::MetaModifier;
        break;
    case Qt::Key_AltGr:
        modifiers ^= Qt::GroupSwitchModifier;
        break;
    }

    QVariantList args;
    args << QVariant::fromValue(keyEvent->timestamp());
    args << QVariant::fromValue(static_cast<uint>(keyEvent->type()));
    args << QVariant::fromValue(qtcode);
    args << QVariant::fromValue(code) << QVariant::fromValue(sym) << QVariant::fromValue(state);
    args << QVariant::fromValue(keyEvent->text());
    args << QVariant::fromValue(keyEvent->isAutoRepeat());

    QAimFilterEventWatcher *watcher = new QAimFilterEventWatcher(reply, this, QGuiApplication::focusWindow(), modifiers, args);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, &QAimPlatformInputContext::filterEventFinished);

    return true;
}

void QAimPlatformInputContext::filterEventFinished(QDBusPendingCallWatcher *call)
{
    QAimFilterEventWatcher *watcher = dynamic_cast<QAimFilterEventWatcher *>(call);
    QDBusPendingReply<int> reply = *call;
    if (reply.isError()) {
        syslog(LOG_ERR, "%s: filterEventFinished, reply error\n", IM_NAME);
        call->deleteLater();
        d->busConnected = false;
        return;
    }

    // Use watcher's window instead of the current focused window
    // since there is a time lag until filterEventFinished() returns.
    QWindow *window = watcher->window();
    if (!window) {
        call->deleteLater();
        return;
    }

    Qt::KeyboardModifiers modifiers = watcher->modifiers();
    QVariantList args = watcher->arguments();
    const ulong time = static_cast<ulong>(args.at(0).toUInt());
    const QEvent::Type type = static_cast<QEvent::Type>(args.at(1).toUInt());
    const int qtcode = args.at(2).toInt();
    const quint32 code = args.at(3).toUInt();
    const quint32 sym = args.at(4).toUInt();
    const quint32 state = args.at(5).toUInt();
    const QString string = args.at(6).toString();
    const bool isAutoRepeat = args.at(7).toBool();
    int retval = reply.value();

    //QDBG("filterEventFinished return" << code << sym << state << retval);
    if (!retval) {
#ifndef QT_NO_CONTEXTMENU
        if (type == QEvent::KeyPress && qtcode == Qt::Key_Menu
            && window != NULL) {
            const QPoint globalPos = window->screen()->handle()->cursor()->pos();
            const QPoint pos = window->mapFromGlobal(globalPos);
#ifndef QT_NO_CONTEXTMENU
            QWindowSystemInterface::handleContextMenuEvent(window, false, pos,
                                                           globalPos, modifiers);
#endif
        }
#endif // QT_NO_CONTEXTMENU
        QWindowSystemInterface::handleExtendedKeyEvent(window, time, type, qtcode, modifiers,
                                                       code, sym, state, string, isAutoRepeat);
    } else {
        g_im_active = retval - 1;
    }
    call->deleteLater();
}

QLocale QAimPlatformInputContext::locale() const
{
    return d->locale;
}

/* alphaimd appears */
void QAimPlatformInputContext::serviceRegistered(const QString &service)
{
    if (d->busConnected) return;
    m_timer.stop();
    m_timer.start(100);

    QDBG("serviceRegistered:" << service);
    syslog(LOG_INFO, "%s: serviceRegistered(%s)\n",IM_NAME, service.toUtf8().constData());
}

/* alphaimd disappears */
void QAimPlatformInputContext::serviceUnregistered(const QString &service)
{
    m_timer.stop();
    d->busConnected = false;
    QDBG("serviceUnRegistered:" << service);
    syslog(LOG_INFO, "%s: serviceUnregistered(%s)\n",IM_NAME, service.toUtf8().constData());
}

void QAimPlatformInputContext::connectToBus()
{
    if (d->busConnected)
        return;

    if (d->createBusProxy()) {
        connectToContextSignals();
        d->context->Enable(d->icid);
        d->busConnected = true;
    }
}

void QAimPlatformInputContext::connectToContextSignals()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    connect(d->context, &QAimInputContextProxy::CommitText, this,
            &QAimPlatformInputContext::commitText, Qt::QueuedConnection);
#else
    connect(d->context, SIGNAL(CommitText(QString)), this,
            SLOT(commitText(QString)), Qt::QueuedConnection);
#endif
}

QAimPlatformInputContextPrivate::QAimPlatformInputContextPrivate()
    : connection(nullptr),
      bus(nullptr),
      context(nullptr),
      valid(false),
      busConnected(false),
      needsSurroundingText(false),
      icid(0)
{
    connection = new QDBusConnection(QDBusConnection::sessionBus());
    valid = !QStandardPaths::findExecutable(QString::fromLocal8Bit("alphaimd"), QStringList()).isEmpty();
    if (!valid) {
        syslog(LOG_ERR, "%s: no executable bin.\n", IM_NAME);
        return;
    }

    QDBG("QAimPlatformInputContextPrivate()\n");
    //createBusProxy();  /* May block main thread.*/
}

bool QAimPlatformInputContextPrivate::createBusProxy()
{
    QDBusReply<uint> ic;

    destroyBusProxy();

    if (!connection || !connection->isConnected())
        return false;

    bus = new QAimProxy(QLatin1String(AIM_SRV_NAME),
                         QLatin1String(AIM_SRV_PATH),
                         *connection);
    if (!bus->isValid()) {
        syslog(LOG_ERR,"[%s]:QAimIC: invalid bus.\n", IM_NAME);
        goto err_bus;
    }

    ic = bus->CreateInputContext(QLatin1String(AIM_QT_IC_NAME));
    if (!ic.isValid()) {
        syslog(LOG_ERR,"[%s]:QAimIC: create IC failed,%s\n", IM_NAME, ic.error());
        goto err_bus;
    }
    icid = ic.value();

    context = new QAimInputContextProxy(QLatin1String(AIM_SRV_NAME),
                                        QLatin1String(AIM_INPUT_CONTEXT_PATH),
                                        *connection);
    if (!context->isValid()) {
        syslog(LOG_ERR,"[%s]:QAimIC: invalid input context.\n", IM_NAME);
        goto err_cntx;
    }

    QDBG("createBusProxy bus connected! ic:" << icid);
    return true;

err_cntx:
    delete context;
    context = nullptr;

err_bus:
    delete bus;
    bus = nullptr;

    return false;
}

void QAimPlatformInputContextPrivate::destroyBusProxy()
{
    busConnected = false;
    icid = 0;

    if (context) {
        Q_ASSERT(QThread::currentThread() == context->thread());
        delete  context;
        context = nullptr;
    }

    if (bus) {
        Q_ASSERT(QThread::currentThread() == bus->thread());
        delete bus;
        bus = nullptr;
    }
}

QT_END_NAMESPACE
