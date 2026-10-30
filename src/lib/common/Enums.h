/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 - 2026 Deskflow Developers
 * SPDX-FileCopyrightText: (C) 2010 - 2018, 2024 - 2025 Synergy App Ltd
 * SPDX-FileCopyrightText: (C) 2002 - 2007 Chris Schoeneman
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include <QObject>

namespace deskflow::client {
Q_NAMESPACE
enum class ErrorType : uint8_t
{
  NoError,
  AlreadyConnected,
  HostnameError,
  GenericError
};
Q_ENUM_NS(ErrorType)
} // namespace deskflow::client

namespace deskflow::core {
Q_NAMESPACE

enum class ProcessState
{
  Starting,
  Started,
  Stopping,
  Stopped,
  RetryPending
};
Q_ENUM_NS(ProcessState)

enum class ConnectionState
{
  Disconnected,
  Connecting,
  Connected,
  Listening
};
Q_ENUM_NS(ConnectionState)

enum class ConnectionRefusal
{
  IncompatibleVersion,
  AlreadyConnected,
  UnknownClient,
  ProtocolError
};
Q_ENUM_NS(ConnectionRefusal)

} // namespace deskflow::core

namespace deskflow {
Q_NAMESPACE

enum class ClipboardStatus
{
  Available,
  WaitingForSession,
  NotCompiled,
  PortalUnavailable,
  PortalVersionUnsupported,
  SessionNotEnabled,
  CoreNotRunning,
  CoreDisconnected,
  NotApplicable
};
Q_ENUM_NS(ClipboardStatus)

constexpr bool clipboardUnavailable(ClipboardStatus status)
{
  using enum ClipboardStatus;
  return status == NotCompiled || status == PortalUnavailable || status == PortalVersionUnsupported ||
         status == SessionNotEnabled;
}
} // namespace deskflow
