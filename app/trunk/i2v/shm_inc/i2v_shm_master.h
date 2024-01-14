/****************************************************************************
 *                                                                          *
 *  File Name: i2v_shm_master.h                                             *
 *  Author:                                                                 *
 *      DENSO International America, Inc.                                   *
 *      North America Research Laboratory, California Office                *
 *      Rutherford Drive                                                    *
 *      Carlsbad, CA 92008                                                  *
 *                                                                          *
 ****************************************************************************/

#ifndef I2V_SHM_MASTER_H
#define I2V_SHM_MASTER_H

#include "i2v_cfg.h"
#include "i2v_scs_data.h"
#include "i2v_spat_cfgdata.h"
#include "i2v_fwdmsg_data.h" 
#include "i2v_ipb_data.h"
#include "i2v_amh_data.h"
#include "i2v_srm_rx_cfgdata.h"
#include "i2v_srm_rx_data.h"

#define I2V_SHM_PATH "/i2v_shm"
#define I2V_SEM_PATH "/i2v_sem"

typedef struct {
    wuint32                     shmid;
    processPidsT                i2vPids;
    cfgItemsT                   cfgData;
    scsCfgItemsT                scsCfgData;
    scsSpatShmDataT             scsSpatData;
    spatCfgItemsT               spatCfgData;
    fwdmsgCfgItemsT             fwdmsgCfgData;
    ipbCfgItemsT                ipbCfgData;
    amhCfgItemsT                amhCfgData;
    amh2GenericMsgDataT         amhImmediatePsm;
    amh2GenericMsgDataT         amhImmediateBsm;
    srmCfgItemsT                srmCfgData;
    scsSRMShmDataT              scsSRMData;
} i2vShmMasterT;

#endif /* I2V_SHM_MASTER_H */
