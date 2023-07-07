/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 16/08/2016
 * Description : Full object detection class representing the output of the
 *               shape predictor class, containing 64 facial point including
 *               eye, nose, and mouth.
 *
 * SPDX-FileCopyrightText:      2016 by Omar Amin <Omar dot moh dot amin at gmail dot com>
 * SPDX-FileCopyrightText: 2016-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fullobjectdetection.h"

namespace Digikam
{

FullObjectDetection::FullObjectDetection(const cv::Rect& rect_,
                                         const std::vector<std::vector<float> >& parts_)
    : rect (rect_),
      parts(parts_)
{
}

FullObjectDetection::FullObjectDetection()
{
}

FullObjectDetection::FullObjectDetection(const cv::Rect& rect_)
    : rect(rect_)
{
}

const cv::Rect& FullObjectDetection::get_rect() const
{
    return rect;
}

cv::Rect& FullObjectDetection::get_rect()
{
    return rect;
}

unsigned long FullObjectDetection::num_parts() const
{
    return (unsigned long)parts.size();
}

const std::vector<float>& FullObjectDetection::part(unsigned long idx) const
{
    return parts[idx];
}

std::vector<float>& FullObjectDetection::part(unsigned long idx)
{
    return parts[idx];
}

// -------------------------------------------------------------------

std::vector<cv::Rect> getEyes(const FullObjectDetection& shape)
{
    std::vector<cv::Rect> eyes;

    for (int j = 0 ; j < 2 ; ++j)
    {
        int start = j ? 36 : 42;
        int end   = j ? 41 : 47;
        int tlx, tly, brx, bry;

        // initializing

        std::vector<float> firstpoint = shape.part(start);
        tlx                           = (int)firstpoint[0];
        brx                           = (int)firstpoint[0];
        tly                           = (int)firstpoint[1];
        bry                           = (int)firstpoint[1];

        for (int i = start ; i <= end ; ++i)
        {
            std::vector<float> x = shape.part(i);

            if      (x[0] < tlx)
            {
                tlx = (int)x[0];
            }
            else if (x[0] > brx)
            {
                brx = (int)x[0];
            }

            if      (x[1] < tly)
            {
                tly = (int)x[1];
            }
            else if (x[1] > bry)
            {
                bry = (int)x[1];
            }
        }

        eyes.push_back(cv::Rect(cv::Point(tlx, tly),
                                cv::Point(brx, bry)));
    }

    return eyes;
}

} // namespace Digikam
