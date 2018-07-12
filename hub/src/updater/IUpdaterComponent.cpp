#include <map>

#include <QStandardPaths>
#include <QApplication>
#include <QMessageBox>

#include "updater/IUpdaterComponent.h"
#include "updater/HubComponentsUpdater.h"
#include "DlgNotification.h"
#include "SettingsManager.h"
#include "HubController.h"
#include "Commons.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"
#include "RestWorker.h"
#include "DownloadFileManager.h"
#include "TrayControlWindow.h"

using namespace update_system;

#ifndef RT_OS_WINDOWS
const QString IUpdaterComponent::P2P = "P2P";
#else
const QString IUpdaterComponent::P2P = "p2p.exe";
#endif
#ifndef RT_OS_WINDOWS
const QString IUpdaterComponent::TRAY = "SubutaiControlCenter";
#else
const QString IUpdaterComponent::TRAY = "SubutaiControlCenter.exe";
#endif
const QString IUpdaterComponent::X2GO = "x2go";
const QString IUpdaterComponent::VAGRANT = "vagrant";
const QString IUpdaterComponent::ORACLE_VIRTUALBOX = "oracle_virtualbox";
const QString IUpdaterComponent::CHROME = "Chrome";
const QString IUpdaterComponent::E2E = "e2e";
const QString IUpdaterComponent::VAGRANT_SUBUTAI = "vagrant_subutai";
const QString IUpdaterComponent::VAGRANT_VBGUEST = "vagrant_vbguest";
const QString IUpdaterComponent::SUBUTAI_BOX = "subutai_box";

const QString &
IUpdaterComponent::component_id_to_user_view(const QString& id) {
  static std::map<QString, QString> dct = {
    {P2P, "P2P"},
    {TRAY, "ControlCenter"},
    {X2GO, "X2Go-Client"},
    {VAGRANT, "Vagrant"},
    {ORACLE_VIRTUALBOX, "Oracle Virtualbox"},
    {CHROME, "Google Chrome"},
    {E2E, "Subutai E2E"},
    {VAGRANT_SUBUTAI, "Subutai plugin"},
    {VAGRANT_VBGUEST, "VirtualBox plugin"},
    {SUBUTAI_BOX, "Subutai box"}
  };
  static const QString def = "";

  if (dct.find(id) == dct.end()) return def;
  return dct.at(id);
}

const QString &
IUpdaterComponent::component_id_changelog(const QString& id) {
  static std::map<QString, QString> dct_changelog = {
    {P2P, "https://github.com/subutai-io/p2p/releases/latest"},
    {TRAY, "https://github.com/subutai-io/control-center/releases/latest"},
    {X2GO, "https://wiki.x2go.org/doku.php/news:start"},
    {VAGRANT, "https://github.com/hashicorp/vagrant/blob/master/CHANGELOG.md"},
    {ORACLE_VIRTUALBOX, "https://www.virtualbox.org/wiki/Changelog"},
    {CHROME, "https://chromereleases.googleblog.com/"},
    {E2E, "https://github.com/subutai-io/browser-plugins/releases/latest"},
    {VAGRANT_SUBUTAI, "https://github.com/subutai-io/vagrant/blob/master/CHANGELOG.md"},
    {VAGRANT_VBGUEST, "https://github.com/dotless-de/vagrant-vbguest/blob/master/CHANGELOG.md"},
    {SUBUTAI_BOX, "https://app.vagrantup.com/subutai/boxes/stretch"}
  };
  static const QString def = "";

  if (dct_changelog.find(id) == dct_changelog.end()) return def;
  return dct_changelog.at(id);
}

DlgNotification::NOTIFICATION_ACTION_TYPE
IUpdaterComponent::component_id_to_notification_action(const QString& id) {
  static std::map<QString, DlgNotification::NOTIFICATION_ACTION_TYPE> dct = {
    {P2P, DlgNotification::N_UPDATE_P2P},
    {TRAY, DlgNotification::N_UPDATE_TRAY},
    {X2GO, DlgNotification::N_ABOUT},
    {VAGRANT, DlgNotification::N_ABOUT},
    {ORACLE_VIRTUALBOX, DlgNotification::N_ABOUT},
    {CHROME, DlgNotification::N_ABOUT},
    {VAGRANT_SUBUTAI, DlgNotification::N_ABOUT},
    {VAGRANT_VBGUEST, DlgNotification::N_ABOUT},
    {SUBUTAI_BOX, DlgNotification::N_ABOUT}
  };
  return dct.at(id);
}

//////////////////////////*X2GO-CLIENT*///////////////////////////////////////

//////////////////////////*VAGRANT*///////////////////////////////////////

///////////////////////////*VIRTUALBOX*///////////////////////////////////////

CUpdaterComponentORACLE_VIRTUALBOX::CUpdaterComponentORACLE_VIRTUALBOX() {
  m_component_id = ORACLE_VIRTUALBOX;
}

CUpdaterComponentORACLE_VIRTUALBOX::~CUpdaterComponentORACLE_VIRTUALBOX() {
}

QString CUpdaterComponentORACLE_VIRTUALBOX::download_oracle_virtualbox_path() {
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);
}

bool CUpdaterComponentORACLE_VIRTUALBOX::update_available_internal() {
    QString version;
    CSystemCallWrapper::oracle_virtualbox_version(version);
    return version == "undefined";
}

chue_t CUpdaterComponentORACLE_VIRTUALBOX::install_internal() {
    qDebug()
            << "Starting install oracle virtualbox";

    QMessageBox *msg_box = new QMessageBox(
          QMessageBox::Information, QObject::tr("Attention!"), QObject::tr(
            "<a href='https://www.virtualbox.org/wiki/VirtualBox'>VirtualBox</a>"
            " is used as the default hypervisor.<br>"
            "VirtualBox will be installed on your machine.<br>"
            "Do you want to proceed?"), QMessageBox::Yes | QMessageBox::No);
    msg_box->setTextFormat(Qt::RichText);

    QObject::connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
    if (msg_box->exec() != QMessageBox::Yes) {
        install_finished_sl(false);
        return CHUE_SUCCESS;
    }

    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    QString file_name = oracle_virtualbox_kurjun_package_name();
    QString file_dir = download_oracle_virtualbox_path();
    QString str_oracle_virtualbox_downloaded_path = file_dir + "/" + file_name;

    std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(file_name);
    if (fi.empty()) {
      qCritical("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());
      install_finished_sl(false);
      return CHUE_NOT_ON_KURJUN;
    }
    std::vector<CGorjunFileInfo>::iterator item = fi.begin();

    CDownloadFileManager *dm = new CDownloadFileManager(item->id(),
                                                        str_oracle_virtualbox_downloaded_path,
                                                        item->size());

    SilentInstaller *silent_installer = new SilentInstaller(this);
    silent_installer->init(file_dir, file_name, CC_VB);
    connect(dm, &CDownloadFileManager::download_progress_sig,
            [this](qint64 rec, qint64 total){update_progress_sl(rec, total+(total/5));});
    connect(dm, &CDownloadFileManager::finished,[silent_installer](bool success){\
        if (!success) {
            silent_installer->outputReceived(success);
        }
        else{
            CNotificationObserver::Instance()->Info(tr("Running installation scripts."), DlgNotification::N_NO_ACTION);
            silent_installer->startWork();
        }
    });
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentORACLE_VIRTUALBOX::install_finished_sl);
    connect(silent_installer, &SilentInstaller::outputReceived,
            dm, &CDownloadFileManager::deleteLater);
    dm->start_download();
    return CHUE_SUCCESS;
}

chue_t CUpdaterComponentORACLE_VIRTUALBOX::update_internal() {
    update_progress_sl(100,100);
    update_finished_sl(true);
    return CHUE_SUCCESS;
}

chue_t CUpdaterComponentORACLE_VIRTUALBOX::uninstall_internal() {
  return CHUE_SUCCESS;
}

void CUpdaterComponentORACLE_VIRTUALBOX::update_post_action(bool success) {
    UNUSED_ARG(success);
}

void CUpdaterComponentORACLE_VIRTUALBOX::install_post_interntal(bool success) {
    if(!success)
            CNotificationObserver::Instance()->Error(tr("Failed to complete VirtualBox installation. Try again later, "
                                                        "or install it manually."), DlgNotification::N_NO_ACTION);
    else CNotificationObserver::Instance()->Info(tr("Virtualbox has been installed."), DlgNotification::N_NO_ACTION);
}

void CUpdaterComponentORACLE_VIRTUALBOX::uninstall_post_internal(bool success) {
}

//////////////////////////*CHROME*///////////////////////////////////////

CUpdaterComponentCHROME::CUpdaterComponentCHROME() {
  m_component_id = CHROME;
}

CUpdaterComponentCHROME::~CUpdaterComponentCHROME() {
}

QString CUpdaterComponentCHROME::download_chrome_path() {
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);
}

bool CUpdaterComponentCHROME::update_available_internal() {
    QString version;
    CSystemCallWrapper::chrome_version(version);
    return version == "undefined";
}

chue_t CUpdaterComponentCHROME::install_internal() {
    qDebug()
            << "Starting install chrome";

    QMessageBox *msg_box = new QMessageBox(
          QMessageBox::Information, QObject::tr("Attention!"), QObject::tr(
            "<a href='https://www.google.com/chrome/'>Chrome</a>"
            " is used as the default browser.<br>"
            "Chrome will be installed on your machine.<br>"
            "Do you want to proceed?"), QMessageBox::Yes | QMessageBox::No);
    msg_box->setTextFormat(Qt::RichText);

    QObject::connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
    if (msg_box->exec() != QMessageBox::Yes) {
        install_finished_sl(false);
        return CHUE_SUCCESS;
    }

    QString file_name = chrome_kurjun_package_name();
    QString file_dir = download_chrome_path();
    QString str_downloaded_path = file_dir + "/" + file_name;

    std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(file_name);
    if (fi.empty()) {
      qCritical("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());
      install_finished_sl(false);
      return CHUE_NOT_ON_KURJUN;
    }
    std::vector<CGorjunFileInfo>::iterator item = fi.begin();

    CDownloadFileManager *dm = new CDownloadFileManager(item->id(),
                                                        str_downloaded_path,
                                                        item->size());

    SilentInstaller *silent_installer = new SilentInstaller(this);
    silent_installer->init(file_dir, file_name, CC_CHROME);
    connect(dm, &CDownloadFileManager::download_progress_sig,
            [this](qint64 rec, qint64 total){update_progress_sl(rec, total+(total/5));});
    connect(dm, &CDownloadFileManager::finished,[silent_installer](bool success){
        if(!success){
            silent_installer->outputReceived(success);
        }
        else{
            CNotificationObserver::Instance()->Info(tr("Running installation scripts."), DlgNotification::N_NO_ACTION);
            silent_installer->startWork();
        }
    });
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentCHROME::install_finished_sl);
    connect(silent_installer, &SilentInstaller::outputReceived,
            dm, &CDownloadFileManager::deleteLater);
    dm->start_download();
    return CHUE_SUCCESS;
}

chue_t CUpdaterComponentCHROME::update_internal() {
    update_progress_sl(100,100);
    update_finished_sl(true);
    return CHUE_SUCCESS;
}

chue_t CUpdaterComponentCHROME::uninstall_internal() {
  static QString empty_string = "";

  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_CHROME);

  connect(silent_uninstaller, &SilentUninstaller::outputReceived,
          this, &CUpdaterComponentCHROME::uninstall_finished_sl);

  silent_uninstaller->startWork();

  return CHUE_SUCCESS;
}

void CUpdaterComponentCHROME::update_post_action(bool success) {
    UNUSED_ARG(success);
}

void CUpdaterComponentCHROME::install_post_interntal(bool success) {
  if(!success)
    CNotificationObserver::Instance()->Error(tr("Failed to complete Google Chrome installation. Try again later, "
                                                "or install it manually."), DlgNotification::N_NO_ACTION);
  else CNotificationObserver::Instance()->Info(tr("Google Chrome has been installed."), DlgNotification::N_NO_ACTION);
}

void CUpdaterComponentCHROME::uninstall_post_internal(bool success) {
  if(!success)
    CNotificationObserver::Instance()->Error(tr("Failed to complete Google Chrome uninstallation. Try again later, "
                                                "or uninstall it manually."), DlgNotification::N_NO_ACTION);
  else CNotificationObserver::Instance()->Info(tr("Google Chrome has been uninstalled."), DlgNotification::N_NO_ACTION);
}

//////////////////////////*E2E*///////////////////////////////////////

CUpdaterComponentE2E::CUpdaterComponentE2E() {
  m_component_id = E2E;
}

CUpdaterComponentE2E::~CUpdaterComponentE2E() {
}

QString CUpdaterComponentE2E::download_e2e_path() {
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);
}

bool CUpdaterComponentE2E::update_available_internal() {
    QString version;
    CSystemCallWrapper::subutai_e2e_version(version);
    return version == "undefined";
}

chue_t CUpdaterComponentE2E::install_internal() {
    qDebug()
            << "Starting install subutai e2e";
    if(CSettingsManager::Instance().default_browser() == "Chrome") {
        QMessageBox *msg_box = new QMessageBox(
              QMessageBox::Information, QObject::tr("Attention!"), QObject::tr(
                "The <a href='https://subutai.io/getting-started.html#E2E'>Subutai E2E plugin</a>"
                " manages your PGP keys.<br>"
                "Installing the E2E plugin will restart your browser. "
                "Be sure to save your work before installing, and "
                "approve the extension after installing.<br>"
                "Do you want to proceed?"), QMessageBox::Yes | QMessageBox::No);
        msg_box->setTextFormat(Qt::RichText);

        QObject::connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
        if (msg_box->exec() != QMessageBox::Yes) {
            install_finished_sl(false);
            return CHUE_SUCCESS;
        }
    }
    static QString empty_stings = "";
    SilentInstaller *silent_installer = new SilentInstaller(this);
    silent_installer->init(empty_stings, empty_stings, CC_E2E);
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentE2E::install_finished_sl);
    silent_installer->startWork();
    return CHUE_SUCCESS;
}

chue_t CUpdaterComponentE2E::update_internal() {
    update_finished_sl(true);
    return CHUE_SUCCESS;
}

chue_t CUpdaterComponentE2E::uninstall_internal() {
  qDebug() << "Starting uninstall subutai e2e";
  static QString empty_string = "";
  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_E2E);
  connect(silent_uninstaller, &SilentUninstaller::outputReceived,
          this, &CUpdaterComponentE2E::uninstall_finished_sl);
  silent_uninstaller->startWork();
  return CHUE_SUCCESS;
}

void CUpdaterComponentE2E::update_post_action(bool success) {
    UNUSED_ARG(success);
}

void CUpdaterComponentE2E::install_post_interntal(bool success) {
    if(!success){
        CNotificationObserver::Instance()->Info(tr("Failed to complete E2E plugin installation. You may try installing directly through the link under "
                                                   "<a href='https://subutai.io/getting-started.html#E2E'>Getting Started</a>"
                                                   " or try again by restarting the Control Center first."), DlgNotification::N_NO_ACTION);
        return;
    }
    QMessageBox *msg_box = new QMessageBox(QMessageBox::Information, QObject::tr("Attention!"),
                                           QObject::tr("<br>Subutai E2E has been installed to your browser</br>"
                                                       "<br>If E2E does not appear, please approve installation from chrome://extensions.\n\t</br>"
                                                       "<br><a href='https://docs.subutai.io/Products/Bazaar/27_E2E_plugin.html'>"
                                                       "Learn more about Subutai E2E."
                                                       "</a></br>"),
                                           QMessageBox::Ok);
    msg_box->setTextFormat(Qt::RichText);
    QObject::connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
    if(msg_box->exec() == QMessageBox::Ok){
        CSystemCallWrapper::chrome_last_section();
    }
}

void CUpdaterComponentE2E::uninstall_post_internal(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Info(tr("Failed to uninstall Subutai E2E plugin. Try uninstalling manually."), DlgNotification::N_NO_ACTION);
    return;
  }
  QMessageBox *msg_box = new QMessageBox(QMessageBox::Information, QObject::tr("Attention!"),
                                         QObject::tr("Subutai E2E has been uninstalled from your browser<br>"
                                                     "Press OK to restore your browser session."),
                                         QMessageBox::Ok);
  msg_box->setTextFormat(Qt::RichText);
  QObject::connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
  if (msg_box->exec() == QMessageBox::Ok) {
      CSystemCallWrapper::chrome_last_section();
  }
}

//////////////////////////*VAGRANT-SUBUTAI*///////////////////////////////////////

CUpdaterComponentVAGRANT_SUBUTAI::CUpdaterComponentVAGRANT_SUBUTAI() {
  m_component_id = VAGRANT_SUBUTAI;
}

CUpdaterComponentVAGRANT_SUBUTAI::~CUpdaterComponentVAGRANT_SUBUTAI() {
}

bool CUpdaterComponentVAGRANT_SUBUTAI::update_available_internal() {
    QString version;
    QString subutai_plugin = "vagrant-subutai";
    system_call_wrapper_error_t res = CSystemCallWrapper::vagrant_subutai_version(version);
    QString cloud_version =
        CRestWorker::Instance()->get_vagrant_plugin_cloud_version(subutai_plugin);
    if (version == "undefined") return true;
    if (res != SCWE_SUCCESS) return false;
    if (cloud_version == "undefined" || cloud_version.isEmpty()) return false;
    return cloud_version > version;
}

chue_t CUpdaterComponentVAGRANT_SUBUTAI::install_internal(){
    qDebug()
            << "Starting install vagrant subutai";

    QMessageBox *msg_box = new QMessageBox(
          QMessageBox::Information, QObject::tr("Attention!"), QObject::tr(
            "The Vagrant Subutai plugin sets up peer parameters, like disk size and RAM.\n"
            "The Vagrant Subutai plugin will be installed on your machine.\n"
            "Do you want to proceed?"), QMessageBox::Yes | QMessageBox::No);

    QObject::connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
    if (msg_box->exec() != QMessageBox::Yes) {
        install_finished_sl(false);
        return CHUE_SUCCESS;
    }

    update_progress_sl(50, 100); // imitation of progress bar :D, todo implement
    static QString empty_string = "";
    SilentInstaller *silent_installer = new SilentInstaller(this);
    silent_installer->init(empty_string, empty_string, CC_VAGRANT_SUBUTAI);
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentVAGRANT_SUBUTAI::install_finished_sl);
    silent_installer->startWork();
    return CHUE_SUCCESS;
}

chue_t CUpdaterComponentVAGRANT_SUBUTAI::update_internal() {
  update_progress_sl(50, 100);
  static QString empty_string = "";

  SilentUpdater *silent_updater = new SilentUpdater(this);
  silent_updater->init(empty_string, empty_string, CC_VAGRANT_SUBUTAI);

  connect(silent_updater, &SilentUpdater::outputReceived,
          this, &CUpdaterComponentVAGRANT_SUBUTAI::update_finished_sl);

  silent_updater->startWork();

  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentVAGRANT_SUBUTAI::uninstall_internal() {
  update_progress_sl(50, 100);
  static QString empty_string = "";

  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_VAGRANT_SUBUTAI);

  connect(silent_uninstaller, &SilentUninstaller::outputReceived,
          this, &CUpdaterComponentVAGRANT_SUBUTAI::uninstall_finished_sl);

  silent_uninstaller->startWork();

  return CHUE_SUCCESS;
}

void CUpdaterComponentVAGRANT_SUBUTAI::update_post_action(bool success) {
    if(!success) {
        CNotificationObserver::Instance()->Info(tr("Failed to update the Vagrant Subutai plugin. You may try manually installing it "
                                                   "or try again by restarting the Control Center first."), DlgNotification::N_NO_ACTION);
    } else {
        CNotificationObserver::Instance()->Info(tr("Vagrant Subutai plugin has been updated successfully."), DlgNotification::N_NO_ACTION);
    }
}
void CUpdaterComponentVAGRANT_SUBUTAI::install_post_interntal(bool success) {
    if(!success) {
        CNotificationObserver::Instance()->Info(tr("Failed to install the Vagrant Subutai plugin. You may try manually installing it "
                                                   "or try again by restarting the Control Center first."), DlgNotification::N_NO_ACTION);
    } else {
        CNotificationObserver::Instance()->Info(tr("Vagrant Subutai plugin has been installed successfully."), DlgNotification::N_NO_ACTION);
    }
}

void CUpdaterComponentVAGRANT_SUBUTAI::uninstall_post_internal(bool success) {
    if (!success) {
      CNotificationObserver::Instance()->Info(tr("Failed to uninstall the Vagrant Subutai plugin. You may try manually uninstalling it "
                                                 "or try again by restarting the Control Center first."), DlgNotification::N_NO_ACTION);
    } else {
      CNotificationObserver::Instance()->Info(tr("Vagrant Subutai plugin has been uninstalled successfully."), DlgNotification::N_NO_ACTION);
    }
}

//////////////////////////*VAGRANT-VBGUEST*///////////////////////////////////////

CUpdaterComponentVAGRANT_VBGUEST::CUpdaterComponentVAGRANT_VBGUEST() {
  m_component_id = VAGRANT_VBGUEST;
}

CUpdaterComponentVAGRANT_VBGUEST::~CUpdaterComponentVAGRANT_VBGUEST() {
}

bool CUpdaterComponentVAGRANT_VBGUEST::update_available_internal() {
    QString version;
    QString subutai_plugin = "vagrant-vbguest";
    system_call_wrapper_error_t res = CSystemCallWrapper::vagrant_vbguest_version(version);
    QString cloud_version =
        CRestWorker::Instance()->get_vagrant_plugin_cloud_version(subutai_plugin);
    if (version == "undefined") return true;
    if (res != SCWE_SUCCESS) return false;
    if (cloud_version == "undefined" || cloud_version.isEmpty()) return false;
    return cloud_version != version;
}
chue_t CUpdaterComponentVAGRANT_VBGUEST::install_internal() {
    qDebug()
            << "Starting install vagrant vbguest";

    QMessageBox *msg_box = new QMessageBox(
          QMessageBox::Information, QObject::tr("Attention!"), QObject::tr(
            "The Vagrant VirtualBox plugin sets VirtualBox as your hypervisor for Vagrant.\n"
            "The Vagrant VirtualBox plugin will be installed on your machine.\n"
            "Do you want to proceed?"), QMessageBox::Yes | QMessageBox::No);

    QObject::connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
    if (msg_box->exec() != QMessageBox::Yes) {
        install_finished_sl(false);
        return CHUE_SUCCESS;
    }

    update_progress_sl(50, 100);
    static QString empty_string = "";
    SilentInstaller *silent_installer = new SilentInstaller(this);
    silent_installer->init(empty_string, empty_string, CC_VAGRANT_VBGUEST);
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentVAGRANT_VBGUEST::install_finished_sl);
    silent_installer->startWork();
    return CHUE_SUCCESS;
}

chue_t CUpdaterComponentVAGRANT_VBGUEST::update_internal() {
    update_progress_sl(50, 100);
    static QString empty_string = "";
    SilentUpdater *silent_updater = new SilentUpdater(this);
    silent_updater->init(empty_string, empty_string, CC_VAGRANT_VBGUEST);
    connect(silent_updater, &SilentUpdater::outputReceived,
            this, &CUpdaterComponentVAGRANT_VBGUEST::update_finished_sl);
    silent_updater->startWork();
    return CHUE_SUCCESS;
}

chue_t CUpdaterComponentVAGRANT_VBGUEST::uninstall_internal() {
  update_progress_sl(50, 100);
  static QString empty_string = "";

  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_VAGRANT_VBGUEST);

  connect(silent_uninstaller, &SilentUninstaller::outputReceived,
          this, &CUpdaterComponentVAGRANT_VBGUEST::uninstall_finished_sl);

  silent_uninstaller->startWork();

  return CHUE_SUCCESS;
}

void CUpdaterComponentVAGRANT_VBGUEST::update_post_action(bool success) {
    if(!success) {
        CNotificationObserver::Instance()->Info(tr("Failed to update the Vagrant VirtualBox plugin. You may try manually installing it "
                                                   "or try again by restarting the Control Center first."), DlgNotification::N_NO_ACTION);
    }
    else {
        CNotificationObserver::Instance()->Info(tr("Vagrant VirtualBox plugin has been updated successfully."), DlgNotification::N_NO_ACTION);
    }
}

void CUpdaterComponentVAGRANT_VBGUEST::install_post_interntal(bool success) {
    if(!success){
        CNotificationObserver::Instance()->Info(tr("Failed to install the Vagrant VirtualBox plugin. You may try manually installing it "
                                                   "or try again by restarting the Control Center first."), DlgNotification::N_NO_ACTION);
    } else {
        CNotificationObserver::Instance()->Info(tr("Vagrant VirtualBox plugin has been installed successfully."), DlgNotification::N_NO_ACTION);
    }
}

void CUpdaterComponentVAGRANT_VBGUEST::uninstall_post_internal(bool success) {
  if(!success){
    CNotificationObserver::Instance()->Info(tr("Failed to uninstall the Vagrant VirtualBox plugin. You may try manually uninstalling it "
                                                   "or try again by restarting the Control Center first."), DlgNotification::N_NO_ACTION);
  } else {
    CNotificationObserver::Instance()->Info(tr("Vagrant VirtualBox plugin has been uninstalled successfully."), DlgNotification::N_NO_ACTION);
  }
}

//////////////////////////*SUBUTAI-BOX*///////////////////////////////////////

CUpdaterComponentSUBUTAI_BOX::CUpdaterComponentSUBUTAI_BOX() {
  m_component_id = SUBUTAI_BOX;
}

CUpdaterComponentSUBUTAI_BOX::~CUpdaterComponentSUBUTAI_BOX() {}

QString CUpdaterComponentSUBUTAI_BOX::download_subutai_box_path() {
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);
}

bool CUpdaterComponentSUBUTAI_BOX::update_available_internal() {
    QString version;
    QString subutai_box = subutai_box_name();
    QString subutai_provider = "virtualbox";
    system_call_wrapper_error_t res =
        CSystemCallWrapper::vagrant_latest_box_version(subutai_box, subutai_provider, version);
    QString cloud_version =
        CRestWorker::Instance()->get_vagrant_box_cloud_version(subutai_box, subutai_provider);
    qDebug() << "subutai box version " << version << " vs "
             << "subutai box cloud version " << cloud_version;
    if (version == "undefined") return true;
    if (res != SCWE_SUCCESS) return false;
    if (cloud_version == "undefined" || cloud_version.isEmpty()) return false;
    return cloud_version > version;
}

chue_t CUpdaterComponentSUBUTAI_BOX::install_internal() {
    qDebug()
            << "Starting install new version of subutai box";

    QString version;
    QString subutai_box = subutai_box_name();
    QString subutai_provider = "virtualbox";

    CSystemCallWrapper::vagrant_latest_box_version(subutai_box, subutai_provider, version);
    if (version == "undefined") {
      QMessageBox *msg_box = new QMessageBox(
            QMessageBox::Information, QObject::tr("Attention!"), QObject::tr(
              "<a href='https://app.vagrantup.com/subutai/boxes/stretch'>Subutai Box</a>"
              " is the resource box for peer creation.<br>"
              "Subutai Box will be installed on your machine.<br>"
              "Do you want to proceed?"), QMessageBox::Yes | QMessageBox::No);
      msg_box->setTextFormat(Qt::RichText);

      QObject::connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
      if (msg_box->exec() != QMessageBox::Yes) {
          install_finished_sl(false);
          return CHUE_SUCCESS;
      }
    }

    QString file_name = subutai_box_kurjun_package_name(subutai_provider);
    QString file_dir = download_subutai_box_path();
    QString str_downloaded_path = file_dir + "/" + file_name;

    std::vector<CGorjunFileInfo> fi =
        CRestWorker::Instance()->get_gorjun_file_info(file_name,
                                                      "https://cdn.subutai.io:8338/kurjun/rest/raw/info");
    if (fi.empty()) {
      qCritical("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());
      install_finished_sl(false);
      return CHUE_NOT_ON_KURJUN;
    }
    std::vector<CGorjunFileInfo>::iterator item = fi.begin();

    CDownloadFileManager *dm = new CDownloadFileManager(item->id(),
                                                        str_downloaded_path,
                                                        item->size());

    dm->set_link("https://cdn.subutai.io:8338/kurjun/rest/raw/download");
    SilentInstaller *silent_installer = new SilentInstaller(this);
    silent_installer->init(file_dir, file_name, CC_SUBUTAI_BOX);

    connect(dm, &CDownloadFileManager::download_progress_sig,
            [this](qint64 rec, qint64 total) {
        update_progress_sl(rec, total+(total/5));
    });

    connect(dm, &CDownloadFileManager::finished,[silent_installer](bool success) {
        if(!success) {
            silent_installer->outputReceived(success);
        } else {
            CNotificationObserver::Instance()->Info(tr("Running installation scripts."), DlgNotification::N_NO_ACTION);
            silent_installer->startWork();
        }
    });
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentSUBUTAI_BOX::install_finished_sl);

    connect(silent_installer, &SilentInstaller::outputReceived,
            dm, &CDownloadFileManager::deleteLater);

    dm->start_download();
    return CHUE_SUCCESS;
}

chue_t CUpdaterComponentSUBUTAI_BOX::uninstall_internal() {
  size_t total = TrayControlWindow::Instance()->machine_peers_table.size();
  qDebug() << "Total machine peers: "
           << total;

  // while removing vagrant box. we should check existing vagrant machines. it will corrupt existing machines.
  if (total > 0) {
    QMessageBox *msg_box = new QMessageBox(
          QMessageBox::Information, QObject::tr("Attention!"), QObject::tr(
            "Removing the Subutai box could corrupt the peers. "
            "We recommend destroying peers first from \"My Peers\" menu.\n"
            "Do you want to proceed?"), QMessageBox::Yes | QMessageBox::No);
    msg_box->setTextFormat(Qt::RichText);

    QObject::connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
    if (msg_box->exec() != QMessageBox::Yes) {
      emit uninstall_finished_sl(false);
      return CHUE_SUCCESS;
    }
  }


  update_progress_sl(50, 100);
  static QString empty_string = "";

  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_SUBUTAI_BOX);

  connect(silent_uninstaller, &SilentUninstaller::outputReceived,
          this, &CUpdaterComponentSUBUTAI_BOX::uninstall_finished_sl);

  silent_uninstaller->startWork();

  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentSUBUTAI_BOX::update_internal() {
    return install_internal();
}

void CUpdaterComponentSUBUTAI_BOX::update_post_action(bool success) {
    if(!success) {
        CNotificationObserver::Instance()->Error(tr("Vagrant Subutai box failed to update"), DlgNotification::N_ABOUT);
    }
    else {
        CNotificationObserver::Instance()->Info(tr("Succesfully updated Vagrant Subutai box, congratulations!"), DlgNotification::N_ABOUT);
    }
}

void CUpdaterComponentSUBUTAI_BOX::install_post_interntal(bool success) {
    if(!success) {
        CNotificationObserver::Instance()->Error(tr("Vagrant Subutai box installation has failed."), DlgNotification::N_NO_ACTION);}
    else {
        CNotificationObserver::Instance()->Info(tr("Vagrant Subutai box has been installed. Congratulations!"), DlgNotification::N_NO_ACTION);
    }
}

void CUpdaterComponentSUBUTAI_BOX::uninstall_post_internal(bool success) {
  if(!success) {
      CNotificationObserver::Instance()->Error(tr("Vagrant Subutai box deletion has failed."), DlgNotification::N_NO_ACTION);}
  else {
      CNotificationObserver::Instance()->Info(tr("Vagrant Subutai box has been deleted. Congratulations!"), DlgNotification::N_NO_ACTION);
  }
}
