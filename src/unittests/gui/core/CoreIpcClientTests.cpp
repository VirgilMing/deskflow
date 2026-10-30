/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "base/Log.h"
#include "deskflow/ipc/CoreIpcServer.h"
#include "gui/ipc/CoreIpcClient.h"

#include <QMetaEnum>
#include <QSignalSpy>
#include <QTest>
#include <QUuid>

class CoreIpcClientTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void connectToServer_existingStatus_receivesSnapshot();
  void setClipboardStatus_allValues_roundTrips_data();
  void setClipboardStatus_allValues_roundTrips();
  void connectToServer_secondClient_receivesSnapshot();
  void setClipboardStatus_changed_receivesUpdate();
  void setClipboardStatus_same_noDuplicateNotification();
  void disconnectFromServer_available_invalidatesStatus();
  void processCommand_shutdown_invalidatesStatus();
  void processCommand_invalid_preservesLastValidStatus();
};

using deskflow::ClipboardStatus;
using Reason = ClipboardStatus;
using deskflow::gui::ipc::CoreIpcClient;

namespace {
struct Session
{
  Log log;
  QString name = QStringLiteral("deskflow-clipboard-test-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
  deskflow::core::ipc::CoreIpcServer server{nullptr, name};
  CoreIpcClient client{nullptr, name};
  Session()
  {
    server.listen();
  }
};
} // namespace

void CoreIpcClientTests::setClipboardStatus_allValues_roundTrips_data()
{
  QTest::addColumn<int>("value");
  const auto values = QMetaEnum::fromType<ClipboardStatus>();
  for (int i = 0; i < values.keyCount(); ++i)
    QTest::newRow(values.key(i)) << values.value(i);
}

void CoreIpcClientTests::setClipboardStatus_allValues_roundTrips()
{
  QFETCH(int, value);
  Session session;
  const auto expected = static_cast<ClipboardStatus>(value);
  const auto initial = expected == Reason::Available ? Reason::NotCompiled : Reason::Available;
  session.server.setClipboardStatus(initial);
  session.client.connectToServer();
  QTRY_COMPARE(session.client.clipboardStatus(), initial);
  session.server.setClipboardStatus(expected);
  QTRY_COMPARE(session.client.clipboardStatus(), expected);
}

void CoreIpcClientTests::connectToServer_existingStatus_receivesSnapshot()
{
  Session session;
  session.server.setClipboardStatus(Reason::NotCompiled);
  session.client.connectToServer();
  QTRY_COMPARE(session.client.clipboardStatus(), Reason::NotCompiled);
}

void CoreIpcClientTests::connectToServer_secondClient_receivesSnapshot()
{
  Session session;
  session.client.connectToServer();
  QTRY_VERIFY(session.client.isConnected());
  session.server.setClipboardStatus(Reason::Available);
  QTRY_COMPARE(session.client.clipboardStatus(), Reason::Available);
  CoreIpcClient second(nullptr, session.name);
  second.connectToServer();
  QTRY_COMPARE(second.clipboardStatus(), Reason::Available);
}

void CoreIpcClientTests::setClipboardStatus_changed_receivesUpdate()
{
  Session session;
  session.client.connectToServer();
  QTRY_VERIFY(session.client.isConnected());
  session.server.setClipboardStatus(Reason::SessionNotEnabled);
  QTRY_COMPARE(session.client.clipboardStatus(), Reason::SessionNotEnabled);
}

void CoreIpcClientTests::setClipboardStatus_same_noDuplicateNotification()
{
  Session session;
  session.server.setClipboardStatus(Reason::Available);
  session.client.connectToServer();
  QTRY_COMPARE(session.client.clipboardStatus(), Reason::Available);
  QSignalSpy changed(&session.client, &CoreIpcClient::clipboardStatusChanged);
  session.server.setClipboardStatus(Reason::Available);
  QCoreApplication::processEvents();
  QCOMPARE(changed.count(), 0);
}

void CoreIpcClientTests::disconnectFromServer_available_invalidatesStatus()
{
  Session session;
  session.server.setClipboardStatus(Reason::Available);
  session.client.connectToServer();
  QTRY_COMPARE(session.client.clipboardStatus(), Reason::Available);
  session.client.disconnectFromServer();
  QCOMPARE(session.client.clipboardStatus(), Reason::CoreDisconnected);
}

void CoreIpcClientTests::processCommand_shutdown_invalidatesStatus()
{
  Session session;
  session.server.setClipboardStatus(Reason::Available);
  session.client.connectToServer();
  QTRY_COMPARE(session.client.clipboardStatus(), Reason::Available);
  session.server.broadcastCommand(QStringLiteral("bye"));
  QTRY_COMPARE(session.client.clipboardStatus(), Reason::CoreNotRunning);
}

void CoreIpcClientTests::processCommand_invalid_preservesLastValidStatus()
{
  Session session;
  session.server.setClipboardStatus(Reason::NotCompiled);
  session.client.connectToServer();
  QTRY_COMPARE(session.client.clipboardStatus(), Reason::NotCompiled);
  QTest::ignoreMessage(QtWarningMsg, "core ipc got invalid clipboard status: Available,NotCompiled");
  session.server.broadcastCommand(QStringLiteral("clipboardStatus"), QStringLiteral("Available,NotCompiled"));
  QTest::qWait(20);
  QCOMPARE(session.client.clipboardStatus(), Reason::NotCompiled);
}

QTEST_GUILESS_MAIN(CoreIpcClientTests)

#include "CoreIpcClientTests.moc"
