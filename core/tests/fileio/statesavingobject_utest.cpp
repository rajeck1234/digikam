/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : a test for the StateSavingObject class
 *
 * SPDX-FileCopyrightText: 2009 by Johannes Wienke <languitar at semipol dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Disable static analysis with clang as it report false positives.
#ifndef __clang_analyzer__

#include "statesavingobject_utest.h"

// Qt includes

#include <QBuffer>
#include <QPointer>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "statesavingobject.h"

using namespace Digikam;

QTEST_GUILESS_MAIN(StateSavingObjectTest)

class Q_DECL_HIDDEN StubStateSaver::Private
{
public:

    Private()
      : loadCalls(0),
        saveCalls(0)
    {
    }

    unsigned int loadCalls;
    unsigned int saveCalls;
};

StubStateSaver::StubStateSaver(QObject* const parent)
    : QObject(parent),
      StateSavingObject(this),
      d(new Private)
{
}

StubStateSaver::~StubStateSaver()
{
    delete d;
}

KConfigGroup StubStateSaver::getGroup()
{
    return getConfigGroup();
}

QString StubStateSaver::getEntryKey(const QString& base)
{
    return entryName(base);
}

void StubStateSaver::doLoadState()
{
    d->loadCalls++;
}

void StubStateSaver::doSaveState()
{
    d->saveCalls++;
}

bool StubStateSaver::loadCalled()
{
    return d->loadCalls > 0;
}

bool StubStateSaver::saveCalled()
{
    return d->saveCalls > 0;
}

unsigned int StubStateSaver::numLoadCalls()
{
    return d->loadCalls;
}

unsigned int StubStateSaver::numSaveCalls()
{
    return d->saveCalls;
}

// -----------------------------------------------------------------------------

StateSavingObjectTest::StateSavingObjectTest(QObject* const parent)
    : QObject(parent)
{
}

void StateSavingObjectTest::testGroupName()
{
    StubStateSaver saver;
    QCOMPARE(saver.getGroup().name(), QLatin1String("<default>"));

    const QString name = QLatin1String("testName 2");
    saver.setObjectName(name);
    QCOMPARE(saver.getGroup().name(), name);

    KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("SimpleTest Group"));
    saver.setConfigGroup(group);
    QCOMPARE(saver.getGroup().name(), group.name());

    // setting object name must not change returned group
    saver.setObjectName(QLatin1String("new Name"));
    QCOMPARE(saver.getGroup().name(), group.name());

    // resetting group must work
    KConfigGroup group2 = KSharedConfig::openConfig()->group(QLatin1String("Another SimpleTest Group"));
    saver.setConfigGroup(group2);
    QCOMPARE(saver.getGroup().name(), group2.name());
}

void StateSavingObjectTest::testPrefixUsage()
{
    // default, empty prefix
    StubStateSaver saver;
    QCOMPARE(saver.getEntryKey(QLatin1String("")), QLatin1String(""));
    QCOMPARE(saver.getEntryKey(QLatin1String("test")), QLatin1String("test"));

    const QString prefix = QLatin1String(" _Pr efix_ ");
    saver.setEntryPrefix(prefix);
    QCOMPARE(saver.getEntryKey(QLatin1String("")), prefix);
    QString tmp = prefix + QLatin1String("test");
    QCOMPARE(saver.getEntryKey(QLatin1String("test")), tmp);
}

void StateSavingObjectTest::testDirectCalling()
{
    StubStateSaver loader;
    QVERIFY(!loader.loadCalled());
    loader.loadState();
    QVERIFY(loader.loadCalled());
    QVERIFY(!loader.saveCalled());

    StubStateSaver saver;
    QVERIFY(!saver.saveCalled());
    saver.saveState();
    QVERIFY(saver.saveCalled());
    QVERIFY(!saver.loadCalled());
}

void StateSavingObjectTest::testDirectChildrenLoading()
{
    StubStateSaver* const parentSaver      = new StubStateSaver(nullptr);
    QPointer<StubStateSaver> directChild1  = new StubStateSaver(parentSaver);
    QPointer<StubStateSaver> directChild2  = new StubStateSaver(parentSaver);
    QPointer<StubStateSaver> indirectChild = new StubStateSaver(directChild1);

    parentSaver->setStateSavingDepth(StateSavingObject::DIRECT_CHILDREN);
    parentSaver->loadState();

    QVERIFY(parentSaver->loadCalled());
    QVERIFY(directChild1->loadCalled());
    QVERIFY(directChild2->loadCalled());
    QVERIFY(!indirectChild->loadCalled());

    QVERIFY(!parentSaver->saveCalled());
    QVERIFY(!directChild1->saveCalled());
    QVERIFY(!directChild2->saveCalled());
    QVERIFY(!indirectChild->saveCalled());

    delete parentSaver;
}

void StateSavingObjectTest::testDirectChildrenSaving()
{
    StubStateSaver* const parentSaver      = new StubStateSaver(nullptr);
    QPointer<StubStateSaver> directChild1  = new StubStateSaver(parentSaver);
    QPointer<StubStateSaver> directChild2  = new StubStateSaver(parentSaver);
    QPointer<StubStateSaver> indirectChild = new StubStateSaver(directChild1);

    parentSaver->setStateSavingDepth(StateSavingObject::DIRECT_CHILDREN);
    parentSaver->saveState();

    QVERIFY(parentSaver->saveCalled());
    QVERIFY(directChild1->saveCalled());
    QVERIFY(directChild2->saveCalled());
    QVERIFY(!indirectChild->saveCalled());

    QVERIFY(!parentSaver->loadCalled());
    QVERIFY(!directChild1->loadCalled());
    QVERIFY(!directChild2->loadCalled());
    QVERIFY(!indirectChild->loadCalled());

    delete parentSaver;
}

void StateSavingObjectTest::testRecursiveChildrenLoading()
{
    StubStateSaver* const parentSaver               = new StubStateSaver(nullptr);
    QPointer<StubStateSaver> directChild1           = new StubStateSaver(parentSaver);
    QPointer<StubStateSaver> directChild2           = new StubStateSaver(parentSaver);
    QPointer<StubStateSaver> indirectChild1         = new StubStateSaver(directChild1);
    QPointer<StubStateSaver> indirectChild2         = new StubStateSaver(directChild2);
    QPointer<StubStateSaver> indirectChild3         = new StubStateSaver(directChild2);
    QBuffer* const directChildStateless             = new QBuffer(parentSaver);
    QPointer<StubStateSaver> indirectStatelessChild = new StubStateSaver(directChildStateless);

    parentSaver->setStateSavingDepth(StateSavingObject::RECURSIVE);
    directChild1->setStateSavingDepth(StateSavingObject::RECURSIVE);

    parentSaver->loadState();

    QVERIFY(parentSaver->loadCalled());
    QVERIFY(directChild1->loadCalled());
    QVERIFY(directChild2->loadCalled());
    QVERIFY(indirectChild1->loadCalled());
    QVERIFY(indirectChild2->loadCalled());
    QVERIFY(indirectChild3->loadCalled());
    QVERIFY(indirectStatelessChild->loadCalled());

    const unsigned int desiredCalls = 1;
    QCOMPARE(parentSaver->numLoadCalls(), desiredCalls);
    QCOMPARE(directChild1->numLoadCalls(), desiredCalls);
    QCOMPARE(directChild2->numLoadCalls(), desiredCalls);
    QCOMPARE(indirectChild1->numLoadCalls(), desiredCalls);
    QCOMPARE(indirectChild2->numLoadCalls(), desiredCalls);
    QCOMPARE(indirectChild3->numLoadCalls(), desiredCalls);
    QCOMPARE(indirectStatelessChild->numLoadCalls(), desiredCalls);

    QCOMPARE(directChild1->getStateSavingDepth(), StateSavingObject::RECURSIVE);

    delete parentSaver;
}

void StateSavingObjectTest::testRecursiveChildrenSaving()
{
    StubStateSaver* const parentSaver               = new StubStateSaver(nullptr);
    QPointer<StubStateSaver> directChild1           = new StubStateSaver(parentSaver);
    QPointer<StubStateSaver> directChild2           = new StubStateSaver(parentSaver);
    QPointer<StubStateSaver> indirectChild1         = new StubStateSaver(directChild1);
    QPointer<StubStateSaver> indirectChild2         = new StubStateSaver(directChild2);
    QPointer<StubStateSaver> indirectChild3         = new StubStateSaver(directChild2);
    QBuffer* const directChildStateless             = new QBuffer(parentSaver);
    QPointer<StubStateSaver> indirectStatelessChild = new StubStateSaver(directChildStateless);

    parentSaver->setStateSavingDepth(StateSavingObject::RECURSIVE);
    directChild1->setStateSavingDepth(StateSavingObject::RECURSIVE);

    parentSaver->saveState();

    QVERIFY(parentSaver->saveCalled());
    QVERIFY(directChild1->saveCalled());
    QVERIFY(directChild2->saveCalled());
    QVERIFY(indirectChild1->saveCalled());
    QVERIFY(indirectChild2->saveCalled());
    QVERIFY(indirectChild3->saveCalled());
    QVERIFY(indirectStatelessChild->saveCalled());

    const unsigned int desiredCalls = 1;
    QCOMPARE(parentSaver->numSaveCalls(), desiredCalls);
    QCOMPARE(directChild1->numSaveCalls(), desiredCalls);
    QCOMPARE(directChild2->numSaveCalls(), desiredCalls);
    QCOMPARE(indirectChild1->numSaveCalls(), desiredCalls);
    QCOMPARE(indirectChild2->numSaveCalls(), desiredCalls);
    QCOMPARE(indirectChild3->numSaveCalls(), desiredCalls);
    QCOMPARE(indirectStatelessChild->numSaveCalls(), desiredCalls);

    QCOMPARE(directChild1->getStateSavingDepth(), StateSavingObject::RECURSIVE);

    delete parentSaver;
}

#endif // __clang_analyzer__
