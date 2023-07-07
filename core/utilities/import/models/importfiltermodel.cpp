/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-21-06
 * Description : Qt filter model for import items
 *
 * SPDX-FileCopyrightText: 2012 by Islam Wazery <wazery at ubuntu dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importfiltermodel.h"

// Local includes

#include "camiteminfo.h"
#include "importfilter.h"
#include "importimagemodel.h"

namespace Digikam
{

ImportSortFilterModel::ImportSortFilterModel(QObject* const parent)
    : DCategorizedSortFilterProxyModel(parent),
      m_chainedModel                  (nullptr)
{
}

ImportSortFilterModel::~ImportSortFilterModel()
{
}

void ImportSortFilterModel::setSourceImportModel(ImportItemModel* const sourceModel)
{
    if (m_chainedModel)
    {
        m_chainedModel->setSourceImportModel(sourceModel);
    }
    else
    {
        setDirectSourceImportModel(sourceModel);
    }
}

ImportItemModel* ImportSortFilterModel::sourceImportModel() const
{
    if (m_chainedModel)
    {
        return m_chainedModel->sourceImportModel();
    }

    return static_cast<ImportItemModel*>(sourceModel());
}

void ImportSortFilterModel::setSourceFilterModel(ImportSortFilterModel* const sourceModel)
{
    if (sourceModel)
    {
        ImportItemModel* const model = sourceImportModel();

        if (model)
        {
            sourceModel->setSourceImportModel(model);
        }
    }

    m_chainedModel = sourceModel;
    setSourceModel(sourceModel);
}

ImportSortFilterModel* ImportSortFilterModel::sourceFilterModel() const
{
    return m_chainedModel;
}

QModelIndex ImportSortFilterModel::mapToSourceImportModel(const QModelIndex& proxyIndex) const
{
    if (!proxyIndex.isValid())
    {
        return QModelIndex();
    }

    if (m_chainedModel)
    {
        return m_chainedModel->mapToSourceImportModel(mapToSource(proxyIndex));
    }

    return mapToSource(proxyIndex);
}

QModelIndex ImportSortFilterModel::mapFromSourceImportModel(const QModelIndex& importModelIndex) const
{
    if (!importModelIndex.isValid())
    {
        return QModelIndex();
    }

    if (m_chainedModel)
    {
        return mapFromSource(m_chainedModel->mapFromSourceImportModel(importModelIndex));
    }

    return mapFromSource(importModelIndex);
}

QModelIndex ImportSortFilterModel::mapFromDirectSourceToSourceImportModel(const QModelIndex& sourceModelIndex) const
{
    if (!sourceModelIndex.isValid())
    {
        return QModelIndex();
    }

    if (m_chainedModel)
    {
        return m_chainedModel->mapToSourceImportModel(sourceModelIndex);
    }

    return sourceModelIndex;
}

QList<QModelIndex> ImportSortFilterModel::mapListToSource(const QList<QModelIndex>& indexes) const
{
    QList<QModelIndex> sourceIndexes;

    Q_FOREACH (const QModelIndex& index, indexes)
    {
        sourceIndexes << mapToSourceImportModel(index);
    }

    return sourceIndexes;
}

QList<QModelIndex> ImportSortFilterModel::mapListFromSource(const QList<QModelIndex>& sourceIndexes) const
{
    QList<QModelIndex> indexes;

    Q_FOREACH (const QModelIndex& index, sourceIndexes)
    {
        indexes << mapFromSourceImportModel(index);
    }

    return indexes;
}

CamItemInfo ImportSortFilterModel::camItemInfo(const QModelIndex& index) const
{
    return sourceImportModel()->camItemInfo(mapToSourceImportModel(index));
}

qlonglong ImportSortFilterModel::camItemId(const QModelIndex& index) const
{
    return sourceImportModel()->camItemId(mapToSourceImportModel(index));
}

QList<CamItemInfo> ImportSortFilterModel::camItemInfos(const QList<QModelIndex>& indexes) const
{
    QList<CamItemInfo> infos;

    Q_FOREACH (const QModelIndex& index, indexes)
    {
        infos << camItemInfo(index);
    }

    return infos;
}

QList<qlonglong> ImportSortFilterModel::camItemIds(const QList<QModelIndex>& indexes) const
{
    QList<qlonglong> ids;

    Q_FOREACH (const QModelIndex& index, indexes)
    {
        ids << camItemId(index);
    }

    return ids;
}

QModelIndex ImportSortFilterModel::indexForPath(const QString& filePath) const
{
    QUrl fileUrl = QUrl::fromLocalFile(filePath);

    return mapFromSourceImportModel(sourceImportModel()->indexForUrl(fileUrl));
}

QModelIndex ImportSortFilterModel::indexForCamItemInfo(const CamItemInfo& info) const
{
    return mapFromSourceImportModel(sourceImportModel()->indexForCamItemInfo(info));
}

QModelIndex ImportSortFilterModel::indexForCamItemId(qlonglong id) const
{
    return mapFromSourceImportModel(sourceImportModel()->indexForCamItemId(id));
}

QList<CamItemInfo> ImportSortFilterModel::camItemInfosSorted() const
{
    QList<CamItemInfo> infos;
    const int          size = rowCount();

    for (int i = 0 ; i < size ; ++i)
    {
        infos << camItemInfo(index(i, 0));
    }

    return infos;
}

ImportFilterModel* ImportSortFilterModel::importFilterModel() const
{
    if (m_chainedModel)
    {
        return m_chainedModel->importFilterModel();
    }

    return nullptr;
}

void ImportSortFilterModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    DCategorizedSortFilterProxyModel::setSourceModel(sourceModel);
}

void ImportSortFilterModel::setDirectSourceImportModel(ImportItemModel* const sourceModel)
{
    setSourceModel(sourceModel);
}

//--- ImportFilterModel methods ---------------------------------

class Q_DECL_HIDDEN ImportFilterModel::ImportFilterModelPrivate : public QObject
{
    Q_OBJECT

public:

    ImportFilterModelPrivate()
      : q              (nullptr),
        importItemModel(nullptr),
        filter         (nullptr)
    {
    }

    void init(ImportFilterModel* const _q);

Q_SIGNALS:

    void reAddCamItemInfos(const QList<CamItemInfo>&);
    void reAddingFinished();

public:

    ImportFilterModel*  q;
    ImportItemModel*    importItemModel;
    CamItemSortSettings sorter;
    Filter*             filter;

private:

    // Disable
    ImportFilterModelPrivate(QObject*);
};

void ImportFilterModel::ImportFilterModelPrivate::init(ImportFilterModel* const _q)
{
    q = _q;
}

ImportFilterModel::ImportFilterModel(QObject* const parent)
    : ImportSortFilterModel(parent),
      d_ptr(new ImportFilterModelPrivate)
{
    d_ptr->init(this);
}

ImportFilterModel::~ImportFilterModel()
{
    Q_D(ImportFilterModel);
    delete d;
}

QVariant ImportFilterModel::data(const QModelIndex& index, int role) const
{
    Q_D(const ImportFilterModel);

    if (!index.isValid())
    {
        return QVariant();
    }

    switch (role)
    {
        case DCategorizedSortFilterProxyModel::CategoryDisplayRole:
            return categoryIdentifier(d->importItemModel->camItemInfoRef(mapToSource(index)));

        case CategorizationModeRole:
            return d->sorter.categorizationMode;

        case SortOrderRole:
            return d->sorter.sortRole;

        case CategoryFormatRole:
            return d->importItemModel->camItemInfoRef(mapToSource(index)).mime;

        case CategoryDateRole:
            return d->importItemModel->camItemInfoRef(mapToSource(index)).ctime;

        case ImportFilterModelPointerRole:
            return QVariant::fromValue(const_cast<ImportFilterModel*>(this));
    }

    return DCategorizedSortFilterProxyModel::data(index, role);
}

ImportFilterModel* ImportFilterModel::importFilterModel() const
{
    return const_cast<ImportFilterModel*>(this);
}

// --- Sorting and Categorization ----------------------------------------------

void ImportFilterModel::setCamItemSortSettings(const CamItemSortSettings& sorter)
{
    Q_D(ImportFilterModel);
    d->sorter = sorter;
    setCategorizedModel(d->sorter.categorizationMode != CamItemSortSettings::NoCategories);
    invalidate();
}

void ImportFilterModel::setCategorizationMode(CamItemSortSettings::CategorizationMode mode)
{
    Q_D(ImportFilterModel);
    d->sorter.setCategorizationMode(mode);
    setCamItemSortSettings(d->sorter);
}

void ImportFilterModel::setSortRole(CamItemSortSettings::SortRole role)
{
    Q_D(ImportFilterModel);
    d->sorter.setSortRole(role);
    setCamItemSortSettings(d->sorter);
}

void ImportFilterModel::setSortOrder(CamItemSortSettings::SortOrder order)
{
    Q_D(ImportFilterModel);
    d->sorter.setSortOrder(order);
    setCamItemSortSettings(d->sorter);
}

void ImportFilterModel::setStringTypeNatural(bool natural)
{
    Q_D(ImportFilterModel);
    d->sorter.setStringTypeNatural(natural);
    setCamItemSortSettings(d->sorter);
}

void ImportFilterModel::setFilter(Digikam::Filter* filter)
{
    Q_D(ImportFilterModel);
    d->filter = filter;
    invalidateFilter();
}

void ImportFilterModel::setCameraThumbsController(CameraThumbsCtrl* const thumbsCtrl)
{
    Q_D(ImportFilterModel);
    d->importItemModel->setCameraThumbsController(thumbsCtrl);
}

void ImportFilterModel::setSendCamItemInfoSignals(bool sendSignals)
{
    if (sendSignals)
    {
        connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(slotRowsInserted(QModelIndex,int,int)));

        connect(this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));
    }
    else
    {
        disconnect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(slotRowsInserted(QModelIndex,int,int)));

        disconnect(this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));
    }
}

void ImportFilterModel::slotRowsInserted(const QModelIndex& /*parent*/, int start, int end)
{
    QList<CamItemInfo> infos;

    for (int i = start ; i < end ; ++i)
    {
        infos << camItemInfo(index(i, 0));
    }

    Q_EMIT camItemInfosAdded(infos);
}

void ImportFilterModel::slotRowsAboutToBeRemoved(const QModelIndex& /*parent*/, int start, int end)
{
    QList<CamItemInfo> infos;

    for (int i = start ; i < end ; ++i)
    {
        infos << camItemInfo(index(i, 0));
    }

    Q_EMIT camItemInfosAboutToBeRemoved(infos);
}

void ImportFilterModel::setDirectSourceImportModel(ImportItemModel* const sourceModel)
{
    Q_D(ImportFilterModel);

    if (d->importItemModel)
    {
/*
        disconnect(d->importItemModel, SIGNAL(modelReset()),
                   this, SLOT(slotModelReset()));

        // TODO: slotModelReset(); will be added when implementing filtering options
*/
        disconnect(d->importItemModel, SIGNAL(processAdded(QList<CamItemInfo>)),
                   this, SLOT(slotProcessAdded(QList<CamItemInfo>)));
    }

    // TODO do we need to delete the old one?

    d->importItemModel = sourceModel;

    if (d->importItemModel)
    {
/*
        connect(d, SIGNAL(reAddCamItemInfos(QList<CamItemInfo>)),
                d->importItemModel, SLOT(reAddCamItemInfos(QList<CamItemInfo>)));

        connect(d, SIGNAL(reAddingFinished()),
                d->importItemModel, SLOT(reAddingFinished()));

        connect(d->importItemModel, SIGNAL(modelReset()),
                this, SLOT(slotModelReset()));
*/
        connect(d->importItemModel, SIGNAL(processAdded(QList<CamItemInfo>)),
                this, SLOT(slotProcessAdded(QList<CamItemInfo>)));
    }

    setSourceModel(d->importItemModel);
}

void ImportFilterModel::slotProcessAdded(const QList<CamItemInfo>&)
{
    invalidate();
}

int ImportFilterModel::compareCategories(const QModelIndex& left, const QModelIndex& right) const
{
    Q_D(const ImportFilterModel);

    if (!d->sorter.isCategorized())
    {
        return 0;
    }

    if (!left.isValid() || !right.isValid())
    {
        return -1;
    }

    return compareInfosCategories(d->importItemModel->camItemInfoRef(left), d->importItemModel->camItemInfoRef(right));
}

bool ImportFilterModel::subSortLessThan(const QModelIndex& left, const QModelIndex& right) const
{
    Q_D(const ImportFilterModel);

    if (!left.isValid() || !right.isValid())
    {
        return true;
    }

    if (left == right)
    {
        return false;
    }

    const CamItemInfo& leftInfo  = d->importItemModel->camItemInfoRef(left);
    const CamItemInfo& rightInfo = d->importItemModel->camItemInfoRef(right);

    if (leftInfo == rightInfo)
    {
        return d->sorter.lessThan(left.data(ImportItemModel::ExtraDataRole), right.data(ImportItemModel::ExtraDataRole));
    }

    return infosLessThan(leftInfo, rightInfo);
}

int ImportFilterModel::compareInfosCategories(const CamItemInfo& left, const CamItemInfo& right) const
{
    Q_D(const ImportFilterModel);

    return d->sorter.compareCategories(left, right);
}

bool ImportFilterModel::infosLessThan(const CamItemInfo& left, const CamItemInfo& right) const
{
    Q_D(const ImportFilterModel);

    return d->sorter.lessThan(left, right);
}

QString ImportFilterModel::categoryIdentifier(const CamItemInfo& info) const
{
    Q_D(const ImportFilterModel);

    switch (d->sorter.categorizationMode)
    {
        case CamItemSortSettings::NoCategories:
            return QString();

        case CamItemSortSettings::CategoryByFolder:
            return info.folder;

        case CamItemSortSettings::CategoryByFormat:
            return info.mime;

        case CamItemSortSettings::CategoryByDate:
            return info.ctime.date().toString(Qt::ISODate);

        default:
            return QString();
    }
}

bool ImportFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const ImportFilterModel);

    if (!d->filter)
    {
        return true;
    }

    QModelIndex idx         = sourceModel()->index(source_row, 0, source_parent);
    const CamItemInfo &info = d->importItemModel->camItemInfo(idx);

    if (d->filter->matchesCurrentFilter(info))
    {
        return true;
    }

    return false;
}

// -------------------------------------------------------------------------------------------------------

NoDuplicatesImportFilterModel::NoDuplicatesImportFilterModel(QObject* const parent)
    : ImportSortFilterModel(parent)
{
}

bool NoDuplicatesImportFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    if (index.data(ImportItemModel::ExtraDataDuplicateCount).toInt() <= 1)
    {
        return true;
    }

    QModelIndex previousIndex = sourceModel()->index(source_row - 1, 0, source_parent);

    if (!previousIndex.isValid())
    {
        return true;
    }

    if (sourceImportModel()->camItemId(mapFromDirectSourceToSourceImportModel(index)) ==
        sourceImportModel()->camItemId(mapFromDirectSourceToSourceImportModel(previousIndex)))
    {
        return false;
    }

    return true;
}

} // namespace Digikam

#include "importfiltermodel.moc"
