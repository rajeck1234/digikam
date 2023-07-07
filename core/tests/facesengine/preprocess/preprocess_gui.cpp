/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-16
 * Description : Face pre-processing CLI tool
 *
 * SPDX-FileCopyrightText: 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QApplication>
#include <QDir>
#include <QLabel>
#include <QImage>
#include <QElapsedTimer>

// Local includes

#include "digikam_debug.h"
#include "qtopencv.h"
#include "tantriggspreprocessor.h"

using namespace Digikam;

// --------------------------------------------------------------------------------------------------

QStringList toPaths(char** const argv, int startIndex, int argc)
{
    QStringList files;

    for (int i = startIndex ; i < argc ; ++i)
    {
        files << QString::fromLocal8Bit(argv[i]);
    }

    return files;
}

QList<cv::Mat> toImages(const QStringList& paths)
{
    QList<cv::Mat> images;

    Q_FOREACH (const QString& path, paths)
    {
        QByteArray s = path.toLocal8Bit();
        images << cv::imread(std::string(s.data()),

#if OPENCV_TEST_VERSION(3,99,0)

            CV_LOAD_IMAGE_GRAYSCALE

#else

            cv::IMREAD_GRAYSCALE

#endif

        );
    }

    return images;
}

// --------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN OpenCVSideBySideDisplay
{
public:

    explicit OpenCVSideBySideDisplay(int rows, int uiSize = 200)
        : bigImage(cv::Mat::zeros(uiSize*rows, 2*uiSize, CV_8UC3)),
          uiSize(uiSize),
          currentRow(0)
    {
    }

    void add(const cv::Mat& left, const cv::Mat& right)
    {
        // Draw images side-by-side for later display

        QSize size(left.cols, left.rows);
        size.scale(uiSize, uiSize, Qt::KeepAspectRatio);
        cv::Size scaleSize(size.width(), size.height());

        const int top = currentRow * uiSize;
        cv::Mat scaledLeft;
        cv::Mat scaledRight;
        cv::resize(left,  scaledLeft,  scaleSize);
        cv::resize(right, scaledRight, scaleSize);

        if (scaledLeft.channels() == 1)
        {
            cv::cvtColor(scaledLeft, scaledLeft, CV_GRAY2BGR);
        }

        if (scaledRight.channels() == 1)
        {
            cv::cvtColor(scaledRight, scaledRight, CV_GRAY2BGR);
        }

        scaledLeft.copyTo(bigImage.colRange(0, scaledLeft.cols).rowRange(top, top + scaledLeft.rows));
        scaledRight.copyTo(bigImage.colRange(uiSize, uiSize + scaledRight.cols).rowRange(top, top + scaledRight.rows));

        ++currentRow;
    }

    void show()
    {
        QLabel label;
        label.setPixmap(QtOpenCV::cvMatToQPixmap(bigImage));
        label.show();
    }

public:

    cv::Mat   bigImage;
    const int uiSize;
    int       currentRow;
};

// --------------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Bad Arguments!!!\nUsage: " << argv[0] << " preprocess <image1> <image2> ... ";
        return 0;
    }

    QApplication app(argc, argv);

    QStringList paths     = toPaths(argv, 1, argc);
    QList<cv::Mat> images = toImages(paths);

    QElapsedTimer timer;
    timer.start();

    TanTriggsPreprocessor preprocessor;
    OpenCVSideBySideDisplay display(images.size());

    Q_FOREACH (const cv::Mat& image, images)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "channels " << image.channels();
        cv::Mat processed = preprocessor.preprocess(image);
        display.add(image, processed);
    }

    int elapsed = timer.elapsed();
    qCDebug(DIGIKAM_TESTS_LOG) << "Preprocessing took " << elapsed << " for " << images.size() << " , "
             << ((float)elapsed/images.size()) << " per image";

    display.show();
    app.exec();

    return 0;
}
