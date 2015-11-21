Raspberry Pi Sense HAT support module for Qt 5
==============================================

Tested on Raspbian Jessie 2015/09/24 and Qt 5.6 as described on https://wiki.qt.io/RaspberryPi2EGLFS

Sensor support is based on RTIMULib as shipped in Raspbian and https://github.com/RPi-Distro/python-sense-hat.

This a true Qt module. To build, do qmake -r && make && make install. To use, add QT += sensehat.

LED example:

    int main(int argc, char **argv)
    {
        QCoreApplication app(argc, argv);
        QSenseHatFb fb;
        fb.setLowLight(true);
        QPainter p(fb.paintDevice());
        p.fillRect(QRect(QPoint(), fb.size()), Qt::black);
        for (int i = Qt::white; i < Qt::darkYellow; ++i) {
            p.setPen(Qt::GlobalColor(i));
            p.drawEllipse(QPoint(4, 4), 3, 3);
            p.drawLine(QPoint(4, 4), QPoint(7, 7));
            sleep(1);
        }
        p.fillRect(QRect(QPoint(), fb.size()), Qt::black);
        return 0;
    }

An alternative is to run with the linuxfb platform plugin and use QRasterWindow. However,
QSenseHatFb is handy because it automatically finds the right device, supports device
specifics (low-light mode), functions (to some extent) without initializing QtGui, and
allows using any platform plugin and simultaneous HDMI output.

Sensors example:

    int main(int argc, char **argv)
    {
        QCoreApplication app(argc, argv);
        QSenseHatSensors sensors;
        sensors.setAutoPoll(true);
        QObject::connect(&sensors, &QSenseHatSensors::humidityChanged, [](qreal h) {
            qDebug() << "Humidity:" << h;
        });
        QObject::connect(&sensors, &QSenseHatSensors::pressureChanged, [](qreal p) {
            qDebug() << "Pressure:" << p;
        });
        QObject::connect(&sensors, &QSenseHatSensors::temperatureChanged, [](qreal c) {
            qDebug() << "Temperature:" << c;
        });
        QObject::connect(&sensors, &QSenseHatSensors::gyroChanged, [](const QVector3D &v) {
            qDebug() << "Gyro:" << v;
        });
        QObject::connect(&sensors, &QSenseHatSensors::accelerationChanged, [](const QVector3D &v) {
            qDebug() << "Acceleration:" << v;
        });
        QObject::connect(&sensors, &QSenseHatSensors::compassChanged, [](const QVector3D &v) {
            qDebug() << "Compass:" << v;
        });
        QObject::connect(&sensors, &QSenseHatSensors::orientationChanged, [](const QVector3D &v) {
            qDebug() << "Orientation:" << v;
        });
        QTimer::singleShot(10000, &app, &QCoreApplication::quit);
        return app.exec();
    }
