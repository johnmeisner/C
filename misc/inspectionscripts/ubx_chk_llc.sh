#!/bin/sh
# Date       / Author / Version / Description
# 2019.10.24 / JStinnett   / v1.0    / Initial release
# 2022.02.16 / JStinnett   / v1.1    / Updated expected response for RSU-5940


export GPS=$1
export GNSS_RST="GNSS_RST_N"
export RETRY_COUNT="2"
export UBX_ACK_ACK="b5620501"
export UBX_ACK_NAK="b5620500"
export GPTXT_RESETTING="526573657474696e67"
export UBX_PREAMBLE="B5 62"
export UBX_MON_LLC_POLL="0A 0D 00 00"
export UBX_MON_LLC_RESPONSE="b5620a0d1800fffffffffdffffffffffffffffffffffe5ffffff6fffffff6b83"
export UBX_DUMMY_MSG="06 09 0D 00 00 00 00 00 FF FF 00 00 00 00 00 00 17"

#counts number of sent messages up to the set number of retries
#anything sent into the counter should fail with exit code 1 and pass with exit code 0
send_counter ()
{
SENT_COUNTER=0
until [ $SENT_COUNTER -eq $RETRY_COUNT ] && echo "$(tput setaf 1)ERROR - MESSAGE NOT ACKNOWLEDGED$(tput sgr0)" &&  ERROR_COUNTER=$(($ERROR_COUNTER+1)) || "$@"
do
  SENT_COUNTER=$(($SENT_COUNTER+1))
  echo "ubx_chk_llc.sh: NOT ACKNOWLEDGED"
  if [ $SENT_COUNTER -ge 1 ] && [ $SENT_COUNTER -le $(($RETRY_COUNT-1)) ]
  then
    echo "ubx_chk_llc.sh: RETRYING"
    TIMEOUT="1s"
  fi
done
}

send_ubx ()
{
echo "ubx_chk_llc.sh: Sending UBX message: "$@""
if [[ $@ == *"UBX-MON-LLC-POLL"* ]]
then
  file_response="$UBX_MON_LLC_RESPONSE"
  ubx_response=`timeout 4s cat $GPS | xxd -p | grep -m1 -a -A 10 "b5620a0d" | tr -d '\n' & echo $2 | xxd -r -p > $GPS`
  llc_response=$(echo $ubx_response | grep -oP "b5620a0d.{0,56}")
  echo "ubx_chk_llc.sh: UBX-MON-LLC (Expected) $file_response"
  echo "ubx_chk_llc.sh: UBX-MON-LLC (Response) $llc_response"
  if [[ $llc_response == $file_response ]]
  then
    echo "ubx_chk_llc.sh: UBX-MON-LLC low-level configuration match!"
    ACK_COUNTER=$(($ACK_COUNTER+1))
    return
  else
    echo "ubx_chk_llc.sh: UBX-MON-LLC low-level configuration mismatch!"
    NAK_COUNTER=$(($NAK_COUNTER+1))
  fi
else
  ubx_response=`timeout $TIMEOUT cat $GPS | xxd -p | grep -m1 -a -e "$UBX_ACK_ACK" -e "$UBX_ACK_NACK" -e "$GPTXT_RESETTING" & echo $2 | xxd -r -p > $GPS`
fi

if [[ $ubx_response == *$UBX_ACK_ACK* ]]
then
  echo "$(tput setaf 2)ACKNOWLEDGED$(tput sgr0)"
  ACK_COUNTER=$(($ACK_COUNTER+1))
elif [[ $ubx_response == *$GPTXT_RESETTING* ]] 
then
  echo "$(tput setaf 2)ACKNOWLEDGED$(tput sgr0)"
  ACK_COUNTER=$(($ACK_COUNTER+1))
elif [[ $ubx_response == *$UBX_ACK_NAK* ]] 
then
  echo "ubxconfig.sh: UBX response - $ubx_response"
  echo "$(tput setaf 3)REJECTED$(tput sgr0)"
  NAK_COUNTER=$(($NAK_COUNTER+1))
else
  return 1
fi
}

ubx_chksum ()
#############################################################################################################################
###Fletcher checksum calculator adapted from: http://www.aeronetworks.ca/2014/07/fletcher-checksum-calculator-in-bash.html###
#############################################################################################################################
{
SUM=0
FLETCHER=0
j=0

printf "$UBX_PREAMBLE "

for i in $1
do
 j=$(echo "ibase=16;$i" | bc)
 printf "%02X " "$j"
 SUM=$(echo "$SUM + $j" | bc)
 SUM=$(echo "$SUM%256" | bc)

 FLETCHER=$(echo "$FLETCHER + $SUM" | bc)
 FLETCHER=$(echo "$FLETCHER%256" | bc)
done

printf "%02X " "$SUM"
printf "%02X\n" "$FLETCHER"
}

main ()
{
#baud array and baud array hex are correlated. If array entry order changed, make sure to update both arrays
declare -a baud_array=("115200" "9600" "230400" "460800" "921600" "4800" "19200" "38400" "57600" ) 
TIMEOUT="1.0s"
ACK_COUNTER=0
NAK_COUNTER=0

#validate the GPS device
if [[ -c $GPS ]]
then
  #prepare serial port
  for i in "${baud_array[@]}"
  do
    echo "ubx_chk_llc.sh: Trying baud rate $i"
    stty -F $GPS raw "$i"
    #Send dummy UBX-CFG-CFG (Save) message to test for baud rate using ACK
    send_counter send_ubx "UBX_DUMMY_MSG" "$(ubx_chksum "$UBX_DUMMY_MSG")"
    if [[ "$ACK_COUNTER" -gt "0" ]]
    then
      echo "ubx_chk_llc.sh: Found u-blox M8 at baud rate $i"  
      break
    fi
  done
  if [ "$ACK_COUNTER" -eq "0" ]
  then
    echo "ubx_chk_llc.sh: u-blox M8 device not found on $GPS"
    read -p "Reset u-blox M8? Press [ENTER] to continue, CTRL+C to exit"
    #u-blox M8 shuts downs UART RX if too frames with incorrect baud rate
    #Must reset to recover from this condition and activate UART RX 
    echo "ubx_chk_llc.sh: Resetting u-blox M8"
    /usr/src/wnc/scripts/gpio_ctl.sh $GNSS_RST_N w 0
    sleep 1
    /usr/src/wnc/scripts/gpio_ctl.sh $GNSS_RST_N w 1
    echo "ubx_chk_llc.sh: Starting ubx_chk_llc.sh again"
    main
  fi
 
    ACK_COUNTER=0
    NAK_COUNTER=0
    echo "ubx_chk_llc.sh: Checking u-blox M8 Low Level Configuration (UBX-MON-LLC)..."
    send_counter send_ubx "UBX-MON-LLC-POLL" "$(ubx_chksum "$UBX_MON_LLC_POLL")"
    if [ "$ACK_COUNTER" -ge "1" ]
    then
        echo ""
        echo "Test PASS"
        exit 0
    elif [ $NAK_COUNT" -ge $RETRY_COUNT" ]
    then
        echo ""
        echo "Test FAIL"
        exit 1
    fi
else
  echo -en "\n"
  echo "USAGE: ubx_chk_llc.sh <GPS Device>"
  echo "  Check for correct low-level configuration for u-blox M8 GNSS"
  echo "  by polling UBX-MON-LLC message"
  echo "  Script will try all baud rates starting with 115200"
  echo "  ex. ubx_chk_llc.sh /dev/ttyLP2"
  return 1
fi
}

#check for dos2unix
if ! command -v dos2unix > /dev/null 2>&1
then
  echo "Please install dos2unix!"
  exit 2
fi

#check for bc
if ! command -v bc > /dev/null 2>&1
then
  echo "Please install bc!"
  exit 2
fi

#run
main

exit $ERROR_COUNTER
