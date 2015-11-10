/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Sense Hat module
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QSenseHatFb>
#include <QPainter>
#include <unistd.h>

int main(int argc, char **argv)
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.sensehat=true"));
    QCoreApplication app(argc, argv);

    QSenseHatFb fb;
    qDebug("Framebuffer valid = %d", fb.isValid());
    if (!fb.isValid())
        return 1;

    fb.setLowLight(true);

    QPainter p(fb.paintDevice());
    int x = 7, dx = -1;
    Qt::GlobalColor col = Qt::white;
    for (int i = 0; i < 200; ++i) {
        p.fillRect(QRect(QPoint(), fb.size()), Qt::black);
        p.setPen(col);
        p.drawEllipse(QPoint(x, 4), 3, 3);
        p.drawLine(QPoint(x, 4), QPoint(x + 3, 7));
        x += dx;
        if (x < -4 || x > 10)
            dx *= -1;
        usleep(1000 * 100);
        if (!(i % 8)) {
            col = Qt::GlobalColor(col + 1);
            if (col == Qt::transparent)
                col = Qt::white;
        }
    }

    p.fillRect(QRect(QPoint(), fb.size()), Qt::black);
    return 0;
}
