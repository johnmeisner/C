#!/bin/sh
# This script queries the RH850 for diagnostic status
# 1) dcin = DC Input Voltage (low/high/pass)
# 2) B2B50 = C-V2X Antenna 1/2 Status (open/short/pass)
# 3) B2B26 = B2B26 Antenna 1/2 Status (open/short/pass)  
# 4) gnss = GNSS Antenna Status (open/short/pass)
#
# Date       / Author      / Version  / Description
# 2020.12.02 / JStinnett   / v1.0     / Initial release
# 2020.12.07 / JStinnett   / v1.1     / Add support to query only DCIN ADC from RH850.  Bug fix to GPS logic
# 2020.12.09 / JStinnett   / v1.2     / Add loop support.  Update GPS antenna detection logic
# 2022.07.02 / JStinnett   / v2.0     / Updated script for RSU-5940.  Added checksum support option
# 2022.08.08 / JStinnett   / v2.1     / Speed up first measurement capture.  Compensate for ADC bias.
# 2023.09.26 / JStinnett   / v2.2     / Fix bug in GNSS delta voltage calculation.  Change GNSS ant delta voltage calibration

trap deactivate SIGINT

function usage ()
{
    echo "" 
    echo "RH850 Antenna Diagnostic and DC Input Status Check"
    echo "RH850_chk_status.sh [A(Antenna)] [0-999(Ant Period)] [D(DCIN)] [0-999(DCIN Period)] [C(checksum)] [L(loop)] [T[timeout count) [V(verbose)]"
    echo "ex: RH850_chk_status.sh A 50 D 10 L [Antenna ADC period = 5s, DCIN ADC period = 1s, loop measurements]"
    echo "ex: RH850_chk_status.sh A 25 D 0    [Antenna ADC period = 2.5s, DCIN ADC disabled, one-shot measurement]"
    echo ""
}

function deactivate () #kill cat PID and remove tmp file
{
    disown $PID
    kill -9 $PID
    rm $tmpfile
    if [ -n "$loop" ] ; then
      tput rmcup 
    fi
    echo -e "ZA,0" > $RH850_TTY  #deactivate antenna ADC broadcast on RH850
    echo -e "ZD,0" > $RH850_TTY  #deactivate DC input ADC broadcast on RH850
    tput reset > $RH850_TTY
    exit 0
}
  
# This function extract the ADC counts from the RH850 response.
function get_adc_count () 
{
    local res=$(echo $lines | awk -F "${1}" '{print $2}' | cut -c 1-5)
    echo $res
}

# This function takes a hex value (1) and verifies that the value is in between the minimum value (2) and the max value (3). 
function check_value ()
{
    [[ ($1 -ge $2) && ($1 -le $3) ]] && echo PASS
    [[ ($1 -lt $2) ]] && echo FAIL_LOW
    [[ ($1 -gt $3) ]] && echo FAIL_HIGH
}

function adc_chksum ()
{
    SUM1=0
    SUM2=0
    j=0
    hexstring=$(xxd -i -u -g 1 <<< "$1")
    #hexstring=${hexstring::-6}
    hexstring=${hexstring::-41}
    #echo $hexstring
    #for i in $hexstring
    for i in ${hexstring//,/ }
    do
        #printf "%02X " "$i"
        SUM1=$(printf '0x%X' $((SUM1 + i)))
        SUM1=$(printf '0x%X' $((SUM1%255)))
        SUM2=$(printf '0x%X' $((SUM2 + SUM1)))
        SUM2=$(printf '0x%X' $((SUM2%255)))
    done
    adc_chksum_calc=$(printf '%04X' $((SUM2*256 + SUM1)))
    echo $adc_chksum_calc	
}


# Constants
RH850_TTY="/dev/ttyLP3"
RH850_BAUD="115200"

GPS_DIAG2_DIAG1_DELTA_MIN="0x0085"  #3mA antenna load (110mV delta)
GPS_DIAG2_DIAG1_DELTA_MAX="0x089E"  #50mA antenna load (1.83V delta)
GPS_DIAG1_ADC="AP0_0="

GPS_DIAG2_MIN="0x0ACD"  #4.9V
GPS_DIAG2_MAX="0x0D7C"  #5.1V
GPS_DIAG2_ADC="AP0_1="

B2B50_ANT1_MIN="0x0100"  #C-V2X ANT1
B2B50_ANT1_MAX="0x0200"
B2B50_ANT1_ADC_OFFSET="0x0"
B2B50_ANT1_ADC="AP0_2="

B2B50_ANT2_MIN="0x0100"  #C-V2X ANT2
B2B50_ANT2_MAX="0x0200"
B2B50_ANT2_ADC_OFFSET="0x0"
B2B50_ANT2_ADC="AP0_3="

B2B26_ANT1_MIN="0x0100"  #DSRC ANT1
B2B26_ANT1_MAX="0x0200"
B2B26_ANT1_ADC_OFFSET="0x0"
B2B26_ANT1_ADC="AP0_4="

B2B26_ANT2_MIN="0x0100"  #DSRC ANT2
B2B26_ANT2_MAX="0x0200"
B2B26_ANT2_ADC_OFFSET="0x0"
B2B26_ANT2_ADC="AP0_5="

DCIN_MIN="0x063A" #10.8V with 300mV drop for ideal diode (COUNT / 4095 * 26.98)
DCIN_MAX="0x07A6" #13.2V with 300mV drop for ideal diode (COUNT / 4095 * 26.98)
DCIN_ADC_OFFSET="0x39"
DCIN_ADC="AP0_12="

ANT_ADC_TIMEOUT_LIMIT=10
DCIN_ADC_TIMEOUT_LIMIT=4

tmpfile=$(mktemp)
# Reset tty just in case
tput reset > $RH850_TTY

# Set up RH850 device
stty -F $RH850_TTY $RH850_BAUD sane
	  
sleep 0.5	  
if [ "$1" == "A" ] ; then  
  if [[ "$2" =~ ^[0-9]+$ ]] && [ "$2" -le 999 ] ; then
    echo -e "ZA,${2}" > $RH850_TTY  #Enable antenna ADC measurements
  else
    usage
    exit 0
  fi
else
  usage
  exit 0 
fi

if [ "$3" == "D" ] ; then
  if [[ "$4" =~ ^[0-9]+$ ]] && [ "$4" -le 999 ] ; then
    echo -e "ZD,${4}" > $RH850_TTY  #Enable DC input ADC measurements
  else
    usage
    exit 0
  fi
else
  usage
  exit 0 
fi

if [ "$5" == "L" ] || [ "$6" == "L" ] || [ "$7" == "L" ] || [ "$8" == "L" ]  ; then  #Check for loop option
  loop="1"
fi

if [ "$5" == "V" ] || [ "$6" == "V" ] || [ "$7" == "V" ] || [ "$8" == "V" ]  ; then  #Check for verbose option
  verbose="1"
fi

if [ "$5" == "C" ] || [ "$6" == "C" ] || [ "$7" == "C" ] || [ "$8" == "C" ] ; then  #Check for checksum option
  chksum="1"
fi

if [ "$5" == "T" ] || [ "$6" == "T" ] || [ "$7" == "T" ] || [ "$8" == "T" ] ; then  #Check for checksum option
  timeout="1"
fi

#Set limits for ADC message timeout from RH850.  
#Clear results from screen during timeout
dcin_adc_timeout=$DCIN_ADC_TIMEOUT_LIMIT
b2b50_1_adc_timeout=$ANT_ADC_TIMEOUT_LIMIT
b2b50_2_adc_timeout=$ANT_ADC_TIMEOUT_LIMIT
b2b26_1_adc_timeout=$ANT_ADC_TIMEOUT_LIMIT
b2b26_2_adc_timeout=$ANT_ADC_TIMEOUT_LIMIT
gnss_adc_timeout=$ANT_ADC_TIMEOUT_LIMIT
chksum_fail_cnt="0"
adc_chksum_calc_fail=""
adc_chksum_fail=""

tput smcup
# Start reading from RH850 ttyLP3 serial
cat $RH850_TTY > $tmpfile & PID=$!


while true; do
  tput ed

  # Get lines from tmp file for processing
  while IFS= read -r line; do
    if [ ! -z "${line}" ] ; then
	  if [ -n $chksm ] ; then  #Process checksum is input argument is provided
        adc_chksum=${line: -4}  #Remove 16-bit fletcher checksum from payload
	    adc_chksum_calc=$(adc_chksum $line)  #Pass line to checksum function 
	    if [ "$adc_chksum_calc" == "$adc_chksum" ]; then  #Check if calculated sum matches payload sum
	      lines="${lines}"$'\n'"${line}"  #If checksum matches, add line for processing
		else
		  chksum_fail_cnt=$((chksum_fail_cnt+1))
		  adc_chksum_calc_fail=$adc_chksum_calc
		  adc_chksum_fail=$adc_chksum
		fi
	  else
	    lines="${lines}"$'\n'"${line}"  #Add line for processing
	  fi
    fi
  done < $tmpfile
  
  > ${tmpfile}
  tput clear
  
  echo "----RH850 Antenna Diagnostic and DC Input Status Check----"
  echo ""
  echo "### DC Input Voltage ###"
  
  adc_hex=$(get_adc_count $DCIN_ADC)
  if [ -n "$adc_hex" ] ; then
    dcin_adc_timeout="0"
    adc_hex=`printf "%#x\n" $(($adc_hex + $DCIN_ADC_OFFSET))`
  fi
  if [ -z "$adc_hex" ] ; then
    adc_hex=$dcin_adc_hex_prev 
    dcin_adc_timeout=$((dcin_adc_timeout+1))
  fi
  if [ -n "$verbose" ] && [ "$dcin_adc_timeout" -le $DCIN_ADC_TIMEOUT_LIMIT ]; then
    echo "${DCIN_ADC}${adc_hex}"
  elif [ -n "$verbose" ] ; then
    echo "$DCIN_ADC"
  fi
  if [ "$dcin_adc_timeout" -le $DCIN_ADC_TIMEOUT_LIMIT ] ; then
    adc_dec=$((adc_hex))
    dc_input=$(echo "scale=4; ($adc_dec/4095) *26.981" | bc -l)
    echo ${dc_input::-2}"V"
    result=$(check_value $adc_dec $DCIN_MIN $DCIN_MAX)
    if [ "$result" == "FAIL_LOW" ] ; then
    echo "RESULT = FAIL : LOW"
    elif [ "$result" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : HIGH"
    else
      echo "RESULT = PASS"
    fi
      dcin_adc_hex_prev=$adc_hex
  else
    echo ""
    echo ""
    timeout_cnt=$((timeout_cnt+1))
  fi
  
  
  echo ""
  echo "### GNSS Antenna Status ###"
  
  adc_hex=$(get_adc_count $GPS_DIAG1_ADC)
  adc2_hex=$(get_adc_count $GPS_DIAG2_ADC)
  if [ -n "$adc_hex" ] ; then
    gnss_adc_timeout="0"
  fi
  if [ -z "$adc_hex" ] || [ -z "$adc2_hex" ] ; then
    adc_hex=$gnss_adc_hex_prev
    adc2_hex=$gnss_adc2_hex_prev
    gnss_adc_timeout=$((gnss_adc_timeout+1))
  fi
  if [ -n "$verbose" ] && [ "$gnss_adc_timeout" -le $ANT_ADC_TIMEOUT_LIMIT ]; then
    echo "${GPS_DIAG1_ADC}${adc_hex}"
    echo "${GPS_DIAG2_ADC}${adc2_hex}"
  elif [ -n "$verbose" ] ; then
    echo "$GPS_DIAG1_ADC"
    echo "$GPS_DIAG2_ADC"
  fi
  if [ "$gnss_adc_timeout" -le $ANT_ADC_TIMEOUT_LIMIT ] ; then
    adc_hex_delta=$(printf '%#x\n' $(( $adc2_hex-$adc_hex )))
    result1=$(check_value $adc_hex_delta $GPS_DIAG2_DIAG1_DELTA_MIN $GPS_DIAG2_DIAG1_DELTA_MAX)
    result2=$(check_value $adc2_hex $GPS_DIAG2_MIN $GPS_DIAG2_MAX)
    if [ "$result2" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : ANTENNA SUPPLY FAILURE"
    elif [ "$result2" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : ANTENNA SUPPLY OUT OF RANGE"
    elif [ "$result1" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : OPEN CIRCUIT"
    elif [ "$result1" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : SHORT CIRCUIT"
    else
      echo "RESULT = PASS"
    fi
    gnss_adc_hex_prev=$adc_hex
    gnss_adc2_hex_prev=$adc2_hex
  else
    echo ""
    timeout_cnt=$((timeout_cnt+1))
  fi
  
  
  echo ""
  echo "### B2B50 Antenna Status ###"
  echo "ANT1:"
  adc_hex=$(get_adc_count $B2B50_ANT1_ADC)
  if [ -n "$adc_hex" ] ; then
    b2b50_1_adc_timeout="0"
    adc_hex=`printf "%#X\n" $(($adc_hex + B2B50_ANT1_ADC_OFFSET))`
  fi
  if [ -z "$adc_hex" ] ; then
    adc_hex=$b2b50_1_adc_hex_prev
    b2b50_1_adc_timeout=$((b2b50_1_adc_timeout+1))	
  fi
  if [ -n "$verbose" ] && [ "$b2b50_1_adc_timeout" -le $ANT_ADC_TIMEOUT_LIMIT ]; then
    echo "${B2B50_ANT1_ADC}${adc_hex}"
  elif [ -n "$verbose" ] ; then
    echo "$B2B50_ANT1_ADC"
  fi
  if [ "$b2b50_1_adc_timeout" -le $ANT_ADC_TIMEOUT_LIMIT ] ; then
    result=$(check_value $adc_hex $B2B50_ANT1_MIN $B2B50_ANT1_MAX)
    if [ "$result" == "PASS" ] ; then
      echo "RESULT = PASS"
    elif [ "$result" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : SHORT CIRCUIT"
    elif [ "$result" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : OPEN CIRCUIT"
    else
      echo "RESULT = FAIL : UNKNOWN"
    fi
      b2b50_1_adc_hex_prev=$adc_hex
  else
    echo ""
    timeout_cnt=$((timeout_cnt+1))
  fi	

  
  echo "ANT2:"
  adc_hex=$(get_adc_count $B2B50_ANT2_ADC)
  if [ -n "$adc_hex" ] ; then
    b2b50_2_adc_timeout="0"
    adc_hex=`printf "%#X\n" $(($adc_hex + B2B50_ANT2_ADC_OFFSET))`
  fi
  if [ -z "$adc_hex" ] ; then
    adc_hex=$b2b50_2_adc_hex_prev 
    b2b50_2_adc_timeout=$((b2b50_2_adc_timeout+1))	
  fi
  if [ -n "$verbose" ] && [ "$b2b50_2_adc_timeout" -le $ANT_ADC_TIMEOUT_LIMIT ]; then
    echo "${B2B50_ANT2_ADC}${adc_hex}"
  elif [ -n "$verbose" ] ; then
    echo "$B2B50_ANT2_ADC"
  fi
  if [ "$b2b50_2_adc_timeout" -le $ANT_ADC_TIMEOUT_LIMIT ] ; then
    result=$(check_value $adc_hex $B2B50_ANT2_MIN $B2B50_ANT2_MAX)
    if [ "$result" == "PASS" ] ; then
      echo "RESULT = PASS"
    elif [ "$result" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : SHORT CIRCUIT"
    elif [ "$result" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : OPEN CIRCUIT"
    else
      echo "RESULT = FAIL : UNKNOWN"
    fi 
    b2b50_2_adc_hex_prev=$adc_hex
  else
    echo ""
    timeout_cnt=$((timeout_cnt+1))
  fi

  
  echo ""
  echo "### B2B26 Antenna Status ###"
  echo "ANT1:"
  adc_hex=$(get_adc_count $B2B26_ANT1_ADC)
  if [ -n "$adc_hex" ] ; then
    b2b26_1_adc_timeout="0"
    adc_hex=`printf "%#X\n" $(($adc_hex + B2B26_ANT1_ADC_OFFSET))`
  fi
  if [ -z "$adc_hex" ] ; then
    adc_hex=$b2b26_1_adc_hex_prev 
    b2b26_1_adc_timeout=$((b2b26_1_adc_timeout+1))
  fi
  if [ -n "$verbose" ] && [ "$b2b26_1_adc_timeout" -le $ANT_ADC_TIMEOUT_LIMIT ]; then
    echo "${B2B26_ANT1_ADC}${adc_hex}"
  elif [ -n "$verbose" ] ; then
    echo "$B2B26_ANT1_ADC"
  fi
  if [ "$b2b26_1_adc_timeout" -le $ANT_ADC_TIMEOUT_LIMIT ] ; then
    result=$(check_value $adc_hex $B2B26_ANT1_MIN $B2B26_ANT1_MAX)  
    if [ "$result" == "PASS" ] ; then
      echo "RESULT = PASS"
    elif [ "$result" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : SHORT CIRCUIT"
    elif [ "$result" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : OPEN CIRCUIT"
    else
      echo "RESULT = FAIL : UNKNOWN"
    fi
    b2b26_1_adc_hex_prev=$adc_hex
  else
    echo ""
    timeout_cnt=$((timeout_cnt+1))
  fi
  
  
  echo "ANT2:"
  adc_hex=$(get_adc_count $B2B26_ANT2_ADC)
  if [ -n "$adc_hex" ] ; then
    b2b26_2_adc_timeout="1"
    adc_hex=`printf "%#X\n" $(($adc_hex + B2B26_ANT2_ADC_OFFSET))`
  fi
  if [ -z "$adc_hex" ] ; then
    adc_hex=$b2b26_2_adc_hex_prev 
    b2b26_2_adc_timeout=$((b2b26_2_adc_timeout+1))
  fi
  if [ -n "$verbose" ] && [ "$b2b26_2_adc_timeout" -le $ANT_ADC_TIMEOUT_LIMIT ]; then
    echo "${B2B26_ANT2_ADC}${adc_hex}"
  elif [ -n "$verbose" ] ; then
    echo "$B2B26_ANT2_ADC"
  fi
  if [ "$b2b26_2_adc_timeout" -le $ANT_ADC_TIMEOUT_LIMIT ] ; then
    result=$(check_value $adc_hex $B2B26_ANT2_MIN $B2B26_ANT2_MAX)
    if [ "$result" == "PASS" ] ; then
      echo "RESULT = PASS"
    elif [ "$result" == "FAIL_LOW" ] ; then
      echo "RESULT = FAIL : SHORT CIRCUIT"
    elif [ "$result" == "FAIL_HIGH" ] ; then
      echo "RESULT = FAIL : OPEN CIRCUIT"
    else
      echo "RESULT = FAIL : UNKNOWN"
    fi
      b2b26_2_adc_hex_prev=$adc_hex
  else
    echo ""
    timeout_cnt=$((timeout_cnt+1))
  fi	
  
  if [ -n "$chksum" ] ; then
    echo ""
    echo "Checksum Fail Count = $chksum_fail_cnt"
    echo "Calculated Checksum Fail = $adc_chksum_calc_fail"
    echo "Checksum Fail = $adc_chksum_fail"
  fi
  
  if [ -n "$timeout" ] ; then
    echo ""
    echo "Timeout Count = $timeout_cnt"
  fi
  
  sleep 0.5
  lines=""
 
  if [ -z "$loop" ] ; then
    #echo "Trying to stop loop"
    if [ "$dcin_adc_timeout" -le $DCIN_ADC_TIMEOUT_LIMIT ] && [ "$b2b50_1_adc_timeout" -le $ANT_ADC_TIMEOUT_LIMIT ] ; then
      #echo "Deactivating!!!!"
      deactivate
    fi
  fi
done
