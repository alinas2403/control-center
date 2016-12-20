#include "rtm/include/RtmController.h"
#include "SystemCallWrapper.h"
#include "libssh2/include/LibsshController.h"
#include "ApplicationLog.h"
#include "SettingsManager.h"
#include "rtm/include/RtmDbController.h"

rtm::sysinfo_t os_get_sysinfo();

#ifdef RT_OS_LINUX
#include <sys/sysinfo.h>
rtm::sysinfo_t os_get_sysinfo() {
  rtm::sysinfo_t res;
  memset(&res, 0, sizeof(rtm::sysinfo_t));
  struct sysinfo si;
  memset(&si, 0, sizeof(struct sysinfo));
  int rc = sysinfo(&si);
  if (rc != 0) {
    CApplicationLog::Instance()->LogTrace("Sysinfo call failed with rc = %d", rc);
    return res;
  }

  res.uptime = si.uptime;
  for (int i = 0; i < 3; ++i)
    res.loads[i] = si.loads[i];
  res.totalram = si.totalram;
  res.freeram = si.freeram;
  res.sharedram = si.sharedram;
  res.bufferram = si.bufferram;
  res.totalswap = si.totalhigh;
  res.freeswap = si.freeswap;
  res.procs = si.procs;
  res.totalhigh = si.totalhigh;
  res.freehigh = si.freehigh;
  res.mem_unit = si.mem_unit;
  return res;
}
#elif defined(RT_OS_WINDOWS)
rtm::sysinfo_t os_get_sysinfo() {
  rtm::sysinfo_t res;
  memset(&res, 0, sizeof(rtm::sysinfo_t));
  return res;
}
#else // defined(RT_OS_DARWIN)
rtm::sysinfo_t os_get_sysinfo() {
  rtm::sysinfo_t res;
  memset(&res, 0, sizeof(rtm::sysinfo_t));
  return res;
}
#endif

//std::vector<std::string>
//rtm::CRtmController::get_output_by_syscall(const char *cmd,
//                                           bool *success) {
//  std::vector<std::string> lst_out;
//  int ec = -1;
//  system_call_wrapper_error_t scwe =
//      CSystemCallWrapper::ssystem_th(cmd, lst_out, ec, true);
//  *success = scwe == SCWE_SUCCESS;
//  return lst_out;
//}
////////////////////////////////////////////////////////////////////////////

std::vector<std::string>
rtm::CRtmController::get_output_by_libssh2(const char *cmd,
                                           bool *success) {
  std::vector<std::string> lst_out;
  int rc = CLibsshController::run_ssh_command_pass_auth(
             CSettingsManager::Instance().rh_host().toStdString().c_str(),
             CSettingsManager::Instance().rh_port(),
             CSettingsManager::Instance().rh_user().toStdString().c_str(),
             CSettingsManager::Instance().rh_pass().toStdString().c_str(),
             cmd,
             10,
             lst_out);
  *success = rc == 0;
  return lst_out;
}
////////////////////////////////////////////////////////////////////////////

rtm::CRtmController::CRtmController() {
//  m_rpp_syscall = new CRtmProcParser(CRtmController::get_output_by_syscall);
  m_rpp_libssh2 = new CRtmProcParser(CRtmController::get_output_by_libssh2);
}

rtm::CRtmController::~CRtmController() {

}
////////////////////////////////////////////////////////////////////////////

rtm::CRtmController*
rtm::CRtmController::Instance() {
  static CRtmController inst;
  return &inst;
}
////////////////////////////////////////////////////////////////////////////

rtm::sysinfo_t
rtm::CRtmController::rtm_sysinfo() const {
  return os_get_sysinfo();
}
////////////////////////////////////////////////////////////////////////////

void
rtm::CRtmController::Megatest() {
//  bool succes;
//  bool is_valid = CRtmDbController::Instance().is_valid();

//  proc_load_avg_t sys1 = m_rpp_syscall->load_average(succes);
//  proc_load_avg_t sys2 = m_rpp_libssh2->load_average(succes);

//  proc_meminfo_t mem1  = m_rpp_syscall->meminfo(succes);
//  proc_meminfo_t mem2  = m_rpp_libssh2->meminfo(succes);

//  std::vector<proc_net_dev_t> ni1 = m_rpp_syscall->network_info(succes);
//  std::vector<proc_net_dev_t> ni2 = m_rpp_libssh2->network_info(succes);

//  proc_uptime_t up1 = m_rpp_syscall->uptime(succes);
//  proc_uptime_t up2 = m_rpp_libssh2->uptime(succes);

//  rtm_sysinfo();
//  CApplicationLog::Instance()->LogInfo("Abircheg");
}
////////////////////////////////////////////////////////////////////////////