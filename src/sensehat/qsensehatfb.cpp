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

#include "qsensehatfb.h"
#include <private/qcore_unix_p.h>
#include <QtCore/QLoggingCategory>
#include <QtGui/QImage>
#include <linux/fb.h>
#include <sys/mman.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcSH, "qt.sensehat")

static const int RESET_GAMMA = 0xF102;

class QSenseHatFbPrivate
{
public:
    QSenseHatFbPrivate(QSenseHatFb *q_ptr) : q(q_ptr) { }
    ~QSenseHatFbPrivate();

    void open(const QString &framebufferDevice);

    QSenseHatFb *q;
    int fd = -1;
    QRect geometry;
    int depth = 0;
    int stride = 0;
    int memSize = 0;
    int memOffset = 0;
    bool isBGR = false;
    QImage image;
};

void QSenseHatFbPrivate::open(const QString &framebufferDevice)
{
    QByteArray fn;
    if (framebufferDevice.isEmpty()) {
        for (int i = 0; i < 4; ++i) {
            QByteArray candidate = QString(QStringLiteral("/sys/class/graphics/fb%1")).arg(i).toUtf8();
            QByteArray buf;
            buf.resize(1024);
            ssize_t len = readlink(candidate.constData(), buf.data(), buf.size());
            if (len > 0) {
                if (QByteArray(buf, len).contains(QByteArrayLiteral("rpi-sense-fb"))) {
                    fn = QString(QStringLiteral("/dev/fb%1")).arg(i).toUtf8();
                    break;
                }
            } else {
                break;
            }
        }
    } else {
        fn = framebufferDevice.toUtf8();
    }

    qCDebug(qLcSH, "Framebuffer device is %s", fn.constData());

    fd = QT_OPEN(fn.constData(), O_RDWR);
    if (fd == -1) {
        qErrnoWarning(errno, "Failed to open %s", fn.constData());
        return;
    }

    fb_fix_screeninfo finfo;
    fb_var_screeninfo vinfo;
    memset(&vinfo, 0, sizeof(vinfo));
    memset(&finfo, 0, sizeof(finfo));
    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo)) {
        qErrnoWarning(errno, "Error reading fixed fb information");
        return;
    }
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
        qErrnoWarning(errno, "Error reading variable fb information");
        return;
    }

    geometry = QRect(vinfo.xoffset, vinfo.yoffset, vinfo.xres, vinfo.yres);
    depth = vinfo.bits_per_pixel;
    stride = finfo.line_length;
    memSize = finfo.smem_len;
    qCDebug(qLcSH) << "Geometry is" << geometry << "Depth" << depth
                   << "Stride" << stride << "Mem size" << memSize;

    QImage::Format format = QImage::Format_Invalid;
    const fb_bitfield rgba[4] = { vinfo.red, vinfo.green, vinfo.blue, vinfo.transp };
    switch (depth) {
    case 16: {
        const fb_bitfield rgb565[4] = {{11, 5, 0}, {5, 6, 0},
                                       {0, 5, 0}, {0, 0, 0}};
        const fb_bitfield bgr565[4] = {{0, 5, 0}, {5, 6, 0},
                                       {11, 5, 0}, {0, 0, 0}};
        if (memcmp(rgba, rgb565, 3 * sizeof(fb_bitfield)) == 0) {
            // This is pretty much what we will always hit with the Sense HAT.
            format = QImage::Format_RGB16;
        } else if (memcmp(rgba, bgr565, 3 * sizeof(fb_bitfield)) == 0) {
            format = QImage::Format_RGB16;
            isBGR = true;
        }
        break;
    }
    case 24: {
        const fb_bitfield rgb888[4] = {{16, 8, 0}, {8, 8, 0},
                                       {0, 8, 0}, {0, 0, 0}};
        const fb_bitfield bgr888[4] = {{0, 8, 0}, {8, 8, 0},
                                       {16, 8, 0}, {0, 0, 0}};
        if (memcmp(rgba, rgb888, 3 * sizeof(fb_bitfield)) == 0) {
            format = QImage::Format_RGB888;
        } else if (memcmp(rgba, bgr888, 3 * sizeof(fb_bitfield)) == 0) {
            format = QImage::Format_RGB888;
            isBGR = true;
        }
        break;
    }
    case 32: {
        const fb_bitfield argb8888[4] = {{16, 8, 0}, {8, 8, 0},
                                         {0, 8, 0}, {24, 8, 0}};
        const fb_bitfield abgr8888[4] = {{0, 8, 0}, {8, 8, 0},
                                         {16, 8, 0}, {24, 8, 0}};
        if (memcmp(rgba, argb8888, 4 * sizeof(fb_bitfield)) == 0) {
            format = QImage::Format_ARGB32;
        } else if (memcmp(rgba, argb8888, 3 * sizeof(fb_bitfield)) == 0) {
            format = QImage::Format_RGB32;
        } else if (memcmp(rgba, abgr8888, 3 * sizeof(fb_bitfield)) == 0) {
            format = QImage::Format_RGB32;
            isBGR = true;
        }
        break;
    }
    default:
        qWarning("Unsupported fb format");
        return;
    }

    qDebug(qLcSH) << "Image format" << format << "isBGR =" << isBGR;

    uchar *data = static_cast<uchar *>(mmap(Q_NULLPTR, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (data == MAP_FAILED) {
        qErrnoWarning(errno, "Failed to mmap framebuffer");
        return;
    }

    memOffset = geometry.y() * stride + geometry.x() * depth / 8;
    data += memOffset;
    image = QImage(data, geometry.width(), geometry.height(), stride, format);
}

QSenseHatFbPrivate::~QSenseHatFbPrivate()
{
    if (fd != -1) {
        if (!image.isNull())
            munmap(image.bits() - memOffset, memSize);
        QT_CLOSE(fd);
    }
}

QSenseHatFb::QSenseHatFb(const QString &framebufferDevice)
    : d_ptr(new QSenseHatFbPrivate(this))
{
    d_ptr->open(framebufferDevice);
    if (isValid())
        setLowLight(false);
}

QSenseHatFb::~QSenseHatFb()
{
    delete d_ptr;
}

bool QSenseHatFb::isValid() const
{
    Q_D(const QSenseHatFb);
    return !d->image.isNull();
}

QSize QSenseHatFb::size() const
{
    Q_D(const QSenseHatFb);
    return d->geometry.size();
}

void QSenseHatFb::setLowLight(bool enable)
{
    Q_D(QSenseHatFb);
    if (d->fd == -1)
        return;

    ioctl(d->fd, RESET_GAMMA, enable ? 1 : 0);
}

QImage *QSenseHatFb::paintDevice()
{
    Q_D(QSenseHatFb);
    return &d->image;
}

QT_END_NAMESPACE
