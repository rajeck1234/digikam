/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 17-8-2016
 * Description : Some matrix utility functions including singular value
 *               decomposition, inverse, and pseudo-inverse.
 *
 * SPDX-FileCopyrightText: 2016      by Omar Amin <Omar dot moh dot amin at gmail dot com>
 * SPDX-FileCopyrightText: 2019      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 * SPDX-FileCopyrightText: 2016-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "matrixoperations.h"

// Qt includes

#include <QtGlobal>

namespace Digikam
{

namespace MatrixOperations
{

std::vector<std::vector<float> > inv2(const std::vector<std::vector<float> >& mat)
{
    Q_ASSERT((mat.size() == 2) && (mat[0].size() == 2));

    std::vector<std::vector<float> > m(2,std::vector<float>(2, 0));

    float det = mat[0][0]*mat[1][1] -
                mat[0][1]*mat[1][0];

    Q_ASSERT(det != 0);

    m[0][0] =  mat[1][1] / det;
    m[0][1] = -mat[0][1] / det;
    m[1][0] = -mat[1][0] / det;
    m[1][1] =  mat[0][0] / det;

    return m;
}

std::vector<std::vector<float> > pinv(const std::vector<std::vector<float> >& mat)
{
    std::vector<std::vector<float> > result(mat[0].size(), std::vector<float>(mat.size()));
    cv::Mat B((int)mat[0].size(), (int)mat.size(),    CV_32FC1);
    cv::Mat A((int)mat.size(),    (int)mat[0].size(), CV_32FC1);

    for (unsigned int i = 0 ; i < mat.size() ; ++i)
    {
        for (unsigned int j = 0 ; j < mat[0].size() ; ++j)
        {
            A.at<float>(i, j) = mat[i][j];
        }
    }

    cv::invert(A, B, cv::DECOMP_SVD);

    for (int i = 0 ; i < B.rows ; ++i)
    {
        for (int j = 0 ; j < B.cols ; ++j)
        {
            result[i][j] = B.at<float>(i, j);
        }
    }

    return result;
}

void stdmattocvmat(const std::vector<std::vector<float> >& src, cv::Mat& dst)
{
    for (unsigned int i = 0 ; i < src.size() ; ++i)
    {
        for (unsigned int j = 0 ; j < src[0].size() ; ++j)
        {
            dst.at<float>(i, j) = src[i][j];
        }
    }
}

void cvmattostdmat(const cv::Mat& dst, std::vector<std::vector<float> >& src)
{
    for (unsigned int i = 0 ; i < src.size() ; ++i)
    {
        for (unsigned int j = 0 ; j < src[0].size() ; ++j)
        {
            src[i][j] = dst.at<float>(i, j);
        }
    }
}

void transpose(std::vector<std::vector<float> >& src,
               std::vector<std::vector<float> >& dst)
{
    for (unsigned int i = 0 ; i < src.size() ; ++i)
    {
        for (unsigned int j = 0 ; j < src[0].size() ; ++j)
        {
            dst[i][j] = src[j][i];
        }
    }

}

float trace(const std::vector<std::vector<float> >& src)
{
    float result = 0.0F;

    for (unsigned int i = 0 ; i < src.size() ; ++i)
    {
        for (unsigned int j = 0 ; j < src[0].size() ; ++j)
        {
            if (i == j)
            {
                result += src[i][i];
            }
        }
    }

    return result;
}

bool svd3(std::vector<std::vector<float> >& a,
          std::vector<float >& w,
          std::vector<std::vector<float> >& v,
          std::vector<float >& rv1)
{
    const float one     = 1.0F;
    const long max_iter = 300;

    // a is a square matrix

    // columns

    const long n        = (long)a.size();

    // rows

    const long m        = (long)a.size();

    const float eps     = std::numeric_limits<float>::epsilon();
    long nm             = 0;
    long l              = 0;
    float g             = 0.0F;
    float scale         = 0.0F;
    float anorm         = 0.0F;
    bool flag           = false;
    float c             = 0.0F;
    float f             = 0.0F;
    float h             = 0.0F;
    float s             = 0.0F;
    float x             = 0.0F;
    float y             = 0.0F;
    float z             = 0.0F;

    for (long i = 0 ; i < n ; ++i)
    {
        l      = i + 1;
        rv1[i] = scale * g;
        g      = 0.0F;
        s      = 0.0F;
        scale  = 0.0F;

        if (i < m)
        {
            for (long k = i ; k < m ; ++k)
            {
                scale += std::abs(a[k][i]);
            }

            if (scale)
            {
                for (long k = i ; k < m ; ++k)
                {
                    a[k][i] /= scale;
                    s       += a[k][i] * a[k][i];
                }

                f       = a[i][i];
                g       = -signdlib(std::sqrt(s), f);
                h       = f * g - s;
                a[i][i] = f - g;

                for (long j = l ; j < n ; ++j)
                {
                    s = 0.0;

                    for (long k = i ; k < m ; ++k)
                    {
                        s += a[k][i] * a[k][j];
                    }

                    f = s / h;

                    for (long k = i ; k < m ; ++k)
                    {
                        a[k][j] += f * a[k][i];
                    }
                }

                for (long k = i ; k < m ; ++k)
                {
                    a[k][i] *= scale;
                }
            }
        }

        w[i]  = scale * g;
        g     = 0.0F;
        s     = 0.0F;
        scale = 0.0F;

        if ((i < m) && (i < n-1))
        {
            for (long k = l ; k < n ; ++k)
            {
                scale += std::abs(a[i][k]);
            }

            if (scale)
            {
                for (long k = l ; k < n ; ++k)
                {
                    a[i][k] /= scale;
                    s       += a[i][k] * a[i][k];
                }

                f       = a[i][l];
                g       = -signdlib(std::sqrt(s), f);
                h       = f * g - s;
                a[i][l] = f - g;

                for (long k = l ; k < n ; ++k)
                {
                    rv1[k] = a[i][k] / h;
                }

                for (long j = l ; j < m ; ++j)
                {
                    s = 0.0;

                    for (long k = l; k < n; ++k)
                    {
                        s += a[j][k] * a[i][k];
                    }

                    for (long k = l; k < n; ++k)
                    {
                        a[j][k] += s * rv1[k];
                    }
                }

                for (long k = l ; k < n ; ++k)
                {
                    a[i][k] *= scale;
                }
            }
        }

        anorm = std::max(anorm, (std::abs(w[i]) + std::abs(rv1[i])));
    }

    for (long i = n-1 ; i >= 0 ; --i)
    {
        if (i < (n-1))
        {
            if (g != 0.0F)
            {
                for (long j = l ; j < n ; ++j)
                {
                    v[j][i] = (a[i][j] / a[i][l]) / g;
                }

                for (long j = l ; j < n ; ++j)
                {
                    s = 0.0F;

                    for (long k = l ; k < n ; ++k)
                    {
                        s += a[i][k] * v[k][j];
                    }

                    for (long k = l ; k < n ; ++k)
                    {
                        v[k][j] += s*v[k][i];
                    }
                }
            }

            for (long j = l ; j < n ; ++j)
            {
                v[i][j] = v[j][i] = 0.0;
            }
        }

        v[i][i] = 1.0;
        g       = rv1[i];
        l       = i;
    }

    for (long i = std::min(m,n) - 1 ; i >= 0 ; --i)
    {
        l = i + 1;
        g = w[i];

        for (long j = l ; j < n ; ++j)
        {
            a[i][j] = 0.0;
        }

        if (g != 0)
        {
            g = 1.0F / g;

            for (long j = l ; j < n ; ++j)
            {
                s = 0.0;

                for (long k = l ; k < m ; ++k)
                {
                    s += a[k][i] * a[k][j];
                }

                f = (s / a[i][i]) * g;

                for (long k = i ; k < m ; ++k)
                {
                    a[k][j] += f * a[k][i];
                }
            }

            for (long j = i ; j < m ; ++j)
            {
                a[j][i] *= g;
            }
        }
        else
        {
            for (long j = i ; j < m ; ++j)
            {
                a[j][i] = 0.0;
            }
        }

        ++a[i][i];
    }

    for (long k = n-1 ; k >= 0 ; --k)
    {
        for (long its = 1 ; its <= max_iter ; ++its)
        {
            flag = true;

            for (l = k ; l >= 1 ; --l)
            {
                nm = l - 1;

                if (std::abs(rv1[l]) <= (eps * anorm))
                {
                    flag = false;
                    break;
                }

                if (std::abs(w[nm]) <= (eps * anorm))
                {
                    break;
                }
            }

            if (flag)
            {
                c = 0.0F;
                s = 1.0F;

                for (long i = l ; i <= k ; ++i)
                {
                    f      = s * rv1[i];
                    rv1[i] = c * rv1[i];

                    if (std::abs(f) <= (eps * anorm))
                    {
                        break;
                    }

                    g    = w[i];
                    h    = pythag(f, g);
                    w[i] = h;
                    h    = 1.0F / h;
                    c    = g * h;
                    s    = -f * h;

                    for (long j = 0 ; j < m ; ++j)
                    {
                        y        = a[j][nm];
                        z        = a[j][i];
                        a[j][nm] = y * c + z * s;
                        a[j][i]  = z * c - y * s;
                    }
                }
            }

            z = w[k];

            if (l == k)
            {
                if (z < 0.0)
                {
                    w[k] = -z;

                    for (long j = 0 ; j < n ; ++j)
                    {
                        v[j][k] = -v[j][k];
                    }
                }

                break;
            }

            if (its == max_iter)
            {
                return false;
            }

            x  = w[l];
            nm = k - 1;
            y  = w[nm];
            g  = rv1[nm];
            h  = rv1[k];
            f  = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0F * h * y);
            g  = pythag(f, one);
            f  = ((x - z) * (x + z) + h * ((y / (f + signdlib(g, f))) - h)) / x;
            c  = s = 1.0;

            for (long j = l ; j <= nm ; ++j)
            {
                long i = j + 1;
                g      = rv1[i];
                y      = w[i];
                h      = s * g;
                g      = c * g;
                z      = pythag(f, h);
                rv1[j] = z;
                c      = f/z;
                s      = h/z;
                f      = x*c + g*s;
                g      = g*c - x*s;
                h      = y*s;
                y     *= c;

                for (long jj = 0 ; jj < n ; ++jj)
                {
                    x        = v[jj][j];
                    z        = v[jj][i];
                    v[jj][j] = x*c + z*s;
                    v[jj][i] = z*c - x*s;
                }

                z    = pythag(f,h);
                w[j] = z;

                if (z != 0)
                {
                    z = 1.0F / z;
                    c = f * z;
                    s = h * z;
                }

                f = c*g + s*y;
                x = c*y - s*g;

                for (long jj = 0 ; jj < m ; ++jj)
                {
                    y        = a[jj][j];
                    z        = a[jj][i];
                    a[jj][j] = y*c + z*s;
                    a[jj][i] = z*c - y*s;
                }
            }

            rv1[l] = 0.0;
            rv1[k] = f;
            w[k]   = x;
        }
    }

    return true;
}

void svd(const std::vector<std::vector<float> >& m,
         std::vector<std::vector<float> >& u,
         std::vector<std::vector<float> >& w,
         std::vector<std::vector<float> >& v)
{

    // initialization

    u.resize(2);
    w.resize(2);
    v.resize(2);

    for (unsigned int i = 0 ; i < 2 ; ++i)
    {

        u[i].resize(2);
        w[i].resize(2);
        v[i].resize(2);

        for (unsigned int j = 0 ; j < 2 ; ++j)
        {
            u[i][j] = m[i][j];
        }
    }

    std::vector<float> W(2);
    std::vector<float> rv1(2);
    svd3(u, W, v, rv1);

    // get w from W

    for (unsigned int i = 0 ; i < 2 ; ++i)
    {
        for (unsigned int j = 0 ; j < 2 ; ++j)
        {
            if (i == j)
            {
                w[i][j] = W[i];
            }
            else
            {
                w[i][j] = 0.0;
            }
        }
    }
}

float determinant(const std::vector<std::vector<float> >& u)
{
    float result = u[0][0]*u[1][1] - u[1][0]*u[0][1];

    return result;
}

} // namespace MatrixOperations

} // namespace Digikam
