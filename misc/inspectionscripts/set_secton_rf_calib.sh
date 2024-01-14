#!/bin/sh
# Date       / Author / Version  
# 11.2.2022 / JStinnett   / v1.0 
# 11.7.2002 / JStinnett   / Add retry logic in case of failure
#
# This script performs a backup of the current Secton
# RF calibration, writes the new calibration file given by
# CAL_FILE to Secton EEPROM, and confirms the calibration
# by reading back and comparing to CAL_FILE

CAL_DIR=/usr/local/dnutils/
CAL_BCKP_DIR=/rwflash/
CAL_FILE=golden_D211_calib_file.bin
CAL_FILE_BCKP=calib_file_D211_bckp.bin
CAL_FILE_CMP=calib_file_D211_cmp.bin

#Change to True to backup calibration before overwriting
BCKP_CAL=False
RETRY_CNT=2

for i in $(seq $RETRY_CNT); do
  echo ""
  echo "set_secton_rf_calib.sh: Starting Secton EEPROM RF calibration"

  if test -f "$CAL_DIR$CAL_FILE_CMP"; then
    rm $CAL_DIR$CAL_FILE_CMP
  fi

  if test -f "$CAL_DIR$CAL_FILE_BCKP"; then
    rm $CAL_DIR$CAL_FILE_BCKP
  fi

  cd /usr/bin

  if [ "$BCKP_CAL" = "True" ]; then
    echo "set_secton_rf_calib.sh: Backing up RF calibration to $CAL_BCKP_DIR$CAL_FILE_BCKP"
diag-cli /dev/spidev1.0 << END > /dev/null 2>&1
wlan calibration read_file $CAL_BCKP_DIR$CAL_FILE_BCKP
quit
END
  fi

  echo ""
  echo "set_secton_rf_calib.sh: Apply ground to Secton module pin 39"
  echo "                        to program RF calibration into EEPROM"
  read -p "Press 'y' to confirm. " -n 1 -r
  echo ""
  if [[ ! $REPLY =~ ^[Yy]$ ]]
  then
    exit 1
  fi

  echo ""
  echo "set_secton_rf_calib.sh: Writing RF calibration to EEPROM!"
diag-cli /dev/spidev1.0 << END > /dev/null 2>&1
eprom write 52 0xff 460
wlan calibration write_file $CAL_DIR$CAL_FILE
quit
END

  echo ""
  echo "set_secton_rf_calib.sh: Remove ground from Secton module pin 39"
  read -p "Press 'y' to confirm. " -n 1 -r
  echo ""
  if [[ ! $REPLY =~ ^[Yy]$ ]]
  then
    exit 1
  fi

  echo ""
  echo "set_secton_rf_calib.sh: Reading calibration from EEPROM!"
diag-cli /dev/spidev1.0 << END > /dev/null 2>&1
wlan calibration read_file /tmp/$CAL_FILE_CMP
quit
END

  if [ -f "/tmp/$CAL_FILE_CMP" ]; then
    echo "set_secton_rf_calib.sh: Confirming new calibration in EEPROM!"
    echo ""
    hash1=$(sha256sum $CAL_DIR$CAL_FILE | cut -d " " -f 1)
    hash2=$(sha256sum /tmp/$CAL_FILE_CMP | cut -d " " -f 1)
    if [ "$hash1" = "$hash2" ]; then
      echo "set_secton_rf_calib.sh: RF calibration SUCCESS!"
      rm /tmp/$CAL_FILE_CMP
      echo "set_secton_rf_calib.sh: Done"
      echo ""
      exit 0
    fi
  fi
  echo "set_secton_rf_calib.sh: RF calibration FAIL!"
  echo "set_secton_rf_calib.sh: Powering OFF Secton!"
  $CAL_DIR/cv2x_pwr_ctl.sh off
  sleep 1
  echo "set_secton_rf_calib.sh: Powering ON Secton!"
  $CAL_DIR/cv2x_pwr_ctl.sh on
  echo "set_secton_rf_calib.sh: Retrying Secton EEPROM RF calibration"
  echo ""
done
