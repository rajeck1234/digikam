/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-11-03
 * Description : Generating random numbers
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_RANDOM_NUMBER_GENERATOR_H
#define DIGIKAM_RANDOM_NUMBER_GENERATOR_H

// Qt includes

#include <QByteArray>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT NonDeterministicRandomData : public QByteArray
{
public:

    /**
     * Constructs a QByteArray of given byte size
     * filled with non-deterministic random data.
     * For larger quantities of data,
     * prefer using a RandomNumberGenerator seeded
     * with non-deterministic data.
     */
    explicit NonDeterministicRandomData(int size);
};

// --------------------------------------------------------------------

/**
 * This class differs from standard pseudo
 * random number generators (rand()) in these points:
 * - it uses a specified, independently implemented algorithm
 *   identical across platforms
 * - provides access to the used seed
 * - it can thus guarantee replayable sequences
 * - it provides convenient seeding of varying quality
 */
class DIGIKAM_EXPORT RandomNumberGenerator
{
public:

    /**
     * Constructs a random number generator that is seeded
     * with a constant value. It is recommended to call a seed method
     * after construction.
     */
    explicit RandomNumberGenerator();
    ~RandomNumberGenerator();

    /**
     * Seeds the generator from a non-deterministic
     * random number generator. This is the most secure
     * seeding method.
     * Returns the new currentSeed().
     */
    quint32 seedNonDeterministic();

    /**
     * Seeds the generator by current time. This is common practice
     * and good enough for most purposes.
     * Returns the new currentSeed().
     */
    quint32 seedByTime();

    /**
     * Produces a non-deterministic seed, as used by seedNonDeterministic()
     */
    static quint32 nonDeterministicSeed();

    /**
     * Produces a seed that includes at least the time as source
     * of random data
     */
    static quint32 timeSeed();

    /**
     * Seeds the generator with the given value.
     * This is not meant to be called with a constant value,
     * but with a value retrieved from currentSeed() on a previous run.
     * Across platforms, the same sequence of random numbers will be
     * generated for the same seed.
     */
    void seed(quint32 seed);

    /**
     * Seeds the generator again with the currentSeed().
     * This is not a no-op, rather, the sequence of random numbers
     * starts again from its beginning after each re-seed.
     * Equivalent to seed(currentSeed())
     */
    void reseed();

    /**
     * Retrieves the current seed. Can be used for seed(quint32)
     * to replay the results again.
     */
    quint32 currentSeed() const;

    /**
     * Returns a random integer in the interval [min, max]
     * (including min and max).
     * Warning: this method is non re-entrant.
     */
    int number(int min, int max);

    /**
     * Returns a random double in the interval [min, max)
     * (including min, excluding max)
     * Warning: this method is non re-entrant.
     */
    double number(double min, double max);

    /**
     * Returns true with a probability of p
     * (where p shall be in the interval [0, 1])
     * Warning: this method is non re-entrant.
     */
    bool yesOrNo(double p);

private:

    // Disable
    RandomNumberGenerator(const RandomNumberGenerator&)            = delete;
    RandomNumberGenerator& operator=(const RandomNumberGenerator&) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_RANDOM_NUMBER_GENERATOR_H
