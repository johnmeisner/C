#!/bin/sh
# Date       / Author / Version / Description
# 2022.03.08 / JStinnett   / v1.0    / Initial release
# 2022.06.28 / JStinnett   / v1.1    / Improvments for no response at high temperature
# 2022.07.07 / JStinnett   / v1.2    / Increase timeout to 7s and fix retry bug


export GPS=$1
export GNSS_RST="GNSS_RST_N"
export RETRY_COUNT="10"
export TIMEOUT="0.5s"
export UBX_ACK_ACK="b5620501"
export UBX_ACK_NAK="b5620500"
export GPTXT_RESETTING="526573657474696e67"
export UBX_PREAMBLE="B5 62"
export UBX_MON_VER_POLL="0A 04 00 00"
export UBX_DUMMY_MSG="06 09 0D 00 00 00 00 00 FF FF 00 00 00 00 00 00 00"

#counts number of sent messages up to the set number of retries 
#anything sent into the counter should fail with exit code 1 and pass with exit code 0
send_counter ()
{
SENT_COUNTER=0
until [ $SENT_COUNTER -eq $RETRY_COUNT ] || [ $ACK_COUNTER -gt "0" ] || "$@"
do
  SENT_COUNTER=$(($SENT_COUNTER+1))
  if [ $SENT_COUNTER -ge 1 ] && [ $SENT_COUNTER -le $(($RETRY_COUNT-1)) ] && [ $ACK_COUNTER -eq "0" ]
  then
    echo "ubx_chk_ver.sh: RETRYING"
  fi
done
}

send_ubx ()
{
echo "ubx_chk_ver.sh: Sending UBX message: "$@""
if [[ $@ == *"UBX-MON-VER-POLL"* ]]
then
  ubx_response=`timeout $TIMEOUT cat $GPS | xxd -p | grep -m1 -a -A 10 "b5620a04" | tr -d '\n' & echo $2 | xxd -r -p > $GPS`
  if [ -z "$ubx_response" ]
  then
    echo "ubx_chk_ver.sh: Response UBX-MON-VER is empty or timed out!"
    return 1
  fi
  ver_response=$(echo $ubx_response | grep -oP "b5620a04.{0,512}")
  echo "ubx_chk_ver.sh: UBX-MON-VER $ver_response"
  ver_length_hex=$( echo $ver_response | cut -c 9-10 )
  ver_length_lo=$(( 16#$ver_length_hex ))
  ver_length_hex=$( echo $ver_response | cut -c 11-12 )
  ver_length_hi=$(( 16#$ver_length_hex ))
  ver_length=$(( $ver_length_hi*256 + $ver_length_lo ))
  ver_length=$(( $ver_length * 2 ))
  echo ""
  response=$( echo $ver_response | cut -c 13-72 | sed 's/0*$//' | xxd -r -p )
  echo "swVersion=$response"
  response=$( echo $ver_response | cut -c 73-92 | sed 's/0*$//' | xxd -r -p )
  echo "hwVersion=$response"
  
  response=$( echo $ver_response | cut -c 93-$ver_length | sed 's/.\{2\}/& /g' | sed 's/00/0A/g' | sed 's/ //g')
  searchstring="0A"
  fwver=$( echo $response | awk -F0A '{print $1}' )
  size=${#fwver}
  fwver=$( echo $fwver | xxd -r -p )
  echo $fwver
  
  response=${response:$size}
  response=$( echo $response | xxd -r -p | tr '\n' ':' | tr -s ':' )
  protver=$( echo $response | cut -d':' -f2 )
  constellation=$( echo $response | cut -d':' -f3 )
  sbas=$( echo $response | cut -d':' -f4 )
  echo $protver
  echo $constellation
  echo $sbas
  echo ""
  ACK_COUNTER=$(($ACK_COUNTER+1))
else
  ubx_response=`timeout $TIMEOUT cat $GPS | xxd -p | grep -m1 -a -e "$UBX_ACK_ACK" -e "$UBX_ACK_NACK" -e "$GPTXT_RESETTING" & echo $2 | xxd -r -p > $GPS`
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
ACK_COUNTER=0
NAK_COUNTER=0

#validate the GPS device
if [[ -c $GPS ]]
then
  #prepare serial port
  for i in "${baud_array[@]}"
  do
    echo "ubx_chk_ver.sh: Trying baud rate $i"
    stty -F $GPS raw "$i"
    #Send dummy UBX-CFG-CFG (Save) message to test for baud rate using ACK
    send_counter send_ubx "UBX_DUMMY_MSG" "$(ubx_chksum "$UBX_DUMMY_MSG")"
    if [[ "$ACK_COUNTER" -gt "0" ]]
    then
      echo "ubx_chk_ver.sh: Found u-blox M8 at baud rate $i"  
      break
    fi
  done
  if [ "$ACK_COUNTER" -eq "0" ]
  then
    echo "ubx_chk_ver.sh: u-blox M8 device not found on $GPS"
    read -p "Reset u-blox M8? Press [ENTER] to continue, CTRL+C to exit"
    #u-blox M8 shuts downs UART RX if too frames with incorrect baud rate
    #Must reset to recover from this condition and activate UART RX 
    echo "ubx_chk_ver.sh: Resetting u-blox M8"
    /usr/local/dnutils/gpio_ctl.sh $GNSS_RST w 0
    sleep 1
    /usr/local/dnutils/gpio_ctl.sh $GNSS_RST w 1
    echo "ubx_chk_ver.sh: Starting ubx_chk_ver.sh again"
    main
  fi
 
  ACK_COUNTER=0
  NAK_COUNTER=0
  echo "ubx_chk_ver.sh: Checking u-blox M8 firmware version (UBX-MON-VER)..."
  send_counter send_ubx "UBX-MON-VER-POLL" "$(ubx_chksum "$UBX_MON_VER_POLL")"
  if [ "$ACK_COUNTER" -gt "0" ]; then
    echo "Test PASS"
  else
    echo "Test FAIL"
  fi
else
  echo -en "\n"
  echo "USAGE: ubx_chk_ver.sh <GPS Device>"
  echo "  Check u-blox M8 GNSS firmware version by polling UBX-MON-VER message"
  echo "  Script will try all baud rates starting with 115200"
  echo "  ex. ubx_chk_ver.sh /dev/ttyLP2"
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
