/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Sense HAT module
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSENSEHATESENSORS_H
#define QSENSEHATESENSORS_H

#include <QtSenseHat/qsenseglobal.h>
#include <QtCore/QObject>
#include <QtGui/QVector3D>

QT_BEGIN_NAMESPACE

class QImage;
class QSenseHatSensorsPrivate;

class QSENSEHAT_EXPORT QSenseHatSensors : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal humidity READ humidity NOTIFY humidityChanged)
    Q_PROPERTY(qreal pressure READ pressure NOTIFY pressureChanged)
    Q_PROPERTY(qreal temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(QVector3D gyro READ gyro NOTIFY gyroChanged)
    Q_PROPERTY(QVector3D acceleration READ acceleration NOTIFY accelerationChanged)
    Q_PROPERTY(QVector3D compass READ compass NOTIFY compassChanged)

public:
    enum InitFlag {
        DontCopyIniFile = 0x01
    };
    Q_DECLARE_FLAGS(InitFlags, InitFlag)

    enum UpdateFlag {
        UpdateHumidity = 0x01,
        UpdatePressure = 0x02,
        UpdateTemperature = 0x04,
        UpdateGyro = 0x08,
        UpdateAcceleration = 0x10,
        UpdateCompass = 0x20,
        UpdateAll = 0xFF
    };
    Q_DECLARE_FLAGS(UpdateFlags, UpdateFlag)

    QSenseHatSensors(InitFlags flags = 0);
    ~QSenseHatSensors();

    void poll(UpdateFlags what = UpdateAll);
    void setAutoPoll(bool enable, UpdateFlags what = UpdateAll);

    qreal humidity() const;
    qreal pressure() const;
    qreal temperature() const;
    QVector3D gyro() const;
    QVector3D acceleration() const;
    QVector3D compass() const;

signals:
    void humidityChanged(qreal value);
    void pressureChanged(qreal value);
    void temperatureChanged(qreal value);
    void gyroChanged(const QVector3D &value);
    void accelerationChanged(const QVector3D &value);
    void compassChanged(const QVector3D &value);

private:
    Q_DISABLE_COPY(QSenseHatSensors)
    Q_DECLARE_PRIVATE(QSenseHatSensors)
    QSenseHatSensorsPrivate *d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSenseHatSensors::InitFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QSenseHatSensors::UpdateFlags)

QT_END_NAMESPACE

#endif
