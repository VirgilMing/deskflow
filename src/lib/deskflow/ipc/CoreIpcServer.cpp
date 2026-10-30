/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-FileCopyrightText: (C) 2025-2026 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "CoreIpcServer.h"

#include "base/Log.h"
#include "common/Constants.h"

#include <QLocalSocket>
#include <QMetaEnum>
#include <utility>

namespace deskflow::core::ipc {

static CoreIpcServer *s_instance = nullptr;

CoreIpcServer::CoreIpcServer(QObject *parent, const QString &serverName)
    : IpcServer(parent, serverName.isEmpty() ? kCoreIpcName : serverName, QStringLiteral("core"))
{
  assert(s_instance == nullptr);
  s_instance = this;
}

CoreIpcServer::~CoreIpcServer()
{
  s_instance = nullptr;
}

void CoreIpcServer::setClipboardStatus(ClipboardStatus status)
{
  if (m_clipboardStatus == status)
    return;
  m_clipboardStatus = status;
  for (auto *pSocket : std::as_const(m_clipboardSubscribers))
    sendClipboardStatus(pSocket);
}

void CoreIpcServer::sendClipboardStatus(QLocalSocket *pSocket)
{
  if (pSocket->state() != QLocalSocket::ConnectedState)
    return;
  const auto name = QMetaEnum::fromType<ClipboardStatus>().valueToKey(static_cast<int>(m_clipboardStatus));
  writeToClientSocket(pSocket, QStringLiteral("clipboardStatus=%1").arg(name));
  pSocket->flush();
}

CoreIpcServer &CoreIpcServer::instance()
{
  assert(s_instance != nullptr);
  return *s_instance;
}

void CoreIpcServer::processCommand(QLocalSocket *clientSocket, const QString &command, const QStringList &parts)
{
  Q_UNUSED(parts)
  if (command == QStringLiteral("getClipboardStatus")) {
    if (!m_clipboardSubscribers.contains(clientSocket)) {
      m_clipboardSubscribers.insert(clientSocket);
      connect(clientSocket, &QObject::destroyed, this, [this, clientSocket] {
        m_clipboardSubscribers.remove(clientSocket);
      });
    }
    sendClipboardStatus(clientSocket);
    return;
  }
  if (command == QStringLiteral("stop")) {
    LOG_DEBUG("core ipc server got stop message");
    writeToClientSocket(clientSocket, QStringLiteral("ok"));
    broadcastCommand(QStringLiteral("bye"));
    Q_EMIT stopProcessRequested();
    return;
  }
  LOG_WARN("core ipc server got unknown command: %s", command.toUtf8().constData());
}

} // namespace deskflow::core::ipc
