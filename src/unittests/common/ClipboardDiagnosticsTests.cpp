/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "common/ClipboardDiagnostics.h"
#include <QTest>

class ClipboardDiagnosticsTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void setSharing_conditions_warnsOnlyWhenEnabled_data();
  void setSharing_conditions_warnsOnlyWhenEnabled();
  void setStatus_beforeOptions_doesNotWarn();
  void setStatus_duplicate_noWarning();
  void setSharing_duplicate_noWarning();
  void setStatus_retry_noDuplicateWarning();
  void setStatus_recoveredThenUnavailable_warns();
  void setSharing_reenabled_warns();
  void setSharing_missingOption_preservesDisabledSetting();
  void setSharing_missingInitialOption_usesProtocolDefault();
  void setSharing_resetThenSameOptions_noDuplicateWarning();
};

using deskflow::ClipboardDiagnostics;
using enum deskflow::ClipboardStatus;

void ClipboardDiagnosticsTests::setSharing_conditions_warnsOnlyWhenEnabled_data()
{
  QTest::addColumn<bool>("enabled");
  QTest::addColumn<bool>("unavailable");
  QTest::addColumn<bool>("warning");
  QTest::newRow("enabled, unavailable") << true << true << true;
  QTest::newRow("disabled, unavailable") << false << true << false;
  QTest::newRow("enabled, available") << true << false << false;
}

void ClipboardDiagnosticsTests::setSharing_conditions_warnsOnlyWhenEnabled()
{
  QFETCH(bool, enabled);
  QFETCH(bool, unavailable);
  QFETCH(bool, warning);
  ClipboardDiagnostics diagnostics;
  diagnostics.setStatus(unavailable ? NotCompiled : Available);
  const auto update = diagnostics.setSharing(enabled);
  QCOMPARE(update, (ClipboardDiagnostics::Update{false, warning}));
}

void ClipboardDiagnosticsTests::setStatus_beforeOptions_doesNotWarn()
{
  ClipboardDiagnostics diagnostics;
  const auto update = diagnostics.setStatus(NotCompiled);
  QCOMPARE(update, (ClipboardDiagnostics::Update{true, false}));
}

void ClipboardDiagnosticsTests::setStatus_duplicate_noWarning()
{
  ClipboardDiagnostics diagnostics;
  diagnostics.setSharing(true);
  diagnostics.setStatus(SessionNotEnabled);
  const auto update = diagnostics.setStatus(SessionNotEnabled);
  QCOMPARE(update, ClipboardDiagnostics::Update{});
}

void ClipboardDiagnosticsTests::setSharing_duplicate_noWarning()
{
  ClipboardDiagnostics diagnostics;
  diagnostics.setStatus(NotCompiled);
  diagnostics.setSharing(true);
  const auto update = diagnostics.setSharing(true);
  QCOMPARE(update, ClipboardDiagnostics::Update{});
}

void ClipboardDiagnosticsTests::setStatus_retry_noDuplicateWarning()
{
  ClipboardDiagnostics diagnostics;
  diagnostics.setSharing(true);
  diagnostics.setStatus(SessionNotEnabled);
  diagnostics.setStatus(WaitingForSession);
  const auto update = diagnostics.setStatus(SessionNotEnabled);
  QCOMPARE(update, (ClipboardDiagnostics::Update{true, false}));
}

void ClipboardDiagnosticsTests::setStatus_recoveredThenUnavailable_warns()
{
  ClipboardDiagnostics diagnostics;
  diagnostics.setSharing(true);
  diagnostics.setStatus(SessionNotEnabled);
  diagnostics.setStatus(Available);
  const auto update = diagnostics.setStatus(SessionNotEnabled);
  QCOMPARE(update, (ClipboardDiagnostics::Update{true, true}));
}

void ClipboardDiagnosticsTests::setSharing_reenabled_warns()
{
  ClipboardDiagnostics diagnostics;
  diagnostics.setStatus(NotCompiled);
  diagnostics.setSharing(true);
  diagnostics.setSharing(false);
  const auto update = diagnostics.setSharing(true);
  QCOMPARE(update, (ClipboardDiagnostics::Update{false, true}));
}

void ClipboardDiagnosticsTests::setSharing_missingOption_preservesDisabledSetting()
{
  ClipboardDiagnostics diagnostics;
  diagnostics.setStatus(NotCompiled);
  diagnostics.setSharing(false);
  const auto update = diagnostics.setSharing(std::nullopt);
  QCOMPARE(update, ClipboardDiagnostics::Update{});
}

void ClipboardDiagnosticsTests::setSharing_missingInitialOption_usesProtocolDefault()
{
  ClipboardDiagnostics diagnostics;
  diagnostics.setStatus(NotCompiled);
  const auto update = diagnostics.setSharing(std::nullopt);
  QCOMPARE(update, (ClipboardDiagnostics::Update{false, true}));
}

void ClipboardDiagnosticsTests::setSharing_resetThenSameOptions_noDuplicateWarning()
{
  ClipboardDiagnostics diagnostics;
  diagnostics.setStatus(NotCompiled);
  diagnostics.setSharing(true);
  diagnostics.resetSharing();
  const auto update = diagnostics.setSharing(std::nullopt);
  QCOMPARE(update, ClipboardDiagnostics::Update{});
}

QTEST_GUILESS_MAIN(ClipboardDiagnosticsTests)
#include "ClipboardDiagnosticsTests.moc"
