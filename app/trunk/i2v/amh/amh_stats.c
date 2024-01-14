/****************************************************************************
 *                                                                          *
 *  File Name: amh_stats.c                                                  *
 *  Author:                                                                 *
 *      DENSO International America, Inc.                                   *
 *      North America Research Laboratory, California Office                *
 *      Rutherford rd, Carlsbad                                             *
 *      CA 92008                                                            *
 *                                                                          *
 ****************************************************************************/
#include <sys/types.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_util.h"
#include "amh.h"

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "amh"

#define AMH_DIAGNOSTIC_WAIT_TIME 10 /*seconds*/

#if defined(EXTRA_EXTRA_DEBUG) /* Enable and each msg type stats dumped. */
STATIC uint8_t dump_amc_stats_count = 0x0;  /* Will dump one line of diag every 10 seconds */
#endif

extern amhManagerStatsT amhManagerStats;
extern bool_t           mainloop;
extern uint32_t         amhEpochCount;

void update_imf_count(uint32_t ret, amhBitmaskType type)
{
    if(AMH_AOK == ret)
        amhManagerStats.imf_tx_cnt++;
    else
        amhManagerStats.imf_tx_err++;

    switch (type) {
        case AMH_MSG_MAP_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.map_imf_msg_tx_count++;
            else 
                amhManagerStats.map_imf_drop_count++;

            break;
        case AMH_MSG_SPAT_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.spat_imf_msg_tx_count++;
            else 
                amhManagerStats.spat_imf_drop_count++;

            break;
        case AMH_MSG_CSR_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.csr_imf_msg_tx_count++;
            else 
                amhManagerStats.csr_imf_drop_count++;

            break;
        case AMH_MSG_EVA_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.eva_imf_msg_tx_count++;
            else 
                amhManagerStats.eva_imf_drop_count++;

            break;
        case AMH_MSG_ICA_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.ica_imf_msg_tx_count++;
            else 
                amhManagerStats.ica_imf_drop_count++;

            break;
        case AMH_MSG_NMEA_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.nmea_imf_msg_tx_count++;
            else 
                amhManagerStats.nmea_imf_drop_count++;

            break;
        case AMH_MSG_PDM_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.pdm_imf_msg_tx_count++;
            else 
                amhManagerStats.pdm_imf_drop_count++;

            break;
        case AMH_MSG_PVD_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.pvd_imf_msg_tx_count++;
            else 
                amhManagerStats.pvd_imf_drop_count++;

            break;
        case AMH_MSG_RSA_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.rsa_imf_msg_tx_count++;
            else 
                amhManagerStats.rsa_imf_drop_count++;

            break;
        case AMH_MSG_RTCM_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.rtcm_imf_msg_tx_count++;
            else 
                amhManagerStats.rtcm_imf_drop_count++;

            break;
        case AMH_MSG_SRM_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.srm_imf_msg_tx_count++;
            else 
                amhManagerStats.srm_imf_drop_count++;

            break;
        case AMH_MSG_SSM_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.ssm_imf_msg_tx_count++;
            else 
                amhManagerStats.ssm_imf_drop_count++;

            break;
        case AMH_MSG_TIM_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.tim_imf_msg_tx_count++;
            else 
                amhManagerStats.tim_imf_drop_count++;

            break;
        case AMH_MSG_PSM_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.psm_imf_msg_tx_count++;
            else 
                amhManagerStats.psm_imf_drop_count++;

            break;
        case AMH_MSG_TEST00_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test00_imf_msg_tx_count++;
            else 
                amhManagerStats.test00_imf_drop_count++;

            break;
        case AMH_MSG_TEST01_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test01_imf_msg_tx_count++;
            else 
                amhManagerStats.test01_imf_drop_count++;

            break;
        case AMH_MSG_TEST02_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test02_imf_msg_tx_count++;
            else 
                amhManagerStats.test02_imf_drop_count++;

            break;
        case AMH_MSG_TEST03_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test03_imf_msg_tx_count++;
            else 
                amhManagerStats.test03_imf_drop_count++;

            break;
        case AMH_MSG_TEST04_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test04_imf_msg_tx_count++;
            else 
                amhManagerStats.test04_imf_drop_count++;

            break;
        case AMH_MSG_TEST05_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test05_imf_msg_tx_count++;
            else 
                amhManagerStats.test05_imf_drop_count++;

            break;
        case AMH_MSG_TEST06_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test06_imf_msg_tx_count++;
            else 
                amhManagerStats.test06_imf_drop_count++;

            break;
        case AMH_MSG_TEST07_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test07_imf_msg_tx_count++;
            else 
                amhManagerStats.test07_imf_drop_count++;

            break;
        case AMH_MSG_TEST08_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test08_imf_msg_tx_count++;
            else 
                amhManagerStats.test08_imf_drop_count++;

            break;
        case AMH_MSG_TEST09_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test09_imf_msg_tx_count++;
            else 
                amhManagerStats.test09_imf_drop_count++;

            break;
        case AMH_MSG_TEST10_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test10_imf_msg_tx_count++;
            else 
                amhManagerStats.test10_imf_drop_count++;

            break;
        case AMH_MSG_TEST11_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test11_imf_msg_tx_count++;
            else 
                amhManagerStats.test11_imf_drop_count++;

            break;
        case AMH_MSG_TEST12_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test12_imf_msg_tx_count++;
            else 
                amhManagerStats.test12_imf_drop_count++;

            break;
        case AMH_MSG_TEST13_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test13_imf_msg_tx_count++;
            else 
                amhManagerStats.test13_imf_drop_count++;

            break;
        case AMH_MSG_TEST14_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test14_imf_msg_tx_count++;
            else 
                amhManagerStats.test14_imf_drop_count++;

            break;
        case AMH_MSG_TEST15_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test15_imf_msg_tx_count++;
            else 
                amhManagerStats.test15_imf_drop_count++;

            break;
        case AMH_MSG_BSM_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.bsm_imf_msg_tx_count++;
            else 
                amhManagerStats.bsm_imf_drop_count++;

            break;
        default: /* Do nothing. */
            break;
    }
}

void update_sar_count(uint32_t ret, amhBitmaskType type)
{
    if(AMH_AOK == ret)
        amhManagerStats.sar_tx_cnt++;
    else
        amhManagerStats.sar_tx_err++;

    switch (type) {
        case AMH_MSG_MAP_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.map_sar_msg_tx_count++;
            else 
                amhManagerStats.map_sar_drop_count++;

            break;
        case AMH_MSG_SPAT_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.spat_sar_msg_tx_count++;
            else 
                amhManagerStats.spat_sar_drop_count++;

            break;
        case AMH_MSG_CSR_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.csr_sar_msg_tx_count++;
            else 
                amhManagerStats.csr_sar_drop_count++;

            break;
        case AMH_MSG_EVA_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.eva_sar_msg_tx_count++;
            else 
                amhManagerStats.eva_sar_drop_count++;

            break;
        case AMH_MSG_ICA_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.ica_sar_msg_tx_count++;
            else 
                amhManagerStats.ica_sar_drop_count++;

            break;
        case AMH_MSG_NMEA_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.nmea_sar_msg_tx_count++;
            else 
                amhManagerStats.nmea_sar_drop_count++;

            break;
        case AMH_MSG_PDM_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.pdm_sar_msg_tx_count++;
            else 
                amhManagerStats.pdm_sar_drop_count++;

            break;
        case AMH_MSG_PVD_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.pvd_sar_msg_tx_count++;
            else 
                amhManagerStats.pvd_sar_drop_count++;

            break;
        case AMH_MSG_RSA_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.rsa_sar_msg_tx_count++;
            else 
                amhManagerStats.rsa_sar_drop_count++;

            break;
        case AMH_MSG_RTCM_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.rtcm_sar_msg_tx_count++;
            else 
                amhManagerStats.rtcm_sar_drop_count++;

            break;
        case AMH_MSG_SRM_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.srm_sar_msg_tx_count++;
            else 
                amhManagerStats.srm_sar_drop_count++;

            break;
        case AMH_MSG_SSM_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.ssm_sar_msg_tx_count++;
            else 
                amhManagerStats.ssm_sar_drop_count++;

            break;
        case AMH_MSG_TIM_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.tim_sar_msg_tx_count++;
            else 
                amhManagerStats.tim_sar_drop_count++;

            break;
        case AMH_MSG_PSM_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.psm_sar_msg_tx_count++;
            else 
                amhManagerStats.psm_sar_drop_count++;

            break;
        case AMH_MSG_TEST00_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test00_sar_msg_tx_count++;
            else 
                amhManagerStats.test00_sar_drop_count++;

            break;
        case AMH_MSG_TEST01_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test01_sar_msg_tx_count++;
            else 
                amhManagerStats.test01_sar_drop_count++;

            break;
        case AMH_MSG_TEST02_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test02_sar_msg_tx_count++;
            else 
                amhManagerStats.test02_sar_drop_count++;

            break;
        case AMH_MSG_TEST03_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test03_sar_msg_tx_count++;
            else 
                amhManagerStats.test03_sar_drop_count++;

            break;
        case AMH_MSG_TEST04_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test04_sar_msg_tx_count++;
            else 
                amhManagerStats.test04_sar_drop_count++;

            break;
        case AMH_MSG_TEST05_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test05_sar_msg_tx_count++;
            else 
                amhManagerStats.test05_sar_drop_count++;

            break;
        case AMH_MSG_TEST06_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test06_sar_msg_tx_count++;
            else 
                amhManagerStats.test06_sar_drop_count++;

            break;
        case AMH_MSG_TEST07_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test07_sar_msg_tx_count++;
            else 
                amhManagerStats.test07_sar_drop_count++;

            break;
        case AMH_MSG_TEST08_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test08_sar_msg_tx_count++;
            else 
                amhManagerStats.test08_sar_drop_count++;

            break;
        case AMH_MSG_TEST09_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test09_sar_msg_tx_count++;
            else 
                amhManagerStats.test09_sar_drop_count++;

            break;
        case AMH_MSG_TEST10_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test10_sar_msg_tx_count++;
            else 
                amhManagerStats.test10_sar_drop_count++;

            break;
        case AMH_MSG_TEST11_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test11_sar_msg_tx_count++;
            else 
                amhManagerStats.test11_sar_drop_count++;

            break;
        case AMH_MSG_TEST12_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test12_sar_msg_tx_count++;
            else 
                amhManagerStats.test12_sar_drop_count++;

            break;
        case AMH_MSG_TEST13_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test13_sar_msg_tx_count++;
            else 
                amhManagerStats.test13_sar_drop_count++;

            break;
        case AMH_MSG_TEST14_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test14_sar_msg_tx_count++;
            else 
                amhManagerStats.test14_sar_drop_count++;

            break;
        case AMH_MSG_TEST15_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.test15_sar_msg_tx_count++;
            else 
                amhManagerStats.test15_sar_drop_count++;

            break;
        case AMH_MSG_BSM_MASK:
            if (ret == AMH_AOK) 
                amhManagerStats.bsm_sar_msg_tx_count++;
            else 
                amhManagerStats.bsm_sar_drop_count++;

            break;
        default: /* Do nothing. */
            break;
    }
}

/* 
 * Slowly dump one line of stats every AMH_DIAGNOSTIC_WAIT_TIME seconds.
 * Not tracking IFM slips since we do not buffer in SHM anymore. Goes straight to radio and waits on mutex.
 */
void dump_amc_stats(void)
{
#if defined(EXTRA_EXTRA_DEBUG)
/* Define one or the other but both is not needed. */
#if 1
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AMC stats:Epoch=%u: SAR Tx=%u Slip=%u Err=%u: IMF Tx=%u Err=%u.\n", amhEpochCount
      , amhManagerStats.sar_tx_cnt, amhManagerStats.sar_tx_slip, amhManagerStats.sar_tx_err, amhManagerStats.imf_tx_cnt, amhManagerStats.imf_tx_err);
#else
  if(0 == dump_amc_stats_count) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AMC stats:Epoch=%u: SAR Tx=%u Slip=%u Err=%u: IMF Tx=%u Err=%u.\n",
                                       amhEpochCount
                                      ,amhManagerStats.sar_tx_cnt
                                      ,amhManagerStats.sar_tx_slip
                                      ,amhManagerStats.sar_tx_err
                                      ,amhManagerStats.imf_tx_cnt
                                      ,amhManagerStats.imf_tx_err);

      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AMC Count: spat=%u,map=%u,tim=%u,rtcm=%u,ica=%d,pdm=%u,rsa=%u,ssm=%u\n",
                                    amhManagerStats.spat_sar_msg_tx_count
                                    ,amhManagerStats.map_sar_msg_tx_count
                                    ,amhManagerStats.tim_sar_msg_tx_count
                                    ,amhManagerStats.rtcm_sar_msg_tx_count
                                    ,amhManagerStats.ica_sar_msg_tx_count
                                    ,amhManagerStats.pdm_sar_msg_tx_count
                                    ,amhManagerStats.rsa_sar_msg_tx_count  
                                    ,amhManagerStats.ssm_sar_msg_tx_count);

  }
  if(1 == dump_amc_stats_count) {

      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"IMF Count: spat=%u,map=%u,tim=%u,rtcm=%u,ica=%d,pdm=%u,rsa=%u,ssm=%u\n",
                                    amhManagerStats.spat_imf_msg_tx_count
                                    ,amhManagerStats.map_imf_msg_tx_count
                                    ,amhManagerStats.tim_imf_msg_tx_count
                                    ,amhManagerStats.rtcm_imf_msg_tx_count
                                    ,amhManagerStats.ica_imf_msg_tx_count
                                    ,amhManagerStats.pdm_imf_msg_tx_count
                                    ,amhManagerStats.rsa_imf_msg_tx_count  
                                    ,amhManagerStats.ssm_imf_msg_tx_count);
  }
  if(2 == dump_amc_stats_count) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SAR Count: csr=%u,eva=%u,nmea=%u,psm=%u,pvd=%d,srm=%u,bsm=%u\n",
                                    amhManagerStats.csr_sar_msg_tx_count
                                    ,amhManagerStats.eva_sar_msg_tx_count
                                    ,amhManagerStats.nmea_sar_msg_tx_count
                                    ,amhManagerStats.psm_sar_msg_tx_count
                                    ,amhManagerStats.pvd_sar_msg_tx_count
                                    ,amhManagerStats.srm_sar_msg_tx_count
                                    ,amhManagerStats.bsm_sar_msg_tx_count);
  }
  if(3 == dump_amc_stats_count) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"IMF Count: csr=%u,eva=%u,nmea=%u,psm=%u,pvd=%d,srm=%u,bsm=%u\n",
                                    amhManagerStats.csr_imf_msg_tx_count
                                    ,amhManagerStats.eva_imf_msg_tx_count
                                    ,amhManagerStats.nmea_imf_msg_tx_count
                                    ,amhManagerStats.psm_imf_msg_tx_count
                                    ,amhManagerStats.pvd_imf_msg_tx_count
                                    ,amhManagerStats.srm_imf_msg_tx_count
                                    ,amhManagerStats.bsm_imf_msg_tx_count);
  }
  if(4 == dump_amc_stats_count) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SAR Count: test00-15(%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)\n", 
                                   amhManagerStats.test00_sar_msg_tx_count
                                  ,amhManagerStats.test01_sar_msg_tx_count
                                  ,amhManagerStats.test02_sar_msg_tx_count
                                  ,amhManagerStats.test03_sar_msg_tx_count
                                  ,amhManagerStats.test04_sar_msg_tx_count
                                  ,amhManagerStats.test05_sar_msg_tx_count
                                  ,amhManagerStats.test06_sar_msg_tx_count
                                  ,amhManagerStats.test07_sar_msg_tx_count
                                  ,amhManagerStats.test08_sar_msg_tx_count
                                  ,amhManagerStats.test09_sar_msg_tx_count
                                  ,amhManagerStats.test10_sar_msg_tx_count
                                  ,amhManagerStats.test11_sar_msg_tx_count
                                  ,amhManagerStats.test12_sar_msg_tx_count
                                  ,amhManagerStats.test13_sar_msg_tx_count
                                  ,amhManagerStats.test14_sar_msg_tx_count
                                  ,amhManagerStats.test15_sar_msg_tx_count);   
  }
  if(5 == dump_amc_stats_count) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"IMF Count: test00-15(%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)\n", 
                                   amhManagerStats.test00_imf_msg_tx_count
                                  ,amhManagerStats.test01_imf_msg_tx_count
                                  ,amhManagerStats.test02_imf_msg_tx_count
                                  ,amhManagerStats.test03_imf_msg_tx_count
                                  ,amhManagerStats.test04_imf_msg_tx_count
                                  ,amhManagerStats.test05_imf_msg_tx_count
                                  ,amhManagerStats.test06_imf_msg_tx_count
                                  ,amhManagerStats.test07_imf_msg_tx_count
                                  ,amhManagerStats.test08_imf_msg_tx_count
                                  ,amhManagerStats.test09_imf_msg_tx_count
                                  ,amhManagerStats.test10_imf_msg_tx_count
                                  ,amhManagerStats.test11_imf_msg_tx_count
                                  ,amhManagerStats.test12_imf_msg_tx_count
                                  ,amhManagerStats.test13_imf_msg_tx_count
                                  ,amhManagerStats.test14_imf_msg_tx_count
                                  ,amhManagerStats.test15_imf_msg_tx_count);   
  }
  if(6 == dump_amc_stats_count) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SAR Slips: spat=%lu,map=%lu,tim=%lu,rtcm=%lu,ica=%lu,pdm=%lu,rsa=%lu,ssm=%lu\n",
                                   amhManagerStats.spat_client_slip_count
                                  ,amhManagerStats.map_client_slip_count
                                  ,amhManagerStats.tim_client_slip_count
                                  ,amhManagerStats.rtcm_client_slip_count
                                  ,amhManagerStats.ica_client_slip_count
                                  ,amhManagerStats.pdm_client_slip_count
                                  ,amhManagerStats.rsa_client_slip_count
                                  ,amhManagerStats.ssm_client_slip_count);   
  }
  if(7 == dump_amc_stats_count) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"IMF Drops: spat=%u,map=%u,tim=%u,rtcm=%u,ica=%d,pdm=%u,rsa=%u,ssm=%u\n",
                                  amhManagerStats.spat_imf_drop_count
                                  ,amhManagerStats.map_imf_drop_count
                                  ,amhManagerStats.tim_imf_drop_count
                                  ,amhManagerStats.rtcm_imf_drop_count
                                  ,amhManagerStats.ica_imf_drop_count
                                  ,amhManagerStats.pdm_imf_drop_count
                                  ,amhManagerStats.rsa_imf_drop_count
                                  ,amhManagerStats.ssm_imf_drop_count);    
  }
  if(8 == dump_amc_stats_count) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SAR Slips(ms): csr=%lu,eva=%lu,nmea=%lu,psm=%lu,pvd=%lu,srm=%lu,bsm=%lu\n",
                                   amhManagerStats.csr_client_slip_count
                                  ,amhManagerStats.eva_client_slip_count
                                  ,amhManagerStats.nmea_client_slip_count
                                  ,amhManagerStats.psm_client_slip_count
                                  ,amhManagerStats.pvd_client_slip_count
                                  ,amhManagerStats.srm_client_slip_count
                                  ,amhManagerStats.bsm_client_slip_count);
  }
  if(9 == dump_amc_stats_count) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"IMF Drops: csr=%u,eva=%u,nmea=%u,psm=%u,pvd=%d,srm=%u,bsm=%u\n",
                                   amhManagerStats.csr_imf_drop_count
                                  ,amhManagerStats.eva_imf_drop_count
                                  ,amhManagerStats.nmea_imf_drop_count
                                  ,amhManagerStats.psm_imf_drop_count
                                  ,amhManagerStats.pvd_imf_drop_count
                                  ,amhManagerStats.srm_imf_drop_count
                                  ,amhManagerStats.bsm_imf_drop_count);
  }
  if(10 == dump_amc_stats_count) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SAR Slips(ms): test00-15(%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu)\n", 
                                 amhManagerStats.test00_client_slip_count
                                ,amhManagerStats.test01_client_slip_count
                                ,amhManagerStats.test02_client_slip_count
                                ,amhManagerStats.test03_client_slip_count
                                ,amhManagerStats.test04_client_slip_count
                                ,amhManagerStats.test05_client_slip_count
                                ,amhManagerStats.test06_client_slip_count
                                ,amhManagerStats.test07_client_slip_count
                                ,amhManagerStats.test08_client_slip_count
                                ,amhManagerStats.test09_client_slip_count
                                ,amhManagerStats.test10_client_slip_count
                                ,amhManagerStats.test11_client_slip_count
                                ,amhManagerStats.test12_client_slip_count
                                ,amhManagerStats.test13_client_slip_count
                                ,amhManagerStats.test14_client_slip_count
                                ,amhManagerStats.test15_client_slip_count);  
  }
  if(11 == dump_amc_stats_count) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"IMF Drops: test00-15(%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)\n", 
                                 amhManagerStats.test00_imf_drop_count
                                ,amhManagerStats.test01_imf_drop_count
                                ,amhManagerStats.test02_imf_drop_count
                                ,amhManagerStats.test03_imf_drop_count
                                ,amhManagerStats.test04_imf_drop_count
                                ,amhManagerStats.test05_imf_drop_count
                                ,amhManagerStats.test06_imf_drop_count
                                ,amhManagerStats.test07_imf_drop_count
                                ,amhManagerStats.test08_imf_drop_count
                                ,amhManagerStats.test09_imf_drop_count
                                ,amhManagerStats.test10_imf_drop_count
                                ,amhManagerStats.test11_imf_drop_count
                                ,amhManagerStats.test12_imf_drop_count
                                ,amhManagerStats.test13_imf_drop_count
                                ,amhManagerStats.test14_imf_drop_count
                                ,amhManagerStats.test15_imf_drop_count); 
  }
  dump_amc_stats_count++;
#endif
#endif
}

void amh_handle_diagnostics(amhBitmaskType encoding, uint64_t delta)
{
  /* Keeps track of how much the slip was, not number of times. */
  switch(encoding) {
      case AMH_MSG_SPAT_MASK:  
          if(amhManagerStats.spat_client_slip_count < delta) {
              amhManagerStats.spat_client_slip_count =  delta;
          }
          break;         
      case AMH_MSG_MAP_MASK:
          if(amhManagerStats.map_client_slip_count < delta) {
              amhManagerStats.map_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_TIM_MASK:
          if(amhManagerStats.tim_client_slip_count < delta) {
              amhManagerStats.tim_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_RTCM_MASK:                
          if(amhManagerStats.rtcm_client_slip_count < delta) {
              amhManagerStats.rtcm_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_ICA_MASK:
          if(amhManagerStats.ica_client_slip_count < delta) {
              amhManagerStats.ica_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_PDM_MASK:   
          if(amhManagerStats.pdm_client_slip_count < delta) {
              amhManagerStats.pdm_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_RSA_MASK:   
          if(amhManagerStats.rsa_client_slip_count < delta) {
              amhManagerStats.rsa_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_SSM_MASK:   
          if(amhManagerStats.ssm_client_slip_count < delta) {
              amhManagerStats.ssm_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_CSR_MASK:
          if(amhManagerStats.csr_client_slip_count < delta) {
              amhManagerStats.csr_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_EVA_MASK:
          if(amhManagerStats.eva_client_slip_count < delta) {
              amhManagerStats.eva_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_NMEA_MASK:
          if(amhManagerStats.nmea_client_slip_count < delta) {
              amhManagerStats.nmea_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_PVD_MASK:
          if(amhManagerStats.pvd_client_slip_count < delta) {
              amhManagerStats.pvd_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_SRM_MASK:
          if(amhManagerStats.srm_client_slip_count < delta) {
              amhManagerStats.srm_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_PSM_MASK:
          if(amhManagerStats.psm_client_slip_count < delta) {
              amhManagerStats.psm_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_BSM_MASK:
          if(amhManagerStats.bsm_client_slip_count < delta) {
              amhManagerStats.bsm_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_TEST00_MASK:  
          if(amhManagerStats.test00_client_slip_count < delta) {
              amhManagerStats.test00_client_slip_count =  delta;
          }
          break;
      case AMH_MSG_TEST01_MASK:
          if(amhManagerStats.test01_client_slip_count < delta) {
              amhManagerStats.test01_client_slip_count =  delta;
          }
          break;
       case AMH_MSG_TEST02_MASK:
          if(amhManagerStats.test02_client_slip_count < delta) {
              amhManagerStats.test02_client_slip_count =  delta;
          }
          break;
       case AMH_MSG_TEST03_MASK:  
          if(amhManagerStats.test03_client_slip_count < delta) {
              amhManagerStats.test03_client_slip_count =  delta;
          }
          break;
       case AMH_MSG_TEST04_MASK:
          if(amhManagerStats.test04_client_slip_count < delta) {
              amhManagerStats.test04_client_slip_count =  delta;
          }
          break;
       case AMH_MSG_TEST05_MASK:  
          if(amhManagerStats.test05_client_slip_count < delta) {
              amhManagerStats.test05_client_slip_count =  delta;
          }
          break;
       case AMH_MSG_TEST06_MASK: 
          if(amhManagerStats.test06_client_slip_count < delta) {
              amhManagerStats.test06_client_slip_count =  delta;
          }
          break;
       case AMH_MSG_TEST07_MASK:
          if(amhManagerStats.test07_client_slip_count < delta) {
              amhManagerStats.test07_client_slip_count =  delta;
          }
          break;
       case AMH_MSG_TEST08_MASK: 
          if(amhManagerStats.test08_client_slip_count < delta) {
              amhManagerStats.test08_client_slip_count =  delta;
          }
          break;
       case AMH_MSG_TEST09_MASK:
          if(amhManagerStats.test09_client_slip_count < delta) {
              amhManagerStats.test09_client_slip_count =  delta;
          }
          break;
       case AMH_MSG_TEST10_MASK:
          if(amhManagerStats.test10_client_slip_count < delta) {
              amhManagerStats.test10_client_slip_count =  delta;
          }
          break;
       case AMH_MSG_TEST11_MASK: 
          if(amhManagerStats.test11_client_slip_count < delta) {
              amhManagerStats.test11_client_slip_count =  delta;
          }
          break;
       case AMH_MSG_TEST12_MASK: 
          if(amhManagerStats.test12_client_slip_count < delta) {
              amhManagerStats.test12_client_slip_count =  delta;
          }
          break;
       case AMH_MSG_TEST13_MASK: 
          if(amhManagerStats.test13_client_slip_count < delta) {
              amhManagerStats.test13_client_slip_count =  delta;
          }
          break;
       case AMH_MSG_TEST14_MASK: 
          if(amhManagerStats.test14_client_slip_count < delta) {
              amhManagerStats.test14_client_slip_count =  delta;
          }
          break;
       case AMH_MSG_TEST15_MASK: 
          if(amhManagerStats.test15_client_slip_count < delta) {
              amhManagerStats.test15_client_slip_count =  delta;
          }
          break;
       default:
          /* If this fails it does not matter. */
          break;
    }
}

