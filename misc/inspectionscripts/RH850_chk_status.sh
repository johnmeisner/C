#!/bin/sh
# This script queries the RH850 for diagnostic status
# 1) dcin = DC Input Voltage (low/high/pass)
# 2) cv2x = C-V2X Antenna 1/2 Status (open/short/pass)
# 3) dsrc = DSRC Antenna 1/2 Status (open/short/pass)  
# 4) gnss = GNSS Antenna Status (open/short/pass)
# 5) rfsw = RF Switch Daughter Board Antenna Status (open/short/pass)
#
# Date       / Author / Version  
# 12.2.2020 / JStinnett   / v1.0   
# 12.7.2020 / JStinnett   / v1.1 Add support to query only DCIN ADC from RH850.  Bug fix to GPS logic
# 12.9.2020 / JStinnett   / v1.2 Add loop support.  Update GPS antenna detection logic
# 1.26.2021 / JStinnett   / v1.3 Add support for RF switch antenna diagnostics

trap deactivate SIGINT

function usage
{
    echo "" 
    echo "Query RH850 diagnostic status"
    echo "RH850_chk_status.sh [cv2x|dcin|dsrc|gps|rfsw|all] [v(verbose)|loop]"
    echo "ex: RH850_check_status.sh all loop"
    echo "ex. RH850_check_status.sh dcin v loop"
    echo ""
}

function deactivate #kill cat PID and remove tmp file
{
    disown $PID
    kill -9 $PID
    rm $tmpfile
    if [ "$2" == "loop" ] || [ "$3" == "loop" ] ; then
      tput rmcup 
    fi
    tput reset > $RH850_TTY
    exit 0
}
  
# This function extract the ADC counts from the RH850 response to the 'Za' command.
function get_adc_count () {
    local res=$(echo "$lines" | grep -w $1 | tail -c 5)
    echo $res
}

# This function takes a hex value (1) and verifies that the value is in between the minimum value (2) and the max value (3). 
function check_value () {
    [[ ($1 -ge $2) && ($1 -le $3) ]] && echo PASS
    [[ ($1 -lt $2) ]] && echo FAIL_LOW
    [[ ($1 -gt $3) ]] && echo FAIL_HIGH
}

if [ "$1" != "cv2x" ] && [ "$1" != "dcin" ] && [ "$1" != "dsrc" ] && [ "$1" != "gps" ] && [ "$1" != "rfsw" ] && [ "$1" != "all" ] ; then
  usage
  exit 0
fi


# Constants
RH850_TTY="/dev/ttyLP3"
RH850_BAUD="115200"

GPS_DIAG2_DIAG1_DELTA_MIN=0x00E3  #50mA antenna load (1.83V delta)
GPS_DIAG2_DIAG1_DELTA_MAX=0x08DF  #5mA antenna load (183mV delta)
GPS_DIAG1_ADC="AP0_0"

GPS_DIAG2_MIN=0x0ACD  #TP3710 = 4.9V
GPS_DIAG2_MAX=0x0D7C  #TP3710 = 5.1V
GPS_DIAG2_ADC="AP0_1"

CV2X_ANT1_MIN=0x0100
CV2X_ANT1_MAX=0x0200
CV2X_ANT1_ADC="AP0_2"

CV2X_ANT2_MIN=0x0100
CV2X_ANT2_MAX=0x0200
CV2X_ANT2_ADC="AP0_3"

RFSW_ANT1_MIN=0x100
RFSW_ANT1_MAX=0x200
RFSW_ANT1_ADC="AP0_4"

RFSW_ANT2_MIN=0x100
RFSW_ANT2_MAX=0x200
RFSW_ANT2_ADC="AP0_5"

DSRC_ANT1_MIN=0x0100
DSRC_ANT1_MAX=0x0200
DSRC_ANT1_ADC="AP0_8"

DSRC_ANT2_MIN=0x0100
DSRC_ANT2_MAX=0x0200
DSRC_ANT2_ADC="AP0_9"

DC_INPUT_MIN=0x0667 #10.8V (COUNT / 4095 * 26.98)
DC_INPUT_MAX=0x07D3 #13.2V (COUNT / 4095 * 26.98)
DC_INPUT_ADC="AP0_12"


tmpfile=$(mktemp)
# Reset tty just in case
tput reset > $RH850_TTY
# Set up RH850 device
stty -F $RH850_TTY $RH850_BAUD sane
#echo -e "ZH P11 1\r" > $RH850_TTY #Activate antenna 5V supply if not already

# Start reading ttyLP3
cat $RH850_TTY > $tmpfile & PID=$!

sleep 1

if [ "$2" == "loop" ] || [ "$3" == "loop" ] ; then
    tput smcup
fi


while true
do
  if [ "$2" == "loop" ] || [ "$3" == "loop" ] ; then
    tput ed
  fi
  
  # Send the command, and stop reading
  if [ "$1" == "dcin" ] ; then
    echo -e "ZA\r" > $RH850_TTY
  else
    echo -e "Za\r" > $RH850_TTY
  fi
  sleep 0.5

  # Get lines
  while IFS= read -r line; do
    if [ ! -z "${line}" ] ; then
	  lines="${lines}"$'\n'"${line}"
    fi
  done < $tmpfile
  lines=$(echo "$lines" | sed 2d)
  if [ "$2" == "loop" ] || [ "$3" == "loop" ] ; then
    tput clear
  fi
  if [ "$2" == "v" ] || [ "$3" == "v" ] ; then
    echo "$lines"
  fi
  > ${tmpfile}

  if [ "$1" == "dcin" ] || [ "$1" == "all" ] ; then
    adc_hex=$(get_adc_count $DC_INPUT_ADC)
    adc_dec=$((16#$adc_hex))
    dc_input=$(echo "scale=2; ($adc_dec/4095) *26.98" | bc -l)
    echo ""
    echo "DC Input Voltage"
    echo "$dc_input""V"
    result=$(check_value "0x"$adc_hex $DC_INPUT_MIN $DC_INPUT_MAX)
    if [ "$result" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : LOW"
    elif [ "$result" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : HIGH"
    else
      echo "RESULT = PASS"
    fi
  fi

  if [ "$1" == "cv2x" ] || [ "$1" == "all" ] ; then
    adc_hex=$(get_adc_count $CV2X_ANT1_ADC)
    diag1_result=$(check_value "0x"$adc_hex $CV2X_ANT1_MIN $CV2X_ANT1_MAX)
    adc_hex=$(get_adc_count $CV2X_ANT2_ADC)
    diag2_result=$(check_value "0x"$adc_hex $CV2X_ANT2_MIN $CV2X_ANT2_MAX)
    echo ""
    echo "C-V2X Antenna Status"
    echo "ANT1:"  
    if [ "$diag1_result" == "PASS" ] ; then
      echo "RESULT = PASS"
    elif [ "$diag1_result" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : SHORT CIRCUIT"
    elif [ "$diag1_result" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : OPEN CIRCUIT"
    else
      echo "RESULT = FAIL : UNKNOWN"
    fi 
    echo "ANT2:"
    if [ "$diag2_result" == "PASS" ] ; then
      echo "RESULT = PASS"
    elif [ "$diag2_result" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : SHORT CIRCUIT"
    elif [ "$diag2_result" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : OPEN CIRCUIT"
    else
      echo "RESULT = FAIL : UNKNOWN"
    fi 
  fi

  if [ "$1" == "dsrc" ] || [ "$1" == "all" ] ; then
    adc_hex=$(get_adc_count $DSRC_ANT1_ADC)
    diag1_result=$(check_value "0x"$adc_hex $DSRC_ANT1_MIN $DSRC_ANT1_MAX)
    adc_hex=$(get_adc_count $DSRC_ANT2_ADC)
    diag2_result=$(check_value "0x"$adc_hex $DSRC_ANT2_MIN $DSRC_ANT2_MAX)
    echo ""
    echo "DSRC Antenna Status"
    echo "ANT1:"  
    if [ "$diag1_result" == "PASS" ] ; then
      echo "RESULT = PASS"
    elif [ "$diag1_result" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : SHORT CIRCUIT"
    elif [ "$diag1_result" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : OPEN CIRCUIT"
    else
      echo "RESULT = FAIL : UNKNOWN"
    fi
    echo "ANT2:"
    if [ "$diag2_result" == "PASS" ] ; then
      echo "RESULT = PASS"
    elif [ "$diag2_result" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : SHORT CIRCUIT"
    elif [ "$diag2_result" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : OPEN CIRCUIT"
    else
      echo "RESULT = FAIL : UNKNOWN"
    fi 
  fi

  if [ "$1" == "rfsw" ] || [ "$1" == "all" ] ; then
    adc_hex=$(get_adc_count $RFSW_ANT1_ADC)
    diag1_result=$(check_value "0x"$adc_hex $RFSW_ANT1_MIN $RFSW_ANT1_MAX)
    adc_hex=$(get_adc_count $RFSW_ANT2_ADC)
    diag2_result=$(check_value "0x"$adc_hex $RFSW_ANT2_MIN $RFSW_ANT2_MAX)
    echo ""
    echo "RF Switch Antenna Status"
    echo "ANT1:"  
    if [ "$diag1_result" == "PASS" ] ; then
      echo "RESULT = PASS"
    elif [ "$diag1_result" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : SHORT CIRCUIT"
    elif [ "$diag1_result" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : OPEN CIRCUIT"
    else
      echo "RESULT = FAIL : UNKNOWN"
    fi
    echo "ANT2:"
    if [ "$diag2_result" == "PASS" ] ; then
      echo "RESULT = PASS"
    elif [ "$diag2_result" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : SHORT CIRCUIT"
    elif [ "$diag2_result" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : OPEN CIRCUIT"
    else
      echo "RESULT = FAIL : UNKNOWN"
    fi 
  fi

  if [ "$1" == "gps" ] || [ "$1" == "all" ] ; then
    adc_hex=$(get_adc_count $GPS_DIAG1_ADC)
    adc2_hex=$(get_adc_count $GPS_DIAG2_ADC)
    adc_hex_delta="$((0x$adc2_hex-0x$adc_hex))"
    diag1_result=$(check_value "0x"$adc_hex_delta $GPS_DIAG2_DIAG1_DELTA_MIN $GPS_DIAG2_DIAG1_DELTA_MAX)
    diag2_result=$(check_value "0x"$adc2_hex $GPS_DIAG2_MIN $GPS_DIAG2_MAX)
    echo ""
    echo "GNSS Antenna Status"
    if [ "$diag2_result" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : ANTENNA SUPPLY FAILURE"
    elif [ "$dig2_result" == "FAIL_HIGH" ] ; then
	  echo "RESULT = FAIL : ANTENNA SUPPLY OUT OF RANGE"
    elif [ "$diag1_result" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : OPEN CIRCUIT"
    elif [ "$diag1_result" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : SHORT CIRCUIT"
	  #echo "Deactivating antenna 5V supply."
      #echo "Run script again to reactivate and check for fault"
      #echo -e "ZL P11 1\r" > $RH850_TTY #Deactivate antenna 5V supply
    else
      echo "RESULT = PASS"
    fi
  fi

  echo ""
  lines=""
  if [ "$2" != "loop" ] && [ "$3" != "loop" ] ; then
     deactivate
  fi
done
