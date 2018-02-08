/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "qaimplatforminputcontext.h"

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

#include <sys/types.h>
#include <signal.h>
#include <syslog.h>

#include <QtDBus>

#ifndef AIM_RELEASE_MASK
#define AIM_RELEASE_MASK (1 << 30)
#endif

QT_BEGIN_NAMESPACE

enum { debug = 0 };

class QAimPlatformInputContextPrivate
{
public:
    QAimPlatformInputContextPrivate();
    ~QAimPlatformInputContextPrivate()
    {
        delete context;
        delete bus;
        delete connection;
    }

    static QString getSocketPath();

    void createBusProxy();

    QDBusConnection *connection;
    QAimProxy *bus;
    QAimInputContextProxy *context;

    bool valid;
    bool busConnected;
    QString predit;
    bool needsSurroundingText;
    QLocale locale;
	uint icid;
};


QAimPlatformInputContext::QAimPlatformInputContext ()
    : d(new QAimPlatformInputContextPrivate())
{
	if (d->connection == NULL) {
		syslog (LOG_ERR, "{AlphaIM} Error can't connect to session bus\n");
		return;
	}

    QDBusServiceWatcher* watherReg = new QDBusServiceWatcher("org.freedesktop.AlphaIM", *(d->connection), QDBusServiceWatcher::WatchForRegistration, this);
	connect(watherReg, SIGNAL(serviceRegistered(QString)), this, SLOT(serviceRegistered()));

	QDBusServiceWatcher* watherUnreg = new QDBusServiceWatcher("org.freedesktop.AlphaIM", *(d->connection), QDBusServiceWatcher::WatchForUnregistration, this);
	connect(watherUnreg, SIGNAL(serviceUnregistered(QString)), this, SLOT(serviceUnregistered()));

	m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(connectToBus()));
	m_timer.start(100);
    //connectToContextSignals();

    QInputMethod *p = qApp->inputMethod();
    connect(p, SIGNAL(cursorRectangleChanged()), this, SLOT(cursorRectChanged()));
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
	if (debug) qDebug() << "~QAimPlatformInputContext";
	if (d->busConnected)
		d->context->Destroy();
    delete d;
}

// false: ~QAimPlatformInputContext()
bool QAimPlatformInputContext::isValid() const
{
	if (debug) qDebug() << "isValid";

    return d->valid && d->connection != NULL;
}

bool QAimPlatformInputContext::hasCapability(Capability capability) const
{
	if (debug) qDebug() << "hasCapability:" << capability;
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

    if (a == QInputMethod::Click)
        commit();
}

void QAimPlatformInputContext::reset()
{
    QPlatformInputContext::reset();

    if (!d->busConnected)
        return;

    d->context->Reset();
    d->predit = QString();
}

void QAimPlatformInputContext::commit()
{
    QPlatformInputContext::commit();

    if (!d->busConnected)
        return;

    QObject *input = qApp->focusObject();
    if (!input) {
        d->predit = QString();
        return;
    }

    if (!d->predit.isEmpty()) {
        QInputMethodEvent event;
        event.setCommitString(d->predit);
        QCoreApplication::sendEvent(input, &event);
    }

    d->context->Reset();
    d->predit = QString();
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

        /*QAimText text;
        text.text = surroundingText;

        QVariant variant;
        variant.setValue(text);
        QDBusVariant dbusText(variant);

        d->context->SetSurroundingText(dbusText, cursorPosition, anchorPosition);*/
    }
    QPlatformInputContext::update(q);
}

void QAimPlatformInputContext::cursorRectChanged()
{
    if (!d->busConnected)
        return;

    QRect r = qApp->inputMethod()->cursorRectangle().toRect();
    if(!r.isValid())
        return;
    QWindow *inputWindow = qApp->focusWindow();
    if (!inputWindow)
        return;
    r.moveTopLeft(inputWindow->mapToGlobal(r.topLeft()));
    if (debug)
        qDebug() << "microFocus" << r;
    d->context->SetCursorLocation(r.x(), r.y(), r.width(), r.height());
}

void QAimPlatformInputContext::setFocusObject(QObject *object)
{
    if (!d->busConnected)
        return;

    if (object)
        d->context->FocusIn(d->icid);
    else
        d->context->FocusOut();
}

void QAimPlatformInputContext::commitText(const QString &text)
{
	//if (debug) qDebug() << "commitText" << text;
    QObject *input = qApp->focusObject();
    if (!input)
        return;

    QInputMethodEvent event;
    event.setCommitString(text);
    QCoreApplication::sendEvent(input, &event);

    d->predit = QString();
}

bool QAimPlatformInputContext::filterEvent(const QEvent *event)
{
    if (!d->busConnected)
        return false;

    if (!inputMethodAccepted())
        return false;

    const QKeyEvent *keyEvent = static_cast<const QKeyEvent *>(event);
    quint32 sym = keyEvent->nativeVirtualKey();
    quint32 code = keyEvent->nativeScanCode();
    quint32 state = keyEvent->nativeModifiers();
    quint32 aimState = state;
	if (debug) qDebug() << "filterEvent, sym:" << sym << " code:" << code << " state:" << state ;
    if (keyEvent->type() != QEvent::KeyPress)
        aimState |= AIM_RELEASE_MASK;
    QDBusPendingReply<bool> reply = d->context->ProcessKeyEvent(sym, code - 8, aimState);

    if (m_eventFilterUseSynchronousMode || reply.isFinished()) {
        bool retval = reply.value();
        //qCDebug(qtQpaInputMethods) << "filterEvent return" << code << sym << state << retval;
        return retval;
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
	//if (debug) qDebug() << "filterEventFinished:";
    QAimFilterEventWatcher *watcher = (QAimFilterEventWatcher *) call;
    QDBusPendingReply<bool> reply = *call;

    if (reply.isError()) {
		syslog (LOG_ERR, "{AlphaIM} filterEventFinished, reply error\n");
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

    // copied from QXcbKeyboard::handleKeyEvent()
    bool retval = reply.value();

    //qCDebug(qtQpaInputMethods) << "filterEventFinished return" << code << sym << state << retval;
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

    }
    call->deleteLater();
}

QLocale QAimPlatformInputContext::locale() const
{
    return d->locale;
}

void QAimPlatformInputContext::serviceRegistered()
{
    m_timer.stop();

    if (d->context) {
		disconnect(d->context, 0, 0, 0);
        disconnect(d->context);
	}
    if (d->bus && d->bus->isValid())
        disconnect(d->bus);
    if (d->connection)
        d->connection->disconnectFromBus(QLatin1String("QAimProxy"));

    m_timer.start(100);
}

void QAimPlatformInputContext::serviceUnregistered()
{
	syslog (LOG_NOTICE, "{AlphaIM} serviceUnregistered\n");
	d->busConnected = false;
}
// When getSocketPath() is modified, the bus is not established yet
// so use m_timer.
void QAimPlatformInputContext::connectToBus()
{
	if (debug) qDebug() << "connectToBus";
	d->createBusProxy();
    connectToContextSignals();
}

void QAimPlatformInputContext::connectToContextSignals()
{
    if (d->context) {
		if (debug) qDebug() << "connectToContextSignals";
        connect(d->context, SIGNAL(CommitText(QString)), SLOT(commitText(QString)));
    }
}

QAimPlatformInputContextPrivate::QAimPlatformInputContextPrivate()
    : connection(0),
      bus(0),
      context(0),
      valid(false),
      busConnected(false),
      needsSurroundingText(false),
	  icid(0)
{
	connection = new QDBusConnection(QDBusConnection::sessionBus());
    valid = !QStandardPaths::findExecutable(QString::fromLocal8Bit("AlphaIM"), QStringList()).isEmpty();
    if (!valid) {
		syslog (LOG_ERR, "{AlphaIM} no executable bin. \n");
        return;
	}
	//createBusProxy();  // May block main thread.
}

void QAimPlatformInputContextPrivate::createBusProxy()
{
	busConnected = false;

	if (debug) qDebug() << "createBusProxy";
    if (!connection || !connection->isConnected())
        return;

    bus = new QAimProxy(QLatin1String("org.freedesktop.AlphaIM"),
                         QLatin1String("/org/freedesktop/AlphaIM"),
                         *connection);
    if (!bus->isValid()) {
        qWarning("QAimPlatformInputContext: invalid bus.");
        return;
    }

    QDBusReply<uint> ic = bus->CreateInputContext(QLatin1String("QAimInputContext"));
    if (!ic.isValid()) {
        qWarning() << "QAimPlatformInputContext: CreateInputContext failed." << ic.error();
        return;
    }
	icid = ic.value();

    context = new QAimInputContextProxy(QLatin1String("org.freedesktop.AlphaIM"),
										QLatin1String("/org/freedesktop/AlphaIM/qim"),
										*connection);
    if (!context->isValid()) {
        qWarning("QAimPlatformInputContext: invalid input context.");
        return;
    }

    context->Enable();

    if (debug)
        qDebug(">>>> bus connected!");
    busConnected = true;
}

QString QAimPlatformInputContextPrivate::getSocketPath()
{
    QByteArray display(qgetenv("DISPLAY"));
    QByteArray host = "unix";
    QByteArray displayNumber = "0";

    int pos = display.indexOf(':');
    if (pos > 0)
        host = display.left(pos);
    ++pos;
    int pos2 = display.indexOf('.', pos);
    if (pos2 > 0)
        displayNumber = display.mid(pos, pos2 - pos);
    else
        displayNumber = display.right(pos);
    if (debug)
        qDebug() << "host=" << host << "displayNumber" << displayNumber << " location:" << QStandardPaths::ConfigLocation;

    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
               QLatin1String("/aim/bus/") +
               QLatin1String(QDBusConnection::localMachineId()) +
               QLatin1Char('-') + QString::fromLocal8Bit(host) + QLatin1Char('-') + QString::fromLocal8Bit(displayNumber);
}

QT_END_NAMESPACE
