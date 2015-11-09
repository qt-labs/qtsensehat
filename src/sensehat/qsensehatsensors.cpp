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

#include "qsensehatsensors.h"
#include <QFile>
#include <QTimer>
#include <RTIMULib.h>
#include <unistd.h>

QT_BEGIN_NAMESPACE

static const int MAX_READ_ATTEMPTS = 5;

Q_DECLARE_LOGGING_CATEGORY(qLcSH)

class QSenseHatSensorsPrivate
{
public:
    QSenseHatSensorsPrivate(QSenseHatSensors *q_ptr, QSenseHatSensors::InitFlags flags)
        : q(q_ptr), flags(flags) { }
    ~QSenseHatSensorsPrivate();

    void open();
    void update(QSenseHatSensors::UpdateFlags what);
    void report(const RTIMU_DATA &data, QSenseHatSensors::UpdateFlags what);

    QSenseHatSensors *q;
    QSenseHatSensors::InitFlags flags;
    RTIMUSettings *settings = Q_NULLPTR;
    RTIMU *rtimu = Q_NULLPTR;
    bool imuInited = false;
    int pollInterval;
    RTHumidity *rthumidity = Q_NULLPTR;
    bool humidityInited = false;
    RTPressure *rtpressure = Q_NULLPTR;
    bool pressureInited = false;
    QTimer pollTimer;
    QSenseHatSensors::UpdateFlags autoPollWhat;
    bool temperatureFromHumidity = true;

    qreal humidity = 0;
    qreal pressure = 0;
    qreal temperature = 0;
    QVector3D gyro;
    QVector3D acceleration;
    QVector3D compass;
};

QSenseHatSensorsPrivate::~QSenseHatSensorsPrivate()
{
    delete rtpressure;
    delete rthumidity;
    delete rtimu;
    delete settings;
}

void QSenseHatSensorsPrivate::open()
{
    const QString defaultConfig = QStringLiteral("/etc/RTIMULib.ini");
    const QString writableConfig = QStringLiteral("RTIMULib.ini");

    if (!flags.testFlag(QSenseHatSensors::DontCopyIniFile)) {
        if (!QFile::exists(writableConfig)) {
            qCDebug(qLcSH) << "Copying" << defaultConfig << "to" << writableConfig;
            if (QFile::exists(defaultConfig))
                QFile::copy(defaultConfig, writableConfig);
            else
                qWarning("/etc/RTIMULib.ini not found, sensors may not be functional");
        }
        settings = new RTIMUSettings;
    } else {
        settings = new RTIMUSettings("/etc", "RTIMULib");
    }

    rtimu = RTIMU::createIMU(settings);
    pollInterval = qMax(1, rtimu->IMUGetPollInterval());
    qCDebug(qLcSH, "IMU name %s Recommended poll interval %d ms", rtimu->IMUName(), pollInterval);

    rthumidity = RTHumidity::createHumidity(settings);
    qCDebug(qLcSH, "Humidity sensor name %s", rthumidity->humidityName());

    rtpressure = RTPressure::createPressure(settings);
    qCDebug(qLcSH, "Pressure sensor name %s", rtpressure->pressureName());
}

void QSenseHatSensorsPrivate::update(QSenseHatSensors::UpdateFlags what)
{
    int humFlags = QSenseHatSensors::UpdateHumidity;
    if (temperatureFromHumidity)
        humFlags |= QSenseHatSensors::UpdateTemperature;
    if (what & humFlags) {
        if (!humidityInited) {
            humidityInited = true;
            if (!rthumidity->humidityInit())
                qWarning("Failed to initialize humidity sensor");
        }
        RTIMU_DATA data;
        if (rthumidity->humidityRead(data))
            report(data, what & humFlags);
        else
            qWarning("Failed to read humidity data");
    }

    int presFlags = QSenseHatSensors::UpdatePressure;
    if (!temperatureFromHumidity)
        presFlags |= QSenseHatSensors::UpdateTemperature;
    if (what & presFlags) {
        if (!pressureInited) {
            pressureInited = true;
            if (!rtpressure->pressureInit())
                qWarning("Failed to initialize pressure sensor");
        }
        RTIMU_DATA data;
        if (rtpressure->pressureRead(data))
            report(data, what & presFlags);
        else
            qWarning("Failed to read pressure data");
    }

    const int imuFlags = QSenseHatSensors::UpdateGyro | QSenseHatSensors::UpdateAcceleration | QSenseHatSensors::UpdateCompass;
    if (what & imuFlags) {
        if (!imuInited) {
            imuInited = true;
            if (!rtimu->IMUInit())
                qWarning("Failed to initialize IMU");
        }
        int attempts = MAX_READ_ATTEMPTS;
        while (attempts--) {
            if (rtimu->IMURead())
                break;
            usleep(pollInterval * 1000);
        }
        if (attempts >= 0)
            report(rtimu->getIMUData(), what & imuFlags);
        else
            qWarning("Failed to read intertial measurement data");
    }
}

void QSenseHatSensorsPrivate::report(const RTIMU_DATA &data, QSenseHatSensors::UpdateFlags what)
{
    if (what.testFlag(QSenseHatSensors::UpdateHumidity)) {
        if (data.humidityValid) {
            humidity = data.humidity;
            emit q->humidityChanged(humidity);
        }
    }

    if (what.testFlag(QSenseHatSensors::UpdatePressure)) {
        if (data.pressureValid) {
            pressure = data.pressure;
            emit q->pressureChanged(pressure);
        }
    }

    if (what.testFlag(QSenseHatSensors::UpdateTemperature)) {
        if (data.temperatureValid) {
            temperature = data.temperature;
            emit q->temperatureChanged(temperature);
        }
    }

    if (what.testFlag(QSenseHatSensors::UpdateGyro)) {
        if (data.gyroValid) {
            gyro = QVector3D(data.gyro.x(), data.gyro.y(), data.gyro.z());
            emit q->gyroChanged(gyro);
        }
    }

    if (what.testFlag(QSenseHatSensors::UpdateAcceleration)) {
        if (data.accelValid) {
            acceleration = QVector3D(data.accel.x(), data.accel.y(), data.accel.z());
            emit q->accelerationChanged(acceleration);
        }
    }

    if (what.testFlag(QSenseHatSensors::UpdateCompass)) {
        if (data.compassValid) {
            compass = QVector3D(data.compass.x(), data.compass.y(), data.compass.z());
            emit q->compassChanged(compass);
        }
    }
}

QSenseHatSensors::QSenseHatSensors(InitFlags flags)
    : d_ptr(new QSenseHatSensorsPrivate(this, flags))
{
    d_ptr->open();
    d_ptr->pollTimer.setInterval(d_ptr->pollInterval);
    connect(&d_ptr->pollTimer, &QTimer::timeout, [this] { d_ptr->update(d_ptr->autoPollWhat); });
}

QSenseHatSensors::~QSenseHatSensors()
{
    delete d_ptr;
}

void QSenseHatSensors::poll(UpdateFlags what)
{
    Q_D(QSenseHatSensors);
    d->update(what);
}

void QSenseHatSensors::setAutoPoll(bool enable, UpdateFlags what)
{
    Q_D(QSenseHatSensors);
    if (enable) {
        d->autoPollWhat = what;
        d->pollTimer.start();
    } else {
        d->pollTimer.stop();
    }
}

qreal QSenseHatSensors::humidity() const
{
    Q_D(const QSenseHatSensors);
    return d->humidity;
}

qreal QSenseHatSensors::pressure() const
{
    Q_D(const QSenseHatSensors);
    return d->pressure;
}

qreal QSenseHatSensors::temperature() const
{
    Q_D(const QSenseHatSensors);
    return d->temperature;
}

QVector3D QSenseHatSensors::gyro() const
{
    Q_D(const QSenseHatSensors);
    return d->gyro;
}

QVector3D QSenseHatSensors::acceleration() const
{
    Q_D(const QSenseHatSensors);
    return d->acceleration;
}

QVector3D QSenseHatSensors::compass() const
{
    Q_D(const QSenseHatSensors);
    return d->compass;
}

QT_END_NAMESPACE
