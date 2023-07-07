/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-03
 * Description : Integrated, multithread face detection / recognition
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_FACE_PIPELINE_H
#define DIGIKAM_FACE_PIPELINE_H

// Local includes

#include "facepipelinepackage.h"

namespace Digikam
{

class FacePipeline : public QObject
{
    Q_OBJECT

public:

    enum FilterMode
    {
        /// Will read any given image
        ScanAll,

        /// Will skip any image that is already marked as scanned
        SkipAlreadyScanned,

        /// Will read unconfirmed faces for recognition
        ReadUnconfirmedFaces,

        /// Will read faces marked for training
        ReadFacesForTraining,

        /// Will read faces which are confirmed
        ReadConfirmedFaces
    };

    enum WriteMode
    {
        /// Write results. Merge with existing entries.
        NormalWrite,

        /// Add new results. Previous all results will be cleared.
        OverwriteAllFaces,

        /// Add new results. Previous unconfirmed results will be cleared.
        OverwriteUnconfirmed

    };

public:

    explicit FacePipeline();
    ~FacePipeline() override;

    /**
     * You can plug these four different steps in the working pipeline.
     * 1) Call any of the four plug...() methods. See below for supported combinations.
     * 2) Call construct() to set up the pipeline.
     *
     * - Database filter: Prepares database records and/or filters out items.
     *                    See FilterMode for specification.
     * - Preview loader:  If no preview loader is plugged, you must provide
     *                    a DImg for face detection and recognition
     * - Face Detector:   If no recognizer is plugged, all detected face are marked
     *                    as the unknown person
     * - Face Recognizer: If no detector is plugged, only already scanned faces
     *                    marked as unknown will be processed.
     *                    They are implicitly read from the database.
     * - DatabaseWriter:  Writes the detection and recognition results to the database.
     *                    The trainer works on a completely different storage
     *                    and is not affected by the database writer.
     * - DatabaseEditor:  Can confirm or reject faces
     *
     * PlugParallel: You can call this instead of the simple plugging method.
     * Depending on the number of processor cores of the machine and the memory cost,
     * more than one element may be plugged and process parallelly for this part of the pipeline.
     *
     * Supported combinations:
     *  (Database Filter ->) (Preview Loader ->) Detector -> Recognizer (-> DatabaseWriter)
     *  (Database Filter ->) (Preview Loader ->) Detector               (-> DatabaseWriter)
     *  (Database Filter ->) (Preview Loader ->) Recognizer             (-> DatabaseWriter)
     *  DatabaseEditor
     *  Trainer
     *  DatabaseEditor -> Trainer
     */
    void plugDatabaseFilter(FilterMode mode);
    void plugRerecognizingDatabaseFilter();
    void plugRetrainingDatabaseFilter();
    void plugFacePreviewLoader();
    void plugFaceDetector();
    void plugParallelFaceDetectors();
    void plugFaceRecognizer();
    void plugDatabaseWriter(WriteMode mode);
    void plugDatabaseEditor();
    void plugTrainer();
    void plugDetectionBenchmarker();
    void plugRecognitionBenchmarker();
    void construct();

    /**
     * Cancels all processing
     */
    void cancel();

    /**
     * Cancels and waits for the pipeline to finish
     */
    void shutDown();

    bool hasFinished()           const;
    QString benchmarkResult()    const;

    /**
     * Set the priority of the threads used by this pipeline.
     * The default setting is QThread::LowPriority.
     */
    void setPriority(QThread::Priority priority);
    QThread::Priority priority() const;

public Q_SLOTS:

    /**
     * Processes the given image info. If a filter is installed,
     * returns false if the info is skipped, or true if it is processed.
     * If no preview loader is plugged, you must provide a DImg for detection or recognition.
     * Any of the signals below will only be emitted if true is returned.
     */
    bool process(const ItemInfo& info);
    bool process(const ItemInfo& info, const DImg& image);

    /**
     * Confirm the face. Pass the original face, and additionally tag id or region
     * if they changed. Returns the confirmed face entry immediately purely for convenience,
     * it is not yet in the database (connect to signal processed() to react when the processing finished).
     * If a trainer is plugged, the face will be trained.
     */
    FaceTagsIface confirm(const ItemInfo& info,
                          const FaceTagsIface& face,
                          int assignedTagId = 0,
                          const TagRegion& assignedRegion = TagRegion());
    FaceTagsIface confirm(const ItemInfo& info,
                          const FaceTagsIface& face,
                          const DImg& image,
                          int assignedTagId = 0,
                          const TagRegion& assignedRegion = TagRegion());
    /**
     * Train the given faces.
     */
    void train(const ItemInfo& info,
               const QList<FaceTagsIface>& faces);
    void train(const ItemInfo& info,
               const QList<FaceTagsIface>& faces,
               const DImg& image);

    /**
     * Remove the given face.
     */
    void remove(const ItemInfo& info,
                const FaceTagsIface& face);

    /**
     * Add an entry manually.
     */
    FaceTagsIface addManually(const ItemInfo& info,
                              const DImg& image,
                              const TagRegion& assignedRegion);

    /**
     * Change the given face's region to newRegion.
     * Does not care for training atm.
     */
    FaceTagsIface editRegion(const ItemInfo& info,
                             const DImg& image,
                             const FaceTagsIface& databaseFace,
                             const TagRegion& newRegion);

    /**
     * Changes the given face's tagId to newTagId.
     * Used to Reject Facial Recognition suggestions, since
     * the tag needs to be converted from Unconfirmed to Unknown.
     */
    FaceTagsIface editTag(const ItemInfo& info,
                          const FaceTagsIface& databaseFace,
                          int newTagId);

    /**
     * Batch processing. If a filter is installed, the skipped() signal
     * will inform about skipped infos. Filtering is done in a thread, returns immediately.
     * Some of the signals below will be emitted in any case.
     */
    void process(const QList<ItemInfo>& infos);

    void setAccuracyAndModel(double accuracy, bool yolo);

Q_SIGNALS:

    /// Emitted when processing is scheduled.
    void scheduled();

    /// Emitted when processing has started
    void started(const QString& message);

    /// Emitted when one package begins processing
    void processing(const FacePipelinePackage& package);

    /// Emitted when one package has finished processing
    void processed(const FacePipelinePackage& package);
    void progressValueChanged(float progress);

    /// Emitted when the last package has finished processing
    void finished();

    /// Emitted when one or several packages were skipped, usually because they have already been scanned.
    void skipped(const QList<ItemInfo>& skippedInfos);

public:

    class Private;

private:

    // Disable
    explicit FacePipeline(QObject*)              = delete;
    FacePipeline(const FacePipeline&)            = delete;
    FacePipeline& operator=(const FacePipeline&) = delete;

private:

    Private* const d;
    friend class Private;
};

} // namespace Digikam

#endif // DIGIKAM_FACE_PIPELINE_H
