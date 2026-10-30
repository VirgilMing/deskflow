/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "common/Settings.h"
#include "gui/dialogs/ServerConfigDialog.h"
#include "gui/widgets/ClipboardStatusLabel.h"

#include <QCheckBox>
#include <QTemporaryDir>
#include <QTest>

class ClipboardStatusLabelTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void initTestCase();
  void setStatus_notCompiled_explainsMissingBuildSupport();
  void setStatus_serverUnavailable_showsWrappedForwardingInfo();
  void setStatus_clientUnavailable_omitsForwardingInfo();
  void setStatus_available_omitsForwardingInfo();
  void setStatus_notApplicable_hidesDiagnostic();
  void setClipboardStatus_serverUnavailable_preservesGlobalSwitch_data();
  void setClipboardStatus_serverUnavailable_preservesGlobalSwitch();

private:
  QTemporaryDir m_settings;
};

using deskflow::ClipboardStatus;
using deskflow::gui::ClipboardStatusLabel;
using enum deskflow::ClipboardStatus;

void ClipboardStatusLabelTests::initTestCase()
{
  Settings::setSettingsFile(m_settings.filePath("settings.ini"));
  Settings::setStateFile(m_settings.filePath("state.ini"));
}

void ClipboardStatusLabelTests::setStatus_notCompiled_explainsMissingBuildSupport()
{
  ClipboardStatusLabel label;
  label.setStatus(NotCompiled);
  QVERIFY(label.text().contains("does not include Wayland clipboard support"));
}

void ClipboardStatusLabelTests::setStatus_serverUnavailable_showsWrappedForwardingInfo()
{
  ClipboardStatusLabel label;
  label.setServer(true);
  label.setStatus(SessionNotEnabled);
  QVERIFY(
      label.wordWrap() && label.text().contains("\nWith clipboard sharing enabled and a size limit greater than zero")
  );
}

void ClipboardStatusLabelTests::setStatus_clientUnavailable_omitsForwardingInfo()
{
  ClipboardStatusLabel label;
  label.setStatus(NotCompiled);
  QVERIFY(!label.text().contains("forward"));
}

void ClipboardStatusLabelTests::setStatus_available_omitsForwardingInfo()
{
  ClipboardStatusLabel label;
  label.setServer(true);
  label.setStatus(NotCompiled);
  label.setStatus(Available);
  QVERIFY(!label.text().contains("forward"));
}

void ClipboardStatusLabelTests::setStatus_notApplicable_hidesDiagnostic()
{
  ClipboardStatusLabel label;
  label.setStatus(NotApplicable);
  QVERIFY(label.isHidden());
}

void ClipboardStatusLabelTests::setClipboardStatus_serverUnavailable_preservesGlobalSwitch_data()
{
  QTest::addColumn<bool>("enabled");
  QTest::newRow("enabled") << true;
  QTest::newRow("disabled") << false;
}

void ClipboardStatusLabelTests::setClipboardStatus_serverUnavailable_preservesGlobalSwitch()
{
  QFETCH(bool, enabled);
  Settings::setValue(Settings::Server::EnableClipboard, enabled);
  ServerConfig config;
  ServerConfigDialog dialog(nullptr, config);
  dialog.setClipboardStatus(NotCompiled);
  const auto *checkbox = dialog.findChild<QCheckBox *>("cbEnableClipboard");
  QVERIFY(checkbox->isEnabled() && checkbox->isChecked() == enabled);
}

QTEST_MAIN(ClipboardStatusLabelTests)

#include "ClipboardStatusLabelTests.moc"
