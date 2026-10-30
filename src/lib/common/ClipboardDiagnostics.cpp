/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "ClipboardDiagnostics.h"

namespace deskflow {

ClipboardDiagnostics::Update ClipboardDiagnostics::setStatus(ClipboardStatus status)
{
  const auto changed = m_status != status;
  m_status = status;
  return evaluate(changed);
}

ClipboardDiagnostics::Update ClipboardDiagnostics::setSharing(std::optional<bool> enabled)
{
  m_sharingEnabled = enabled.value_or(m_sharingEnabled.value_or(true));
  return evaluate();
}

ClipboardDiagnostics::Update ClipboardDiagnostics::evaluate(bool statusChanged)
{
  Update update{.statusChanged = statusChanged};
  if (m_sharingEnabled == false || m_status == ClipboardStatus::Available)
    m_warnedUnavailable.reset();

  if (m_sharingEnabled != true)
    return update;

  if (clipboardUnavailable(m_status) && m_warnedUnavailable != m_status) {
    update.warnUnavailable = true;
    m_warnedUnavailable = m_status;
  }
  return update;
}

} // namespace deskflow
