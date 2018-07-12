#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

#include "Commons.h"
#include "DownloadFileManager.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"
#include "P2PController.h"
#include "RestWorker.h"
#include "SystemCallWrapper.h"
#include "updater/ExecutableUpdater.h"
#include "updater/HubComponentsUpdater.h"
#include "updater/UpdaterComponentVirtualbox.h"

CUpdaterComponentVIRTUALBOX::CUpdaterComponentVIRTUALBOX() {
  m_component_id = ORACLE_VIRTUALBOX;
}

CUpdaterComponentVIRTUALBOX::~CUpdaterComponentVIRTUALBOX() {
}

QString CUpdaterComponentVIRTUALBOX::download_virtualbox_path() {
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);
}

bool CUpdaterComponentVIRTUALBOX::update_available_internal() {
    QString version;
    CSystemCallWrapper::oracle_virtualbox_version(version);
    return version == "undefined";
}

chue_t CUpdaterComponentVIRTUALBOX::install_internal() {
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
    QString file_name = oracle_virtualbox_kurjun_package_name();
    QString file_dir = download_virtualbox_path();
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
            this, &CUpdaterComponentVIRTUALBOX::install_finished_sl);
    connect(silent_installer, &SilentInstaller::outputReceived,
            dm, &CDownloadFileManager::deleteLater);
    dm->start_download();
    return CHUE_SUCCESS;
}

chue_t CUpdaterComponentVIRTUALBOX::update_internal() {
    update_progress_sl(100,100);
    update_finished_sl(true);
    return CHUE_SUCCESS;
}

chue_t CUpdaterComponentVIRTUALBOX::uninstall_internal() {
  return CHUE_SUCCESS;
}

void CUpdaterComponentVIRTUALBOX::update_post_action(bool success) {
    UNUSED_ARG(success);
}

void CUpdaterComponentVIRTUALBOX::install_post_interntal(bool success) {
    if(!success)
            CNotificationObserver::Instance()->Error(tr("Failed to complete VirtualBox installation. Try again later, "
                                                        "or install it manually."), DlgNotification::N_NO_ACTION);
    else CNotificationObserver::Instance()->Info(tr("Virtualbox has been installed."), DlgNotification::N_NO_ACTION);
}

void CUpdaterComponentVIRTUALBOX::uninstall_post_internal(bool success) {
}
