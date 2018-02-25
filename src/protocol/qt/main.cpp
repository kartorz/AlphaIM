/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** This file is part of the plugins of the Qt Toolkit.
**
**
****************************************************************************/

#include <qpa/qplatforminputcontextplugin_p.h>
#include <QtCore/QStringList>
#include "qaimplatforminputcontext.h"

QT_BEGIN_NAMESPACE

class QAimPlatformInputContextPlugin : public QPlatformInputContextPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformInputContextFactoryInterface_iid FILE "aim.json")

public:
    QAimPlatformInputContext *create(const QString&, const QStringList&) Q_DECL_OVERRIDE;
};

QAimPlatformInputContext *QAimPlatformInputContextPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);

    if (system.compare(QLatin1String("alphaim"), Qt::CaseInsensitive) == 0) {
        return new QAimPlatformInputContext;
    }
    return nullptr;
}

QT_END_NAMESPACE

#include "main.moc"
