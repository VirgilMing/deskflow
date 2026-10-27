/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-FileCopyrightText: (C) 2025-2026 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "CoreIpcClient.h"

#include "common/Constants.h"

#include <QMetaEnum>
#include <QString>
#include <utility>

namespace deskflow::gui::ipc {

CoreIpcClient::CoreIpcClient(QObject *parent, const QString &socketName)
    : IpcClient(parent, socketName.isEmpty() ? kCoreIpcName : socketName, QStringLiteral("core"))
{
  m_reconnectTimer.setSingleShot(true);
  connect(&m_reconnectTimer, &QTimer::timeout, this, &IpcClient::connectToServer);
  connect(this, &IpcClient::connected, this, [this] {
    m_followConnection = true;
    sendMessage(QStringLiteral("getClipboardStatus"));
  });
  connect(this, &IpcClient::connectionFailed, this, [this] {
    setClipboardStatus(ClipboardStatus::CoreDisconnected);
    if (std::exchange(m_followConnection, false))
      m_reconnectTimer.start(500);
  });
  connect(this, &IpcClient::serverShutdown, this, [this] {
    m_followConnection = false;
    m_reconnectTimer.stop();
    setClipboardStatus(ClipboardStatus::CoreNotRunning);
  });
}

void CoreIpcClient::disconnectFromServer()
{
  m_followConnection = false;
  m_reconnectTimer.stop();
  IpcClient::disconnectFromServer();
  setClipboardStatus(ClipboardStatus::CoreDisconnected);
}

void CoreIpcClient::setClipboardStatus(ClipboardStatus status)
{
  if (m_clipboardStatus == status)
    return;
  m_clipboardStatus = status;
  Q_EMIT clipboardStatusChanged(status);
}

void CoreIpcClient::sendStop()
{
  sendMessage(QStringLiteral("stop"));
}

void CoreIpcClient::processCommand(const QString &command, const QStringList &parts)
{
  const auto args = parts.size() >= 2 ? parts.at(1) : QString();
  if (command == QStringLiteral("clipboardStatus")) {
    bool valid = false;
    const auto value = QMetaEnum::fromType<ClipboardStatus>().keyToValue(args.toUtf8().constData(), &valid);
    if (valid && parts.size() == 2)
      setClipboardStatus(static_cast<ClipboardStatus>(value));
    else
      qWarning("core ipc got invalid clipboard status: %s", qPrintable(args));
    return;
  }
  Q_EMIT commandReceived(command, args);
}

} // namespace deskflow::gui::ipc
