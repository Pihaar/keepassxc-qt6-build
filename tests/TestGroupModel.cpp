/*
 *  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TestGroupModel.h"

#include <QSignalSpy>
#include <QTest>

#include "core/Group.h"
#include "crypto/Crypto.h"
#include "gui/group/GroupModel.h"
#include "modeltest.h"

QTEST_GUILESS_MAIN(TestGroupModel)

void TestGroupModel::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
    QVERIFY(Crypto::init());
}

void TestGroupModel::test()
{
    auto db = new Database();

    Group* groupRoot = db->rootGroup();
    groupRoot->setObjectName("groupRoot");
    groupRoot->setName("groupRoot");

    auto group1 = new Group();
    group1->setObjectName("group1");
    group1->setName("group1");
    group1->setParent(groupRoot);

    auto group11 = new Group();
    group1->setObjectName("group11");
    group11->setName("group11");
    group11->setParent(group1);

    auto group12 = new Group();
    group1->setObjectName("group12");
    group12->setName("group12");
    group12->setParent(group1);

    auto group121 = new Group();
    group1->setObjectName("group121");
    group121->setName("group121");
    group121->setParent(group12);

    auto model = new GroupModel(db, this);

    auto modelTest = new ModelTest(model, this);

    QModelIndex indexRoot = model->index(0, 0);
    QModelIndex index1 = model->index(0, 0, indexRoot);
    QModelIndex index11 = model->index(0, 0, index1);
    QPersistentModelIndex index12 = model->index(1, 0, index1);
    QModelIndex index121 = model->index(0, 0, index12);

    QCOMPARE(model->data(indexRoot).toString(), QString("groupRoot"));
    QCOMPARE(model->data(index1).toString(), QString("group1"));
    QCOMPARE(model->data(index11).toString(), QString("group11"));
    QCOMPARE(model->data(index12).toString(), QString("group12"));
    QCOMPARE(model->data(index121).toString(), QString("group121"));

    QSignalSpy spy1(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)));
    group11->setName("test");
    group121->setIcon(4);
    QCOMPARE(spy1.size(), 2);
    QCOMPARE(model->data(index11).toString(), QString("test"));

    QSignalSpy spyAboutToAdd(model, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)));
    QSignalSpy spyAdded(model, SIGNAL(rowsInserted(QModelIndex, int, int)));
    QSignalSpy spyAboutToRemove(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)));
    QSignalSpy spyRemoved(model, SIGNAL(rowsRemoved(QModelIndex, int, int)));
    QSignalSpy spyAboutToMove(model, SIGNAL(rowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));
    QSignalSpy spyMoved(model, SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)));

    auto group2 = new Group();
    group2->setObjectName("group2");
    group2->setName("group2");
    group2->setParent(groupRoot);
    QModelIndex index2 = model->index(1, 0, indexRoot);
    QCOMPARE(spyAboutToAdd.size(), 1);
    QCOMPARE(spyAdded.size(), 1);
    QCOMPARE(spyAboutToRemove.size(), 0);
    QCOMPARE(spyRemoved.size(), 0);
    QCOMPARE(spyAboutToMove.size(), 0);
    QCOMPARE(spyMoved.size(), 0);
    QCOMPARE(model->data(index2).toString(), QString("group2"));

    group12->setParent(group1, 0);
    QCOMPARE(spyAboutToAdd.size(), 1);
    QCOMPARE(spyAdded.size(), 1);
    QCOMPARE(spyAboutToRemove.size(), 0);
    QCOMPARE(spyRemoved.size(), 0);
    QCOMPARE(spyAboutToMove.size(), 1);
    QCOMPARE(spyMoved.size(), 1);
    QCOMPARE(model->data(index12).toString(), QString("group12"));

    group12->setParent(group1, 1);
    QCOMPARE(spyAboutToAdd.size(), 1);
    QCOMPARE(spyAdded.size(), 1);
    QCOMPARE(spyAboutToRemove.size(), 0);
    QCOMPARE(spyRemoved.size(), 0);
    QCOMPARE(spyAboutToMove.size(), 2);
    QCOMPARE(spyMoved.size(), 2);
    QCOMPARE(model->data(index12).toString(), QString("group12"));

    group12->setParent(group2);
    QCOMPARE(spyAboutToAdd.size(), 1);
    QCOMPARE(spyAdded.size(), 1);
    QCOMPARE(spyAboutToRemove.size(), 0);
    QCOMPARE(spyRemoved.size(), 0);
    QCOMPARE(spyAboutToMove.size(), 3);
    QCOMPARE(spyMoved.size(), 3);
    QVERIFY(index12.isValid());
    QCOMPARE(model->data(index12).toString(), QString("group12"));
    QCOMPARE(model->data(index12.model()->index(0, 0, index12)).toString(), QString("group121"));

    delete group12;
    QCOMPARE(spyAboutToAdd.size(), 1);
    QCOMPARE(spyAdded.size(), 1);
    QCOMPARE(spyAboutToRemove.size(), 2);
    QCOMPARE(spyRemoved.size(), 2);
    QCOMPARE(spyAboutToMove.size(), 3);
    QCOMPARE(spyMoved.size(), 3);
    QVERIFY(!index12.isValid());

    // test removing a group that has children
    delete group1;

    delete db;

    delete modelTest;
    delete model;
}
