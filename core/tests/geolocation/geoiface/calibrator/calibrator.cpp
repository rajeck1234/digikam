/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-18
 * Description : A tool to calibrate the tiling levels
 *               used in geolocation interface
 *
 * SPDX-FileCopyrightText: 2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "calibrator.h"

// Qt includes

#include <QAction>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QApplication>
#include <QCommandLineParser>
#include <QLineEdit>

// Local includes

#include "digikam_debug.h"
#include "abstractmarkertiler.h"
#include "itemmarkertiler.h"
#include "mapwidget.h"
#include "geocoordinates.h"
#include "digikam_version.h"

using namespace Digikam;

const int CoordinatesRole = Qt::UserRole + 1;

class Q_DECL_HIDDEN CalibratorModelHelper::Private
{
public:

    explicit Private()
        : model(nullptr)
    {
    }

    QStandardItemModel* model;
};

CalibratorModelHelper::CalibratorModelHelper(QStandardItemModel* const model,
                                             QObject* const parent)
    : GeoModelHelper(parent),
      d             (new Private())
{
    d->model = model;
}

CalibratorModelHelper::~CalibratorModelHelper()
{
    delete d;
}

QAbstractItemModel* CalibratorModelHelper::model() const
{
    return d->model;
}

QItemSelectionModel* CalibratorModelHelper::selectionModel() const
{
    return nullptr;
}

bool CalibratorModelHelper::itemCoordinates(const QModelIndex& index,
                                            GeoCoordinates* const coordinates) const
{
    if (!index.isValid())
        return false;

    const QVariant coordinatesVariant       = index.data(CoordinatesRole);
    GeoCoordinates itemCoordinates = coordinatesVariant.value<GeoCoordinates>();

    if (coordinates)
        *coordinates = itemCoordinates;

    return itemCoordinates.hasCoordinates();
}

void CalibratorModelHelper::setItemCoordinates(const QModelIndex& index,
                                               const GeoCoordinates& coordinates)
{
    if (!index.isValid())
        return;

    d->model->setData(index, QVariant::fromValue(coordinates), CoordinatesRole);
}

GeoModelHelper::PropertyFlags CalibratorModelHelper::modelFlags() const
{
    return FlagVisible;
}

// ---------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN Calibrator::Private
{
public:

    explicit Private()
     : hBoxLayout       (nullptr),
       model            (nullptr),
       modelHelper      (nullptr),
       markerTiler      (nullptr),
       groupingMode     (nullptr),
       sbLevel          (nullptr),
       zoomDisplay      (nullptr),
       zoomDisplayTimer (nullptr)
    {
    }

    QHBoxLayout*                                     hBoxLayout;
    QList<QPair<QWidget*, MapWidget*> >              extraWidgetHolders;
    QStandardItemModel*                              model;
    CalibratorModelHelper*                           modelHelper;
    ItemMarkerTiler*                                 markerTiler;

    QButtonGroup*                                    groupingMode;
    QSpinBox*                                        sbLevel;
    QLineEdit*                                       zoomDisplay;
    QTimer*                                          zoomDisplayTimer;
};

Calibrator::Calibrator(QWidget* const parent)
    : QMainWindow(parent),
      d          (new Private())
{
    d->model       = new QStandardItemModel(this);
    d->modelHelper = new CalibratorModelHelper(d->model, this);
    d->markerTiler = new ItemMarkerTiler(d->modelHelper, this);

    QVBoxLayout* const vboxLayout1 = new QVBoxLayout();
    QWidget* const dummy1          = new QWidget(this);
    dummy1->setLayout(vboxLayout1);
    setCentralWidget(dummy1);

    d->hBoxLayout = new QHBoxLayout();
    vboxLayout1->addLayout(d->hBoxLayout);

    d->groupingMode                     = new QButtonGroup(this);
    d->groupingMode->setExclusive(true);
    QRadioButton* const buttonGrouped   = new QRadioButton(QLatin1String("Grouped"), this);
    d->groupingMode->addButton(buttonGrouped, 0);
    QRadioButton* const buttonUngrouped = new QRadioButton(QLatin1String("Ungrouped"), this);
    d->groupingMode->addButton(buttonUngrouped, 1);
    buttonGrouped->setChecked(true);

    d->sbLevel                 = new QSpinBox(this);
    d->sbLevel->setMinimum(1);
    d->sbLevel->setMaximum(TileIndex::MaxLevel);
    QLabel* const labelsbLevel = new QLabel(QLatin1String("Level:"), this);
    labelsbLevel->setBuddy(d->sbLevel);

    d->zoomDisplay                 = new QLineEdit(this);
    d->zoomDisplay->setReadOnly(true);
    QLabel* const labelZoomDisplay = new QLabel(QLatin1String("Zoom:"), this);
    labelZoomDisplay->setBuddy(d->zoomDisplay);

    QHBoxLayout* const hboxLayout1 = new QHBoxLayout(this);
    hboxLayout1->addWidget(new QLabel(QLatin1String("Display mode:"), this));
    hboxLayout1->addWidget(buttonGrouped);
    hboxLayout1->addWidget(buttonUngrouped);
    hboxLayout1->addWidget(labelsbLevel);
    hboxLayout1->addWidget(d->sbLevel);
    hboxLayout1->addWidget(labelZoomDisplay);
    hboxLayout1->addWidget(d->zoomDisplay);
    hboxLayout1->addStretch(10);
    vboxLayout1->addLayout(hboxLayout1);

    QHBoxLayout* const hboxLayout2 = new QHBoxLayout(this);
    QPushButton* const pbAddMap    = new QPushButton(QLatin1String("Add Map Widget"), this);
    hboxLayout2->addWidget(pbAddMap);
    QPushButton* const pbRemoveMap = new QPushButton(QLatin1String("Remove Map Widget"), this);
    hboxLayout2->addWidget(pbRemoveMap);
    vboxLayout1->addLayout(hboxLayout2);

    connect(d->groupingMode, SIGNAL(buttonClicked(int)),
            this, SLOT(updateGroupingMode()));

    connect(d->sbLevel, SIGNAL(valueChanged(int)),
            this, SLOT(updateMarkers()));

    connect(pbAddMap, SIGNAL(clicked()),
            this, SLOT(slotAddMapWidget()));

    connect(pbRemoveMap, SIGNAL(clicked()),
            this, SLOT(slotRemoveMapWidget()));

    updateMarkers();
    updateGroupingMode();

    slotAddMapWidget();

    d->zoomDisplayTimer = new QTimer(this);
    d->zoomDisplayTimer->start(200);

    connect(d->zoomDisplayTimer, SIGNAL(timeout()),
            this, SLOT(updateZoomView()));
}

Calibrator::~Calibrator()
{
    delete d;
}

void Calibrator::updateGroupingMode()
{
    const bool shouldBeGrouped = (d->groupingMode->checkedId() == 0);

    for (int i = 0 ; i < d->extraWidgetHolders.count() ; ++i)
    {
        MapWidget* const mapWidget = d->extraWidgetHolders.at(i).second;

        if (shouldBeGrouped)
        {
            mapWidget->removeUngroupedModel(d->modelHelper);
            mapWidget->setGroupedModel(d->markerTiler);
        }
        else
        {
            mapWidget->setGroupedModel(nullptr);
            mapWidget->addUngroupedModel(d->modelHelper);
        }
    }
}

void Calibrator::addMarkerAt(const GeoCoordinates& coordinates)
{
    qCDebug(DIGIKAM_TESTS_LOG) << coordinates;
    QStandardItem* const item = new QStandardItem(coordinates.geoUrl());
    item->setData(QVariant::fromValue(coordinates), CoordinatesRole);

    d->model->appendRow(item);
}

void Calibrator::updateMarkers()
{
    d->model->clear();

    const int newLevel = d->sbLevel->value();
    const int Tiling   = TileIndex::Tiling;

    // add markers in all four corners and in the middle of the edges:
    typedef QPair<int, int> QIntPair;
    QList<QIntPair> partialTilePositions;

    // corners:
    partialTilePositions
        << QIntPair(0, 0)
        << QIntPair(Tiling-1, Tiling-1)
        << QIntPair(Tiling*(Tiling-1), Tiling*(Tiling-1))
        << QIntPair(Tiling*Tiling-1, Tiling*Tiling-1);

    // middle of edges:
    partialTilePositions
        << QIntPair(Tiling/2, 0)
        << QIntPair(Tiling*(Tiling/2), 0)
        << QIntPair(Tiling*(Tiling/2)+Tiling-1, (Tiling-1))
        << QIntPair(Tiling*Tiling-Tiling/2-1, Tiling*Tiling-1);

    // center of the map:
    partialTilePositions
        << QIntPair(Tiling*(Tiling/2)+Tiling/2, 0)
        << QIntPair(Tiling*(Tiling/2-1)+Tiling/2, Tiling*(Tiling-1))
        << QIntPair(Tiling*(Tiling/2-1)+Tiling/2-1, Tiling*Tiling-1)
        << QIntPair(Tiling*(Tiling/2)+Tiling/2-1, Tiling-1);

    // at +/- ~70 degrees (cutoff of Mercator projection is at 80):
    partialTilePositions
        << QIntPair(Tiling, 0)
        << QIntPair(2*Tiling-1, Tiling-1)
        << QIntPair(Tiling*(Tiling-2), Tiling*(Tiling-1))
        << QIntPair(Tiling*(Tiling-1)-1, Tiling*Tiling-1);

    for (int ptp = 0; ptp<partialTilePositions.count(); ++ptp)
    {
        QIntPair currentPair     = partialTilePositions.at(ptp);
        const int level0Index    = currentPair.first;
        const int followingIndex = currentPair.second;

        TileIndex markerIndex;
        markerIndex.appendLinearIndex(level0Index);

        for (int level = 1 ; level < newLevel-2 ; level++)
        {
            markerIndex.appendLinearIndex(followingIndex);
        }

        const int smallPart = followingIndex % Tiling;

        for (int i = -1 ; i <= 1 ; ++i)
        {
            if ((smallPart+i >= 0) && (smallPart+i < Tiling))
            {
                for (int j = -1 ; j <= 1 ; ++j)
                {
                    const int newLinIndex = followingIndex + i + j*Tiling;

                    if ((newLinIndex >= 0) && (newLinIndex < Tiling*Tiling))
                    {
                        TileIndex newIndex = markerIndex;
                        newIndex.appendLinearIndex(newLinIndex);
                        addMarkerAt(newIndex.toCoordinates());
/*
                         for (int corner = 1 ; corner <= 4 ; ++corner)
                         {
                             addMarkerAt(newIndex.toCoordinates(TileIndex::CornerPosition(corner)));
                         }
*/
                    }
                }
            }
        }
    }

    qCDebug(DIGIKAM_TESTS_LOG)<<d->model->rowCount();
}

void Calibrator::updateZoomView()
{
    if (d->extraWidgetHolders.isEmpty())
    {
        return;
    }

    MapWidget* const firstMapWidget = d->extraWidgetHolders.first().second;
    const QString newZoom                        = firstMapWidget->getZoom();

    if (newZoom!=d->zoomDisplay->text())
    {
        d->zoomDisplay->setText(newZoom);
    }
}

void Calibrator::slotAddMapWidget()
{
    QVBoxLayout* const boxLayout            = new QVBoxLayout();
    MapWidget* const mapWidget = new MapWidget();
    boxLayout->addWidget(mapWidget);
    boxLayout->addWidget(mapWidget->getControlWidget());

    QAction* const activateMapAction = new QAction(QLatin1String("Active"), mapWidget);
    activateMapAction->setData(QVariant::fromValue<void*>(mapWidget));
    activateMapAction->setCheckable(true);
    QToolButton* const toolButton    = new QToolButton(mapWidget);
    toolButton->setDefaultAction(activateMapAction);
    mapWidget->addWidgetToControlWidget(toolButton);

    connect(activateMapAction, SIGNAL(triggered(bool)),
            this, SLOT(slotActivateMapActionTriggered(bool)));

    QWidget* const dummyWidget = new QWidget();
    dummyWidget->setLayout(boxLayout);
    d->extraWidgetHolders.append(QPair<QWidget*, MapWidget*>(dummyWidget, mapWidget));

    d->hBoxLayout->addWidget(dummyWidget);

    updateGroupingMode();
}

void Calibrator::slotRemoveMapWidget()
{
    if (d->extraWidgetHolders.isEmpty())
    {
        return;
    }

    QPair<QWidget*, MapWidget*> info = d->extraWidgetHolders.takeLast();
    d->hBoxLayout->removeWidget(info.first);
    delete info.first;
}

void Calibrator::slotActivateMapActionTriggered(bool state)
{
    QAction* const senderAction = qobject_cast<QAction*>(sender());

    if (!senderAction)
    {
        return;
    }

    MapWidget* const mapWidget = static_cast<MapWidget*>(senderAction->data().value<void*>());
    mapWidget->setActive(state);
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    Calibrator* const calibrator = new Calibrator();
    calibrator->show();

    return app.exec();
}
