#include <QFileDialog>
#include <QStandardPaths>

#include <QListView>
#include <QMessageBox>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QDebug>
#include <QCompleter>

#include "Commons.h"
#include "DlgSettings.h"
#include "NotificationObserver.h"
#include "RhController.h"
#include "OsBranchConsts.h"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"
#include "ui_DlgSettings.h"
#include "Logger.h"
#include "LanguageController.h"
#include "TraySkinController.h"
#include "VagrantProvider.h"
#include "PeerController.h"

static void fill_log_level_combobox(QComboBox* cb) {
  for (int i = 0; i <= Logger::LOG_DISABLED; ++i)
    cb->addItem(Logger::LogLevelToStr((Logger::LOG_LEVEL)i));
}

//////////////////////////////////////////////////////////////////////////

static void fill_tray_skin_combobox(QComboBox* cb) {
  for (int i = 0; i < TraySkinController::SKIN_NUMBER; ++i)
    cb->addItem(TraySkinController::tray_skin_to_str((TraySkinController::TRAY_SKINS)i));
}
//////////////////////////////////////////////////////////////////////////

static void fill_locale_combobox(QComboBox* cb) {
  for (int i = 0; i <= LanguageController::LOCALE_LAST; ++i)
    cb->addItem(LanguageController::LocaleTypeToStr((LanguageController::LOCALE_TYPE)i));
}
//////////////////////////////////////////////////////////////////////////

static void fill_vagrant_provider_combobox(QComboBox* cb) {
  std::vector<int> providers_by_os = VagrantProvider::Instance()->List();

  for (int i = 0; i < (int)providers_by_os.size(); i++) {
    cb->addItem(VagrantProvider::ProviderToStr((VagrantProvider::PROVIDERS)providers_by_os.at(i)),
                QVariant(providers_by_os.at(i)));
  }
}
//////////////////////////////////////////////////////////////////////////

static void fill_notifications_level_combobox(QComboBox* cb) {
  for (int i = 0; i <= CNotificationObserver::NL_CRITICAL; ++i)
    cb->addItem(CNotificationObserver::notification_level_to_str(
                  (CNotificationObserver::notification_level_t)i));
}
////////////////////////////////////////////////////////////////////////////

static void fill_freq_combobox(QComboBox* cb) {
  for (int i = 0; i < CSettingsManager::UF_LAST; ++i)
    cb->addItem(CSettingsManager::update_freq_to_str(
                  (CSettingsManager::update_freq_t)i));
}
////////////////////////////////////////////////////////////////////////////

static void fill_preferred_notifications_location_combobox(QComboBox* cb) {
  for (int i = 0; i < CNotificationObserver::NPP_LAST; ++i)
    cb->addItem(CNotificationObserver::notifications_preffered_place_to_str(
                  (CNotificationObserver::notification_preffered_place_t)i));
}
////////////////////////////////////////////////////////////////////////////

DlgSettings::DlgSettings(QWidget* parent)
  : QDialog(parent),
    ui(new Ui::DlgSettings),
    m_tab_resize_filter(nullptr),
    m_model_resource_hosts(nullptr),
    m_refresh_rh_list_progress_val(0) {
  ui->setupUi(this);
  ui->sb_refresh_timeout->setValue(
        CSettingsManager::Instance().refresh_time_sec());
  ui->le_p2p_command->setText(CSettingsManager::Instance().p2p_path());
  ui->le_hypervisor_command->setText(CSettingsManager::Instance().current_hypervisor_path());
  ui->le_vagrant_command->setText(CSettingsManager::Instance().vagrant_path());
  ui->sb_notification_delay->setMinimum(
        CSettingsManager::NOTIFICATION_DELAY_MIN);
  ui->sb_notification_delay->setMaximum(
        CSettingsManager::NOTIFICATION_DELAY_MAX);
  ui->sb_notification_delay->setValue(
        CSettingsManager::Instance().notification_delay_sec());
  ui->le_ssh_command->setText(CSettingsManager::Instance().ssh_path());
  ui->le_scp_command->setText(CSettingsManager::Instance().scp_path());

  ui->le_x2goclient_command->setText(CSettingsManager::Instance().x2goclient());

  ui->le_ssh_user->setText(CSettingsManager::Instance().ssh_user());
  ui->le_rhip_host->setText(CSettingsManager::Instance().rh_host());
  ui->le_rhip_password->setText(CSettingsManager::Instance().rh_pass());
  ui->le_rhip_port->setText(
        QString("%1").arg(CSettingsManager::Instance().rh_port()));
  ui->le_rhip_user->setText(CSettingsManager::Instance().rh_user());
  ui->le_logs_storage->setText(CSettingsManager::Instance().logs_storage());
  ui->le_ssh_keys_storage->setText(
        CSettingsManager::Instance().ssh_keys_storage());
  ui->le_ssh_keygen_command->setText(
        CSettingsManager::Instance().ssh_keygen_cmd());
  ui->lbl_hypervisor_command->setText(VagrantProvider::Instance()->CurrentOpenFileTitle());

  ui->lbl_err_logs_storage->hide();
  ui->lbl_err_ssh_keys_storage->hide();
  ui->lbl_err_vm_storage->hide();
  ui->lbl_err_ssh_user->hide();
  ui->lbl_err_p2p_command->hide();
  ui->lbl_err_x2goclient_command->hide();
  ui->lbl_err_ssh_command->hide();
  ui->lbl_err_vagrant_command->hide();
  ui->lbl_err_ssh_keygen_command->hide();
  ui->lbl_err_scp_command->hide();
  ui->lbl_err_terminal_arg->hide();
  ui->lbl_err_terminal_cmd->hide();
  ui->lbl_err_rhip_host->hide();
  ui->lbl_err_resource_hosts->hide();
  ui->lbl_err_rhip_port->hide();
  ui->lbl_err_rhip_user->hide();
  ui->lbl_err_rhip_password->hide();

  fill_freq_combobox(ui->cb_p2p_frequency);
  fill_freq_combobox(ui->cb_tray_frequency);
  fill_notifications_level_combobox(ui->cb_notification_level);
  fill_log_level_combobox(ui->cb_log_level);
  fill_tray_skin_combobox(ui->cb_tray_skin);
  fill_vagrant_provider_combobox(ui->cb_vagrant_provider);

  fill_locale_combobox(ui->cb_locale);

  ui->cb_locale->setVisible(true);
  ui->lbl_locale->setVisible(true);

  fill_preferred_notifications_location_combobox(
        ui->cb_preferred_notifications_place);

  ui->cb_p2p_frequency->setCurrentIndex(CSettingsManager::Instance().p2p_update_freq());
  ui->cb_tray_frequency->setCurrentIndex(CSettingsManager::Instance().tray_update_freq());
  ui->cb_notification_level->setCurrentIndex(CSettingsManager::Instance().notifications_level());
  ui->cb_log_level->setCurrentIndex(CSettingsManager::Instance().logs_level());
  ui->cb_tray_skin->setCurrentIndex(CSettingsManager::Instance().tray_skin());
  ui->cb_vagrant_provider->setCurrentText(VagrantProvider::Instance()->CurrentStr());

  ui->cb_locale->setCurrentIndex(CSettingsManager::Instance().locale());

  ui->cb_preferred_notifications_place->setCurrentIndex(
        CSettingsManager::Instance().preferred_notifications_place());

  ui->chk_p2p_autoupdate->setChecked(CSettingsManager::Instance().p2p_autoupdate());
  ui->chk_tray_autoupdate->setChecked(CSettingsManager::Instance().tray_autoupdate());

#ifndef RT_OS_DARWIN
  m_tab_resize_filter = new TabResizeFilter(ui->tabWidget);
  ui->tabWidget->installEventFilter(m_tab_resize_filter);
#endif

  ui->le_terminal_cmd->setText(CSettingsManager::Instance().terminal_cmd());
  ui->le_terminal_arg->setText(CSettingsManager::Instance().terminal_arg());

#ifndef RT_OS_WINDOWS
  QStringList terminalLists = CCommons::SupportTerminals();
  QCompleter *completer = new QCompleter(terminalLists, this);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  ui->le_terminal_cmd->setCompleter(completer);
  ui->gb_terminal_settings->setVisible(true);
#endif

  m_model_resource_hosts = new QStandardItemModel(this);
  ui->lstv_resource_hosts->setModel(m_model_resource_hosts);

  ui->pb_refresh_rh_list->setMinimum(0);
  ui->pb_refresh_rh_list->setMaximum(CRhController::REFRESH_DELAY_SEC);
  ui->pb_refresh_rh_list->setValue(m_refresh_rh_list_progress_val);
  m_refresh_rh_list_timer.setInterval(1000);

  ui->chk_use_animations->setChecked(
        CSettingsManager::Instance().use_animations());
  ui->chk_autostart->setChecked(CSettingsManager::Instance().autostart());

  QStringList supported_browsers_list = supported_browsers();
  current_browser = CSettingsManager::Instance().default_browser();
  ui->cb_browser->addItems(supported_browsers_list);
  for (int i = 0; i < supported_browsers_list.size(); i++){
      if(supported_browsers_list[i] == current_browser){
          ui->cb_browser->setCurrentIndex(i);
      }
  }

  if (CSettingsManager::Instance().default_browser() == "Chrome") {
    std::pair<QStringList, QStringList> chrome_profiles_list = chrome_profiles();
    current_chrome_profile = CSettingsManager::Instance().default_chrome_profile();
    ui->cb_profile->addItems(chrome_profiles_list.second);
    for (int i = 0; i < chrome_profiles_list.first.size(); i++){
      if (chrome_profiles_list.first[i] == current_chrome_profile){
        ui->cb_profile->setCurrentIndex(i);
      }
    }
  } else if (CSettingsManager::Instance().default_browser() == "Firefox") {
    QStringList firefox_profiles_list = firefox_profiles().first;
    current_firefox_profile = CSettingsManager::Instance().default_firefox_profile();
    ui->cb_profile->addItems(firefox_profiles_list);
    for (int i = 0; i < firefox_profiles_list.size(); i++) {
      if (firefox_profiles_list[i] == current_firefox_profile) {
        ui->cb_profile->setCurrentIndex(i);
      }
    }
  }

  switch(VagrantProvider::Instance()->CurrentProvider()) {
  case VagrantProvider::VMWARE_DESKTOP:
    ui->le_vm_storage->setText(CSettingsManager::Instance().vmware_vm_storage());
    break;
  case VagrantProvider::VIRTUALBOX:
    ui->le_vm_storage->setText(CSystemCallWrapper::get_virtualbox_vm_storage());
    break;
  default:
    ui->le_vm_storage->setText(CSystemCallWrapper::get_virtualbox_vm_storage());
    break;
  }

  rebuild_rh_list_model();

  connect(ui->btn_ok, &QPushButton::released, this,
          &DlgSettings::btn_ok_released);
  connect(ui->btn_cancel, &QPushButton::released, this,
          &DlgSettings::btn_cancel_released);
  connect(ui->btn_p2p_file_dialog, &QPushButton::released, this,
          &DlgSettings::btn_p2p_file_dialog_released);
  connect(ui->btn_scp_command, &QPushButton::released, this,
         &DlgSettings::btn_scp_command_released);
  connect(ui->btn_hypervisor_command, &QPushButton::released, this,
        &DlgSettings::btn_hypervisor_release);
  connect(ui->btn_ssh_command, &QPushButton::released, this,
          &DlgSettings::btn_ssh_command_released);
  connect(ui->btn_vagrant_command, &QPushButton::released, this,
          &DlgSettings::btn_vagrant_command_released);
  connect(ui->btn_x2goclient_command, &QPushButton::released, this,
          &DlgSettings::btn_x2goclient_command_released);
  connect(ui->btn_ssh_keygen_command, &QPushButton::released, this,
          &DlgSettings::btn_ssh_keygen_command_released);
  connect(ui->btn_logs_storage, &QPushButton::released, this,
          &DlgSettings::btn_logs_storage_released);
  connect(ui->btn_ssh_keys_storage, &QPushButton::released, this,
          &DlgSettings::btn_ssh_keys_storage_released);
  connect(ui->btn_vm_storage, &QPushButton::released, this,
          &DlgSettings::btn_vm_storage_released);
  connect(ui->btn_refresh_rh_list, &QPushButton::released, this,
          &DlgSettings::btn_refresh_rh_list_released);
  connect(ui->lstv_resource_hosts, &QListView::doubleClicked, this,
          &DlgSettings::lstv_resource_hosts_double_clicked);
  connect(CRhController::Instance(), &CRhController::resource_host_list_updated,
          this, &DlgSettings::resource_host_list_updated_sl);
  connect(&m_refresh_rh_list_timer, &QTimer::timeout, this,
          &DlgSettings::refresh_rh_list_timer_timeout);
  connect(ui->le_terminal_cmd, &QLineEdit::textChanged, this,
          &DlgSettings::le_terminal_cmd_changed);

  this->adjustSize();
}

DlgSettings::~DlgSettings() {
  if (m_tab_resize_filter) delete m_tab_resize_filter;
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::rebuild_rh_list_model() {
  m_model_resource_hosts->clear();

  QStandardItem* item = new QStandardItem("127.0.0.1"); // default ip address
  item->setEditable(false);
  m_model_resource_hosts->appendRow(item);

  for (auto i = CRhController::Instance()->dct_resource_hosts().begin();
       i != CRhController::Instance()->dct_resource_hosts().end(); ++i) {
    QStandardItem* item = new QStandardItem(i->second);
    item->setEditable(false);
    m_model_resource_hosts->appendRow(item);
  }
}
////////////////////////////////////////////////////////////////////////////

/*!
 * \brief This template structure allows us to validate every field.
 * When validation is failed user receives notification and focus moves to wrong
 * field.
 */
template <class TC>
struct field_validator_t {
  TC* fc;  // field control
  QLabel* lbl_err;
  bool (*f_validator)(const TC*);
  int8_t tab_index;
  QString validator_msg;
};

bool is_le_empty_validate(const QLineEdit* le) {
  return !le->text().trimmed().isEmpty();
}

bool folder_has_write_permission(const QLineEdit* le) {
  QFileInfo fi(le->text());
  return fi.isDir() && fi.isWritable();
}

bool is_path_valid(const QLineEdit* le) {
  QFileInfo fi(le->text());
  return fi.exists();
}

bool can_launch_application(const QLineEdit* le) {
  return CCommons::IsApplicationLaunchable(le->text());
}

bool can_launch_terminal(const QLineEdit* le) {
#ifndef RT_OS_DARWIN
  return CCommons::IsApplicationLaunchable(le->text());
#endif
  return CCommons::IsTerminalLaunchable(le->text());
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_ok_released() {
  static const QString empty_validator_msg = tr("Field cannot be empty");
  static const QString folder_permission_validator_msg =
      tr("Administrative rights are needed to open the folder");
  static const QString path_invalid_validator_msg = tr("Invalid path");
  static const QString can_launch_application_msg =
      tr("Cannot launch application");

  if (ui->cb_browser->currentText() == "Edge" &&
      CSettingsManager::Instance().default_browser() != "Edge") {
    QString msg =
        QObject::tr("You have selected Microsoft Edge as a default browser. "
                    "Due to the <a href='https://docs.microsoft.com/en-us/legal"
                    "/windows/agreements/microsoft-browser-extension-policy'>"
                    "Microsoft browser extension policy</a>, Control Center "
                    "can not install/uninstall the Subutai E2E Plugin. Please, "
                    "install it manually.");
    QMessageBox *msgbox =
        new QMessageBox(QMessageBox::Information, QObject::tr("Attention!"),
                        msg, QMessageBox::Close);
    connect(msgbox, &QMessageBox::finished, msgbox, &QMessageBox::deleteLater);
    msgbox->exec();
  }

  CSettingsManager::Instance().set_default_browser(ui->cb_browser->currentText());

  QString profile_name = ui->cb_profile->currentText();
  std::pair<QStringList, QStringList> pnames = chrome_profiles();
  if (!pnames.first.empty()) {
    bool dpset = false;
    for (int i = 0; i < pnames.second.size(); i++) {
      if (pnames.second[i] == profile_name) {
        CSettingsManager::Instance().set_default_chrome_profile(pnames.first[i]);
        dpset = true;
        break;
      }
    }
    if (!dpset) {
      CSettingsManager::Instance().set_default_chrome_profile(pnames.first[0]);
    }
  } else {
    CSettingsManager::Instance().set_default_chrome_profile("Default");
  }

  QLineEdit* le[] = {ui->le_logs_storage,  ui->le_ssh_keys_storage, ui->le_vm_storage,
                     ui->le_p2p_command,   ui->le_ssh_command, ui->le_vagrant_command, ui->le_scp_command};
  QStringList lst_home =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  QString home_folder = lst_home.empty() ? "~" : lst_home[0];

  for (size_t i = 0; i < sizeof(le) / sizeof(QLineEdit*); ++i) {
    QString txt = le[i]->text();
    if (!txt.isEmpty() && txt.at(0) == QChar('~')) {
      txt.replace(0, 1, home_folder);
      le[i]->setText(txt);
    }
  }

  field_validator_t<QLineEdit> le_validators[] = {
    {ui->le_ssh_user, ui->lbl_err_ssh_user, is_le_empty_validate, 0, empty_validator_msg},

    {ui->le_ssh_keys_storage, ui->lbl_err_ssh_keys_storage, is_le_empty_validate, 0, empty_validator_msg},
    {ui->le_ssh_keys_storage, ui->lbl_err_ssh_keys_storage, is_path_valid, 0, path_invalid_validator_msg},
    {ui->le_ssh_keys_storage, ui->lbl_err_ssh_keys_storage, folder_has_write_permission, 0,
     folder_permission_validator_msg},

    {ui->le_vm_storage, ui->lbl_err_vm_storage, is_le_empty_validate, 0, empty_validator_msg},
    {ui->le_vm_storage, ui->lbl_err_vm_storage, is_path_valid, 0, path_invalid_validator_msg},
    {ui->le_vm_storage, ui->lbl_err_vm_storage, folder_has_write_permission, 0,
     folder_permission_validator_msg},

    {ui->le_logs_storage, ui->lbl_err_logs_storage, is_le_empty_validate, 0, empty_validator_msg},
    {ui->le_logs_storage, ui->lbl_err_logs_storage, is_path_valid, 0, path_invalid_validator_msg},
    {ui->le_logs_storage, ui->lbl_err_logs_storage, folder_has_write_permission, 0,
     folder_permission_validator_msg},

    {ui->le_p2p_command, ui->lbl_err_p2p_command, is_le_empty_validate, 1, empty_validator_msg},
    {ui->le_p2p_command, ui->lbl_err_p2p_command, can_launch_application, 1,
     can_launch_application_msg},

    {ui->le_vagrant_command, ui->lbl_err_vagrant_command, is_le_empty_validate, 1, empty_validator_msg},
    {ui->le_vagrant_command, ui->lbl_err_vagrant_command, can_launch_application, 1,
     can_launch_application_msg},

    {ui->le_scp_command, ui->lbl_err_scp_command, is_le_empty_validate, 1, empty_validator_msg},
    {ui->le_scp_command, ui->lbl_err_scp_command, can_launch_application, 1,
     can_launch_application_msg},

    {ui->le_hypervisor_command, ui->lbl_err_hypervisor_command, is_le_empty_validate, 1, empty_validator_msg},
    {ui->le_hypervisor_command, ui->lbl_err_hypervisor_command, can_launch_application, 1, can_launch_application_msg},

    {ui->le_ssh_command, ui->lbl_err_ssh_command, is_le_empty_validate, 1, empty_validator_msg},
    {ui->le_ssh_command, ui->lbl_err_ssh_command, can_launch_application, 1,
     can_launch_application_msg},

    {ui->le_x2goclient_command, ui->lbl_err_x2goclient_command, is_le_empty_validate, 1, empty_validator_msg},
    {ui->le_x2goclient_command, ui->lbl_err_x2goclient_command, can_launch_terminal, 1, can_launch_application_msg},


    {ui->le_terminal_cmd, ui->lbl_err_terminal_cmd, is_le_empty_validate, 1, empty_validator_msg},
    {ui->le_terminal_cmd, ui->lbl_err_terminal_cmd, can_launch_terminal, 1,
     can_launch_application_msg},
    {ui->le_terminal_arg, ui->lbl_err_terminal_arg, is_le_empty_validate, 1, empty_validator_msg},

    {ui->le_ssh_keygen_command, ui->lbl_err_ssh_keygen_command, is_le_empty_validate, 1, empty_validator_msg},
    {ui->le_ssh_keygen_command, ui->lbl_err_ssh_keygen_command, can_launch_application, 1,
     can_launch_application_msg},

    {ui->le_rhip_host, ui->lbl_err_rhip_host, is_le_empty_validate, 2, empty_validator_msg},
    {ui->le_rhip_password, ui->lbl_err_rhip_password, is_le_empty_validate, 2, empty_validator_msg},
    {ui->le_rhip_port, ui->lbl_err_rhip_port, is_le_empty_validate, 2, empty_validator_msg},
    {ui->le_rhip_user, ui->lbl_err_rhip_user, is_le_empty_validate, 2, empty_validator_msg},

    {NULL, NULL, NULL, -1, ""}};

  std::vector<field_validator_t<QLineEdit> > lst_failed_validators;
  field_validator_t<QLineEdit>* tmp = le_validators;

  do {
    tmp->lbl_err->hide();
    if (!tmp->fc->isVisible()) continue;
    if (tmp->f_validator(tmp->fc)) continue;
    lst_failed_validators.push_back(*tmp);
  } while ((++tmp)->fc);

  for (int8_t i = 0; i < (int8_t)lst_failed_validators.size(); ++i) {
    ui->tabWidget->setCurrentIndex(lst_failed_validators[i].tab_index);
    lst_failed_validators[i].fc->setFocus();
    lst_failed_validators[i].lbl_err->show();
    lst_failed_validators[i].lbl_err->setText(QString("<font color='red'>%1</font>").
                                              arg(lst_failed_validators[i].validator_msg));
  }

  if (!lst_failed_validators.empty()) {
    QMessageBox* msg_box =
        new QMessageBox(QMessageBox::Question, tr("Attention! Wrong settings"),
                        tr("You have %1 wrong settings. "
                                "Would you like to correct it? "
                                "Yes - try to correct, No - save anyway")
                        .arg(lst_failed_validators.size()),
                        QMessageBox::Yes | QMessageBox::No);
    connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);

    if (msg_box->exec() == QMessageBox::Yes) {
      return;
    }
  }  // if !lst_failed_validators.empty()


  QString recommendedArg;
  if (CCommons::HasRecommendedTerminalArg(ui->le_terminal_cmd->text(),
                                          recommendedArg)) {
    if (recommendedArg != ui->le_terminal_arg->text()) {
      QMessageBox *msg_box =
          new QMessageBox(QMessageBox::Question, tr("Attention! Wrong terminal argument"),
                          QString("Recommended argument for \"%1\" is \"%2\". Would you like to change it?")
                          .arg(ui->le_terminal_cmd->text()).arg(recommendedArg),
                          QMessageBox::Yes | QMessageBox::No);
      connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
      if (msg_box->exec() == QMessageBox::Yes) {
        ui->le_terminal_arg->setText(recommendedArg);
      }
    }
  }

  switch (VagrantProvider::Instance()->CurrentProvider()) {
  case VagrantProvider::VIRTUALBOX:
    CSystemCallWrapper::set_virtualbox_vm_storage(ui->le_vm_storage->text());
    break;
  case VagrantProvider::VMWARE_DESKTOP:
    CSettingsManager::Instance().set_vmware_vm_storage(ui->le_vm_storage->text());

    break;
  default:
    CSystemCallWrapper::set_virtualbox_vm_storage(ui->le_vm_storage->text());
    break;
  }

  CSettingsManager::Instance().set_vm_storage(ui->le_vm_storage->text());

  CSettingsManager::Instance().set_ssh_user(ui->le_ssh_user->text());
  CSettingsManager::Instance().set_logs_storage(ui->le_logs_storage->text());
  CSettingsManager::Instance().set_ssh_keys_storage(ui->le_ssh_keys_storage->text());

  CSettingsManager::Instance().set_p2p_path(ui->le_p2p_command->text());
  CSettingsManager::Instance().set_vagrant_path(ui->le_vagrant_command->text());
  CSettingsManager::Instance().set_x2goclient_path(ui->le_x2goclient_command->text());
  CSettingsManager::Instance().set_ssh_path(ui->le_ssh_command->text());
  CSettingsManager::Instance().set_scp_path(ui->le_scp_command->text());
  CSettingsManager::Instance().set_hypervisor_path(ui->le_hypervisor_command->text());

  CSettingsManager::Instance().set_rh_host(ui->le_rhip_host->text());
  CSettingsManager::Instance().set_rh_pass(ui->le_rhip_password->text());
  CSettingsManager::Instance().set_rh_port(ui->le_rhip_port->text().toUInt());
  CSettingsManager::Instance().set_rh_user(ui->le_rhip_user->text());

  CSettingsManager::Instance().set_refresh_time_sec(
        ui->sb_refresh_timeout->value());
  CSettingsManager::Instance().set_notification_delay_sec(
        ui->sb_notification_delay->value());

  CSettingsManager::Instance().set_p2p_autoupdate(
        ui->chk_p2p_autoupdate->checkState() == Qt::Checked);
  CSettingsManager::Instance().set_tray_autoupdate(
        ui->chk_tray_autoupdate->checkState() == Qt::Checked);

  CSettingsManager::Instance().set_p2p_update_freq(
        ui->cb_p2p_frequency->currentIndex());
  CSettingsManager::Instance().set_tray_update_freq(
        ui->cb_tray_frequency->currentIndex());
  CSettingsManager::Instance().set_preferred_notifications_place(
        ui->cb_preferred_notifications_place->currentIndex());

  CSettingsManager::Instance().set_notifications_level(
        ui->cb_notification_level->currentIndex());

  CSettingsManager::Instance().set_logs_level(ui->cb_log_level->currentIndex());
  CSettingsManager::Instance().set_tray_skin(ui->cb_tray_skin->currentIndex());

  CSettingsManager::Instance().set_locale(ui->cb_locale->currentIndex());
  CSettingsManager::Instance().set_vagrant_provider(ui->cb_vagrant_provider->currentData().toInt());

  CSettingsManager::Instance().set_terminal_cmd(ui->le_terminal_cmd->text());
  CSettingsManager::Instance().set_terminal_arg(ui->le_terminal_arg->text());

  CSettingsManager::Instance().set_use_animations(
        ui->chk_use_animations->checkState() == Qt::Checked);
  CSettingsManager::Instance().set_ssh_keygen_cmd(
        ui->le_ssh_keygen_command->text());
  CSettingsManager::Instance().set_autostart(ui->chk_autostart->checkState() ==
                                             Qt::Checked);
  CSettingsManager::Instance().save_all();

  // Update peer info and interfaces list
  CPeerController::Instance()->refresh();

  this->close();
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_cancel_released() { this->close(); }

////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_p2p_file_dialog_released() {
  QString fn = QFileDialog::getOpenFileName(this, tr("P2P command"));
  if (fn == "") return;
  ui->le_p2p_command->setText(fn);
  qDebug() << "Selected filename";
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_vagrant_command_released() {
  QString fn = QFileDialog::getOpenFileName(this, tr("Vagrant command"));
  if (fn == "") return;
  ui->le_vagrant_command->setText(fn);
  qDebug() << "Selected vagrant path";
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_scp_command_released() {
  QString fn = QFileDialog::getOpenFileName(this, tr("SCP command"));
  if (fn == "") return;
  ui->le_scp_command->setText(fn);
  qDebug() << "Selected scp path";
}
////////////////////////////////////////////////////////////////////////////
void DlgSettings::btn_ssh_command_released() {
  QString fn = QFileDialog::getOpenFileName(this, tr("SSH command"));
  if (fn == "") return;
  ui->le_ssh_command->setText(fn);
  qDebug() << "Selected filename "<< fn;
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_hypervisor_release() {
  QString fn = QFileDialog::getOpenFileName(this, VagrantProvider::Instance()->CurrentOpenFileTitle());
  if (fn == "") return;
  ui->le_hypervisor_command->setText(fn);
  qDebug() << "Selected virtualbox path" << fn;
}

////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_x2goclient_command_released() {
  QString fn = QFileDialog::getOpenFileName(this, tr("X2Go-Client command"));
  if (fn == "") return;
  ui->le_x2goclient_command->setText(fn);
  qDebug() << "Selected filename " << fn;
}

////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_ssh_keygen_command_released() {
  QString fn = QFileDialog::getOpenFileName(this, tr("SSH-keygen command"));
  if (fn == "") return;
  ui->le_ssh_keygen_command->setText(fn);
  qDebug() << "Selected filename" << fn;
}

////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_logs_storage_released() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("Logs storage"));
  if (dir == "") return;
  ui->le_logs_storage->setText(dir);
  qDebug() << "Selected directory" << dir;
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_ssh_keys_storage_released() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("SSH-keys storage"));
  if (dir == "") return;
  ui->le_ssh_keys_storage->setText(dir);
  qDebug() << "Selected directory " << dir;
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_vm_storage_released() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("VM storage"));
  if (dir == "") return;
  ui->le_vm_storage->setText(dir);
  qDebug() << "Selected directory " << dir;
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::lstv_resource_hosts_double_clicked(QModelIndex ix0) {
  if (!ix0.isValid()) return;
  ui->le_rhip_host->setText(ix0.data().toString());
  qDebug() << "Selected RH IP" << ix0.data().toString();
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_refresh_rh_list_released() {
  m_refresh_rh_list_progress_val = 0;
  ui->btn_refresh_rh_list->setEnabled(false);
  m_refresh_rh_list_timer.start();
  CRhController::Instance()->refresh();
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::refresh_rh_list_timer_timeout() {
  ui->pb_refresh_rh_list->setValue(++m_refresh_rh_list_progress_val);
  if (m_refresh_rh_list_progress_val == CRhController::REFRESH_DELAY_SEC)
    m_refresh_rh_list_timer.stop();
}

void DlgSettings::resource_host_list_updated_sl(bool has_changes) {
  UNUSED_ARG(has_changes);
  rebuild_rh_list_model();
  ui->btn_refresh_rh_list->setEnabled(true);
  qDebug() << "Has changes " << has_changes;
}

////////////////////////////////////////////////////////////////////////////

void DlgSettings::le_terminal_cmd_changed() {
  QString recommendedArg;
  if (CCommons::HasRecommendedTerminalArg(ui->le_terminal_cmd->text(), recommendedArg))
    ui->le_terminal_arg->setText(recommendedArg);
  qDebug() << "recommendedArg = " << recommendedArg;
}
///////////////////////////////////////////∫/////////////////////////////////
