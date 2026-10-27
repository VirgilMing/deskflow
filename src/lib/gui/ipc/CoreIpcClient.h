/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-FileCopyrightText: (C) 2025-2026 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "IpcClient.h"
#include "common/Enums.h"

#include <QObject>
#include <QTimer>

namespace deskflow::gui::ipc {

class CoreIpcClient : public IpcClient
{
  Q_OBJECT

public:
  explicit CoreIpcClient(QObject *parent = nullptr, const QString &socketName = {});

  void sendStop();
  void disconnectFromServer();
  ClipboardStatus clipboardStatus() const
  {
    return m_clipboardStatus;
  }

Q_SIGNALS:
  void commandReceived(const QString &command, const QString &args);
  void clipboardStatusChanged(deskflow::ClipboardStatus status);

protected:
  void processCommand(const QString &command, const QStringList &parts) override;

private:
  void setClipboardStatus(ClipboardStatus status);
  QTimer m_reconnectTimer;
  bool m_followConnection = false;
  ClipboardStatus m_clipboardStatus{ClipboardStatus::WaitingForSession};
};

} // namespace deskflow::gui::ipc
