#!/bin/sh
# Date       / Author / Version  
# 11.12.2020 / SBaucom   / v2.0 
# 02.16.2022 / JStinnett / v2.1 Updated RST and DAV I/O mappings for RSU-5940
# 02.25.2022 / SBaucom   / v2.2 Added dsrc/ main board command line options


function usage
{
    echo "hsm_test.sh <option>"
    echo "options: main, dsrc, all"
    echo "     main: test the main board's HSM."
    echo "     dsrc: test the DSRC daughter board's HSM."
    echo "     all: test both board's HSM."

}


function test_dsrc_hsm
{
    RESULT=PASS

    dsrc_hsm_jcop=`v2xse-se-info -t /dev/spidev2.0 -d 416 -r 384 | grep "JCOP version"` 
    dsrc_hsm_jcop=$(echo $dsrc_hsm_jcop | cut -d':' -f2 | tr -d '[:space:]')
    dsrc_hsm_us_applet=`v2xse-se-info -t /dev/spidev2.0 -d 416 -r 384 | grep "US Applet"`
    dsrc_hsm_us_applet=$(echo $dsrc_hsm_us_applet | cut -d':' -f2 | tr -d '[:space:]')
    dsrc_hsm_eu_applet=`v2xse-se-info -t /dev/spidev2.0 -d 416 -r 384 | grep "EU Applet"`
    dsrc_hsm_eu_applet=$(echo $dsrc_hsm_eu_applet | cut -d':' -f2 | tr -d '[:space:]')

    # Test DSRC HSM JCOP
    echo "DSRC HSM JCOP version: $dsrc_hsm_jcop"
    if [[ "$dsrc_hsm_jcop" == "J5S2M0024BB70800" ]]; then 
        echo "DSRC HSM JCOP is the correct version."
    elif [[ "$dsrc_hsm_jcop" == "J5S2M001E0800800" ]]; then
        RESULT=FAIL
        echo "DSRC HSM JCOP must be updated. "
    else
        RESULT=FAIL
        echo "Error: DSRC HSM JCOP version is invalid "
    fi


    # Test DSRC HSM US Applet 
    echo "DSRC HSM US Applet version: $dsrc_hsm_us_applet"
    if [[ "$dsrc_hsm_us_applet" == "2.12.3" ]]; then 
        echo "DSRC HSM US Applet is the correct version." 
    elif [[ "$dsrc_hsm_us_applet" == "2.12.2" ]]; then 
        RESULT=FAIL
        echo "DSRC HSM US Applet must be updated. " 
    else
        RESULT=FAIL
        echo "Error: DSRC HSM US Applet version is invalid. " 
    fi 

    # Test DSRC HSM EU Applet
    echo "DSRC HSM EU Applet version: $dsrc_hsm_eu_applet"
    if [[ "$dsrc_hsm_eu_applet" == "2.12.3" ]]; then 
        echo "DSRC HSM EU Applet is the correct version." 
    elif [[ "$dsrc_hsm_eu_applet" == "2.12.2" ]]; then 
        RESULT=FAIL
        echo "DSRC HSM EU Applet must be updated. " 
    else
        RESULT=FAIL
        echo "Error: DSRC HSM EU Applet version is invalid. " 
    fi 

    echo "DSRC BOARD RESULT: $RESULT"
}

function test_main_hsm
{
    RESULT=PASS

    main_hsm_jcop=`v2xse-se-info -t /dev/spidev0.0 -d 406 -r 488 | grep "JCOP version"`
    main_hsm_jcop=$(echo $main_hsm_jcop | cut -d':' -f2 | tr -d '[:space:]')
    main_hsm_us_applet=`v2xse-se-info -t /dev/spidev0.0 -d 406 -r 488 | grep "US Applet"`
    main_hsm_us_applet=$(echo $main_hsm_us_applet | cut -d':' -f2 | tr -d '[:space:]')
    main_hsm_eu_applet=`v2xse-se-info -t /dev/spidev0.0 -d 406 -r 488 | grep "EU Applet"`
    main_hsm_eu_applet=$(echo $main_hsm_eu_applet | cut -d':' -f2 | tr -d '[:space:]')
	

    # Test Main Board HSM JCOP
    echo "Main Board HSM JCOP version: $main_hsm_jcop"
    if [[ "$main_hsm_jcop" == "J5S2M0024BB70800" ]]; then 
        echo "Main Board HSM JCOP is the correct version"
    elif [[ "$main_hsm_jcop" == "J5S2M001E0800800" ]]; then
        RESULT=FAIL
        echo "Main Board HSM JCOP must be updated. "
    else
        RESULT=FAIL
        echo "Error: Main Board HSM JCOP version is invalid "
    fi


    # Test Main Board HSM US Applet Version
    echo "Main Board HSM US Applet version: $main_hsm_us_applet"
    if [[ "$main_hsm_us_applet" == "2.12.3" ]]; then 
        echo "Main Board HSM US Applet is the correct version." 
    elif [[ "$main_hsm_us_applet" == "2.12.2" ]]; then 
        RESULT=FAIL
        echo "Main Board HSM US Applet must be updated. " 
    else
        RESULT=FAIL
        echo "Error: Main Board HSM US Applet version is invalid. " 
    fi 


    # Test Main Board HSM EU Applet Version
    echo "Main Board HSM EU Applet version: $main_hsm_eu_applet"
    if [[ "$main_hsm_eu_applet" == "2.12.3" ]]; then 
        echo "Main Board HSM EU Applet is the correct version." 
    elif [[ "$main_hsm_eu_applet" == "2.12.2" ]]; then 
        RESULT=FAIL
        echo "Main Board HSM EU Applet must be updated. " 
    else
        RESULT=FAIL
        echo "Error: Main Board HSM EU Applet version is invalid. " 
    fi 

    echo "MAIN BOARD HSM RESULT: $RESULT"
}


if [[  "$1" == "help" ]] || [[ -z "$1" ]]; then 
    usage
    exit
fi


if [[ "$1" == "dsrc" ]] || [[ "$1" == "all" ]]; then 
    dsrc_hsm=$(test_dsrc_hsm)
    echo "$dsrc_hsm"
fi

if [[ "$1" == "main" ]] || [[ "$1" == "all" ]]; then 
    main_hsm=$(test_main_hsm)
    echo "$main_hsm"
fi

if [[ "$dsrc_hsm" == *"FAIL"* ]] || [[ "$dsrc_hsm" == *"FAIL"* ]]; then 
    RESULTS="FAIL"
else
    RESULTS="PASS"
fi

echo ""
echo "Result: $RESULTS"



