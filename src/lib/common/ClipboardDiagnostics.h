/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "common/Enums.h"

#include <optional>

namespace deskflow {

// Tracks capability warnings independently of asynchronous status and sharing-option updates.
class ClipboardDiagnostics
{
public:
  struct Update
  {
    bool statusChanged = false;
    bool warnUnavailable = false;
    bool operator==(const Update &) const = default;
  };

  const ClipboardStatus &status() const
  {
    return m_status;
  }
  Update setStatus(ClipboardStatus status);
  // An omitted option preserves the current setting, or uses the protocol default after reset.
  Update setSharing(std::optional<bool> enabled);
  void resetSharing()
  {
    m_sharingEnabled.reset();
  }

private:
  Update evaluate(bool statusChanged = false);

  ClipboardStatus m_status = ClipboardStatus::WaitingForSession;
  std::optional<bool> m_sharingEnabled;
  std::optional<ClipboardStatus> m_warnedUnavailable;
};

} // namespace deskflow
