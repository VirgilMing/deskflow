/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "base/Log.h"
#include "deskflow/ipc/CoreIpcServer.h"
#include "gui/core/CoreProcess.h"
#include "gui/ipc/CoreIpcClient.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>

#include <memory>

namespace deskflow::gui {

class CoreProcessTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void initTestCase();
  void checkExistingProcess_matchingCore_observesWithoutTakingOwnership();
  void stop_observedCore_doesNotEnterStoppingOrSendStop();
  void serverShutdown_observedCore_clearsStatusWithoutChangingProcessState();
  void setProcessState_ownedCoreStopped_invalidatesStatus();
  void setProcessState_ownedCoreStopped_cancelsPendingConnection();
  void setProcessState_ownedCoreStopping_keepsControlConnection();
  void observeClipboardStatus_replaced_ignoresOldSource();
  void connectionFailed_observedCore_recoversSnapshot();
  void setConnectionState_remoteDisconnected_retainsLocalStatus();

private:
  QTemporaryDir m_settings;
};

using enum ClipboardStatus;

namespace {
struct Session
{
  Log log;
  QString name = QStringLiteral("deskflow-process-test-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
  core::ipc::CoreIpcServer server{nullptr, name};
  ServerConfig config;
  std::unique_ptr<CoreProcess> process;
  Session()
  {
    server.listen();
    QTest::ignoreMessage(QtCriticalMsg, "core server binary does not exist");
    process = std::make_unique<CoreProcess>(config, name);
  }
};
} // namespace

void CoreProcessTests::initTestCase()
{
  Settings::setSettingsFile(m_settings.filePath("settings.ini"));
  Settings::setStateFile(m_settings.filePath("state.ini"));
}

void CoreProcessTests::checkExistingProcess_matchingCore_observesWithoutTakingOwnership()
{
  Session session;
  session.server.setClipboardStatus(NotCompiled);
  session.process->checkExistingProcess();
  QTRY_COMPARE(session.process->localClipboardStatus(), NotCompiled);
  QSignalSpy stateChanged(session.process.get(), &CoreProcess::processStateChanged);
  session.server.setClipboardStatus(Available);
  QTRY_COMPARE(session.process->localClipboardStatus(), Available);
  QVERIFY(session.process->processState() == core::ProcessState::Stopped && stateChanged.isEmpty());
}

void CoreProcessTests::stop_observedCore_doesNotEnterStoppingOrSendStop()
{
  Session session;
  session.process->m_process = new QProcess(session.process.get());
  session.server.setClipboardStatus(Available);
  session.process->checkExistingProcess();
  QTRY_COMPARE(session.process->localClipboardStatus(), Available);
  QSignalSpy stopRequested(&session.server, &core::ipc::IpcServer::stopProcessRequested);
  QTest::ignoreMessage(QtWarningMsg, "core process already stopped");
  session.process->stop(Settings::ProcessMode::Desktop);
  QCoreApplication::processEvents();
  QVERIFY(
      session.process->processState() == core::ProcessState::Stopped && stopRequested.isEmpty() &&
      session.process->localClipboardStatus() == Available
  );
}

void CoreProcessTests::serverShutdown_observedCore_clearsStatusWithoutChangingProcessState()
{
  Session session;
  session.server.setClipboardStatus(Available);
  session.process->checkExistingProcess();
  QTRY_COMPARE(session.process->localClipboardStatus(), Available);
  session.server.broadcastCommand(QStringLiteral("bye"));
  QTRY_COMPARE(session.process->localClipboardStatus(), CoreNotRunning);
  QCOMPARE(session.process->processState(), core::ProcessState::Stopped);
}

void CoreProcessTests::setProcessState_ownedCoreStopped_invalidatesStatus()
{
  Session session;
  session.process->m_coreIpcClient = new ipc::CoreIpcClient(session.process.get(), session.name);
  session.process->observeClipboardStatus(session.process->m_coreIpcClient);
  session.process->setProcessState(core::ProcessState::Started);
  session.server.setClipboardStatus(Available);
  session.process->m_coreIpcClient->connectToServer();
  QTRY_COMPARE(session.process->localClipboardStatus(), Available);
  session.process->setProcessState(core::ProcessState::Stopped);
  QCOMPARE(session.process->localClipboardStatus(), CoreNotRunning);
}

void CoreProcessTests::setProcessState_ownedCoreStopped_cancelsPendingConnection()
{
  Session session;
  const auto name = session.name + QStringLiteral("-delayed");
  session.process->m_coreIpcClient = new ipc::CoreIpcClient(session.process.get(), name);
  session.process->observeClipboardStatus(session.process->m_coreIpcClient);
  session.process->setProcessState(core::ProcessState::Started);
  session.process->m_coreIpcClient->connectToServer();
  QTest::qWait(20);
  session.process->setProcessState(core::ProcessState::Stopped);
  QLocalServer delayedServer;
  QVERIFY(delayedServer.listen(name));
  QSignalSpy connected(&delayedServer, &QLocalServer::newConnection);
  QTest::qWait(600);
  QCOMPARE(connected.count(), 0);
}

void CoreProcessTests::setProcessState_ownedCoreStopping_keepsControlConnection()
{
  Session session;
  session.process->m_coreIpcClient = new ipc::CoreIpcClient(session.process.get(), session.name);
  session.process->observeClipboardStatus(session.process->m_coreIpcClient);
  session.process->setProcessState(core::ProcessState::Started);
  session.server.setClipboardStatus(Available);
  session.process->m_coreIpcClient->connectToServer();
  QTRY_COMPARE(session.process->localClipboardStatus(), Available);
  session.process->setProcessState(core::ProcessState::Stopping);
  QVERIFY(session.process->localClipboardStatus() == CoreNotRunning && session.process->m_coreIpcClient->isConnected());
}

void CoreProcessTests::observeClipboardStatus_replaced_ignoresOldSource()
{
  Session session;
  session.server.setClipboardStatus(NotCompiled);
  session.process->checkExistingProcess();
  QTRY_COMPARE(session.process->localClipboardStatus(), NotCompiled);
  auto *pPrevious = session.process->m_pClipboardObserver;
  auto *pNext = new ipc::CoreIpcClient(session.process.get(), session.name);
  session.process->observeClipboardStatus(pNext);
  QSignalSpy changed(session.process.get(), &CoreProcess::localClipboardStatusChanged);
  Q_EMIT pPrevious->clipboardStatusChanged(Available);
  QCOMPARE(changed.count(), 0);
}

void CoreProcessTests::connectionFailed_observedCore_recoversSnapshot()
{
  Session session;
  session.server.setClipboardStatus(NotCompiled);
  session.process->checkExistingProcess();
  QTRY_COMPARE(session.process->localClipboardStatus(), NotCompiled);
  for (auto *pPeer : session.server.findChildren<QLocalSocket *>())
    pPeer->abort();
  session.server.setClipboardStatus(Available);
  QTRY_COMPARE(session.process->localClipboardStatus(), Available);
}

void CoreProcessTests::setConnectionState_remoteDisconnected_retainsLocalStatus()
{
  Session session;
  session.server.setClipboardStatus(Available);
  session.process->checkExistingProcess();
  QTRY_COMPARE(session.process->localClipboardStatus(), Available);
  session.process->setConnectionState(core::ConnectionState::Connected);
  session.process->setConnectionState(core::ConnectionState::Disconnected);
  QCOMPARE(session.process->localClipboardStatus(), Available);
}

} // namespace deskflow::gui

QTEST_MAIN(deskflow::gui::CoreProcessTests)
#include "CoreProcessTests.moc"
