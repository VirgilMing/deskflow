/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "ClipboardStatusLabel.h"
#include <QEvent>

namespace deskflow::gui {
ClipboardStatusLabel::ClipboardStatusLabel(QWidget *parent) : QLabel(parent)
{
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  setWordWrap(true);
  setTextFormat(Qt::PlainText);
  updateText();
}

void ClipboardStatusLabel::setStatus(ClipboardStatus status)
{
  m_status = status;
  updateText();
}

void ClipboardStatusLabel::setServer(bool server)
{
  m_server = server;
  updateText();
}

void ClipboardStatusLabel::changeEvent(QEvent *pEvent)
{
  QLabel::changeEvent(pEvent);
  if (pEvent->type() == QEvent::LanguageChange)
    updateText();
}

void ClipboardStatusLabel::updateText()
{
  QString text;
  using enum ClipboardStatus;
  switch (m_status) {
  case Available:
    text = tr("Local clipboard support is available");
    break;
  case NotCompiled:
    text = tr("Local clipboard synchronization is unavailable: this build does not include Wayland clipboard support");
    break;
  case PortalUnavailable:
    text = tr("Local clipboard synchronization is unavailable: the clipboard portal could not be accessed");
    break;
  case PortalVersionUnsupported:
    text =
        tr("Local clipboard synchronization is unavailable: the input capture portal version does not support "
           "clipboard sharing");
    break;
  case SessionNotEnabled:
    text = tr(
        "Local clipboard synchronization is unavailable: clipboard access is not enabled for the current portal session"
    );
    break;
  case WaitingForSession:
    text = tr("Checking local clipboard support...");
    break;
  case CoreNotRunning:
    text = tr("Start Deskflow to check local clipboard support");
    break;
  case CoreDisconnected:
    text = tr("Local clipboard status is unknown: connection to the core process was lost");
    break;
  case NotApplicable:
    break;
  }
  if (m_server && clipboardUnavailable(m_status)) {
    text += '\n';
    text +=
        tr("With clipboard sharing enabled and a size limit greater than zero, this server can still forward clipboard "
           "data between clients that support clipboard sharing.");
  }
  setText(text);
  setVisible(m_status != NotApplicable);
}
} // namespace deskflow::gui
