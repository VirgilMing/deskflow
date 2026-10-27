/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "common/Enums.h"
#include <QLabel>

namespace deskflow::gui {
class ClipboardStatusLabel : public QLabel
{
  Q_OBJECT
public:
  explicit ClipboardStatusLabel(QWidget *parent = nullptr);
  void setStatus(ClipboardStatus status);
  void setServer(bool server);

protected:
  void changeEvent(QEvent *pEvent) override;

private:
  void updateText();
  ClipboardStatus m_status = ClipboardStatus::CoreNotRunning;
  bool m_server = false;
};
} // namespace deskflow::gui
