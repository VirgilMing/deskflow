/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-FileCopyrightText: (C) 2025-2026 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "IpcServer.h"
#include "common/Enums.h"

#include <QObject>
#include <QSet>

class QLocalSocket;

namespace deskflow::core::ipc {

class CoreIpcServer : public IpcServer
{
  Q_OBJECT

public:
  explicit CoreIpcServer(QObject *parent, const QString &serverName = {});
  ~CoreIpcServer() override;

  void setClipboardStatus(ClipboardStatus status);

  static CoreIpcServer &instance();

private:
  ClipboardStatus m_clipboardStatus{ClipboardStatus::NotApplicable};
  QSet<QLocalSocket *> m_clipboardSubscribers;
  void sendClipboardStatus(QLocalSocket *pSocket);
  void processCommand(QLocalSocket *clientSocket, const QString &command, const QStringList &parts) override;
};

} // namespace deskflow::core::ipc
