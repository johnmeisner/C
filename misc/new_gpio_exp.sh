#!/bin/sh
echo " new_gpio_exp.sh:"
echo " Version : v1.2"
echo " Note:"
echo " 2020/05/28 Ignore some gpio pins that have been configured in .dtsi"
echo " 2020/08/26 Clean up new_gpio_exp.sh.  Now for C-sample only."

#gpiochip11: GPIOs 176-191, parent: i2c/6-0021, pca9555, can sleep:
#IO exp I2C addr:0x21
PIN_NAD_PCIE_CLK_REQ_N_3V3_CN=176
PIN_CV2X_RF_SEL=176
PIN_ENET_INT_N=177
PIN_VCONN_FAULT_N=178
PIN_VDD_CAM_6V_Fault_N=179
PIN_VDD_CAM_PWR_EN=180
PIN_HDMI_5V0_EN=181  #set in kernel
PIN_HDMI_3V3_EN=182  #set in kernel
PIN_iMX8_BT_WAKE_HOST=183  #set in DTS
PIN_GPIO3_IMX8=184
PIN_FAN_FAULT_N=185
PIN_UART_SEL=186
PIN_iMX8_BT_WAKE_SLAVE=187  #set in DTS
PIN_USB_UART_SEL=188
PIN_WBT_PWR_EN=189  #set in DTS
PIN_NAD_PCIE_WAKEUP_N_3V3_CN=190
PIN_DSRC_RF_SEL=190
PIN_PCIE_SW_PWR_EN=191

#gpiochip10: GPIOs 192-207, parent: i2c/6-0020, pca9555, can sleep:
#IO exp I2C addr:0x20
PIN_RH850_CAN1_ERR_N=192
PIN_RH850_CAN2_ERR_N=193
PIN_HDMI_5V0_FLT_N=194
PIN_RH850_CAN_STB_N=195  #set in DTS
PIN_RH850_CAN_EN=196  #set in DTS
PIN_GPIO2_IMX8=197
PIN_DSRC_PWR_EN=198  #set in kernel
PIN_DSRC_SPI_SEL=199  #set in kernel
PIN_GPIO1_IMX8=200
PIN_UBLOX_GYRO_EN=201  #set in kernel
PIN_TYPEC_INT_IN=202
PIN_Typec_SS_MUX_EN=203
PIN_GPIO_DRIVER_EN=204  #set in kernel
PIN_IMX8_PCIE_PWR_SAV=205  #set in kernel
PIN_IMX8_HSM_RST_3V3=206  #set in kernel
PIN_DSRC_RST_N=207  #set in kernel

#gpiochip9: GPIOs 208-223, parent: i2c/4-0022, pca9555, can sleep:
#IO exp I2C addr:0x22
PIN_UBLOX_USB_PATH_SEL=208
PIN_USB_PWR_EN=209
PIN_SXF1800_GPIO_0=210
PIN_MDM_PMD_GPIO3=211
PIN_USB_HUB_RST_N_1V8=212  #set in kernel
PIN_MDM_PMD_GPIO1=213
PIN_CAM_GENERAL_IO0=214
PIN_VCC_FAN_V12P0_EN=215
#PIN_iMX8_BT_RF_KILL_1V8=216
#PIN_iMX8_WLAN_RF_KILL_1V8=217
#PIN_C_V2X_GYRO_EN=218
PIN_GNSS_SEL2=219
PIN_GNSS_SEL1=220
PIN_MDM_GPIO22=221
PIN_IMX_PMD_CBLPWR_N=222  #set in kernel
PIN_CAM_DES_EN=223

function usage
{
    echo "[Initialize gpio pins of expander]"
    echo "    new_gpio_exp.sh init"
    echo "[Read/Write gpio pin of expander]"
    echo "    Read : new_gpio_exp.sh pin_name r"
    echo "    Write: new_gpio_exp.sh pin_name w [0|1]"
    echo "              NAD_PCIE_CLK_REQ_N_3V3_CN, ENET_INT_N, VCONN_FAULT_N, VDD_CAM_6V_Fault_N, VDD_CAM_PWR_EN, HDMI_5V0_EN"
    echo "              HDMI_3V3_EN, iMX8_BT_WAKE_HOST, GPIO3_IMX8, FAN_FAULT_N, UART_SEL, iMX8_BT_WAKE_SLAVE, USB_UART_SEL"
    echo "              WBT_PWR_EN, NAD_PCIE_WAKEUP_N_3V3_CN, PCIE_SW_PWR_EN, RH850_CAN1_ERR_N, RH850_CAN1_ERR_N, RH850_CAN2_ERR_N"
    echo "              HDMI_5V0_FLT_N=194, RH850_CAN_STB_N, RH850_CAN_EN, GPIO2_IMX8, DSRC_PWR_EN, DSRC_SPI_SEL, GPIO1_IMX8"
    echo "              UBLOX_GYRO_EN, TYPEC_INT_IN, Typec_SS_MUX_EN, GPIO_DRIVER_EN, IMX8_PCIE_PWR_SAV, IMX8_HSM_RST_3V3, DSRC_RST_N"
    echo "              UBLOX_USB_PATH_SEL, USB_PWR_EN, SXF1800_GPIO_0, MDM_PMD_GPIO3, USB_HUB_RST_N_1V8, MDM_PMD_GPIO1, CAM_GENERAL_IO0"
    echo "              VCC_FAN_V12P0_EN, iMX8_BT_RF_KILL_1V8, iMX8_WLAN_RF_KILL_1V8, C_V2X_GYRO_EN, GNSS_SEL2, GNSS_SEL1, MDM_GPIO22"
    echo "              IMX_PMD_CBLPWR_N, CAM_DES_EN"
    echo " "
    echo "    Note: For RSU - NAD_PCIE_CLK_REQ_N_3V3_CN = CV2X_RF_SEL"
    echo "                    NAD_PCIE_WAKEUP_N_3V3_CN = DSRC_RF_SEL"

    echo "[Help]"
    echo "new_gpio_exp.sh help"
}


function init
{
    #I2C addr: 0x20
    num=0
    

    #I2C addr: 0x21
    num=0
    for (( pin=176; pin<192; pin=pin+1 ))
        do
        if [ "$pin" == "$PIN_iMX8_BT_WAKE_HOST" ] || [ "$pin" == "$PIN_iMX8_BT_WAKE_SLAVE" ] || [ "$pin" == "$PIN_WBT_PWR_EN" ]; then
            echo "IO Exp GPIO Pin $pin is declared in dts file!!"
        else
        	if [ ! -d /sys/class/gpio/gpio$pin ] ; then
            	echo $pin > /sys/class/gpio/export
        	fi
        	#Handle direction
        	if [ "$pin" == "$PIN_ENET_INT_N" ] || [ "$pin" == "$PIN_VCONN_FAULT_N" ] || [ "$pin" == "$PIN_VDD_CAM_6V_Fault_N" ] || [ "$pin" == "$PIN_iMX8_BT_WAKE_HOST" ] || [ "$pin" == "$PIN_GPIO3_IMX8" ] || [ "$pin" == "$PIN_FAN_FAULT_N" ]; then
           		# Direction is input
            	echo in > /sys/class/gpio/gpio$pin/direction
            else
				if [ "$pin" == "$PIN_PCIE_SW_PWR_EN" ] || [ "$pin" == "$PIN_HDMI_5V0_EN" ] || [ "$pin" == "$PIN_HDMI_3V3_EN" ] ; then
            		echo "IO Exp GPIO $pin is already set in kernel!!"
				else
					# Direction is output
                	echo out > /sys/class/gpio/gpio$pin/direction
                	# Modified by Kevin 20181225 -- 0010417: [Derek][GPIO] To make default value of USB_UART_SEL to 1
                	if [ "$pin" == "$PIN_UART_SEL" ] || [ "$pin" == "$PIN_USB_UART_SEL" ]; then
                		# Default output is high  
                		echo 1 > /sys/class/gpio/gpio$pin/value
                	else
                		# Default output is low
                    	echo 0 > /sys/class/gpio/gpio$pin/value
                	fi
            	fi
        	fi
		fi
        num=$(($num + 1))
    done    

    for (( pin=192; pin<208; pin=pin+1 ))
        do
	if [ "$pin" == "$PIN_RH850_CAN_STB_N" ] || [ "$pin" == "$PIN_RH850_CAN_EN" ] ; then
            echo "IO Exp GPIO Pin $pin is declared in dts file!!"
        else
		if [ ! -d /sys/class/gpio/gpio$pin ] ; then
            	    echo $pin > /sys/class/gpio/export
        	fi
        	#Handle direction
        	if [ "$pin" == "$PIN_RH850_CAN1_ERR_N" ] || [ "$pin" == "$PIN_RH850_CAN2_ERR_N" ] || [ "$pin" == "$PIN_HDMI_5V0_FLT_N" ] || [ "$pin" == "$PIN_GPIO2_IMX8" ] || [ "$pin" == "$PIN_GPIO1_IMX8" ] || [ "$pin" == "$PIN_TYPEC_INT_N" ] ; then
            	# Direction is input
            		echo in > /sys/class/gpio/gpio$pin/direction
        	else
			if [ "$pin" == "$PIN_DSRC_SPI_SEL" ] || [ "$pin" == "$PIN_DSRC_PWR_EN" ] || [ "$pin" == "$PIN_UBLOX_GYRO_EN" ] || [ "$pin" == "$PIN_GPIO_DRIVER_EN" ] || [ "$pin" == "$PIN_IMX8_HSM_RST_3V3" ] || [ "$pin" == "$PIN_DSRC_RST_N" ] || [ "$pin" == "$PIN_IMX8_PCIE_PWR_SAV" ] ; then
				echo "IO Exp GPIO $pin is already set in kernel!!"
			else
				# Direction is output
				echo out > /sys/class/gpio/gpio$pin/direction
				echo 0 > /sys/class/gpio/gpio$pin/value
			fi
		fi
	fi
        num=$(($num + 1))
    done
	
    #I2C addr: 0x22
    num=0
    for (( pin=208; pin<224; pin=pin+1 ))
        do
        if [ "$pin" == "$PIN_iMX8_BT_RF_KILL_1V8" ] || [ "$pin" == "$PIN_iMX8_WLAN_RF_KILL_1V8" ] || [ "$pin" == "$PIN_C_V2X_GYRO_EN" ] ; then
            echo "IO Exp GPIO Pin $pin is declared in dts file!!"
        else
            if [ ! -d /sys/class/gpio/gpio$pin ] ; then
                echo $pin > /sys/class/gpio/export
            fi
            #Handle direction
            if [ "$pin" == "$PIN_SXF1800_GPIO_0" ] || [ "$pin" == "$PIN_MDM_PMD_GPIO3" ]  ; then
                # Direction is input
                echo in > /sys/class/gpio/gpio$pin/direction
            else
			if [ "$pin" == "$PIN_USB_HUB_RST_N_1V8" ] || [ "$pin" == "$PIN_IMX_PMD_CBLPWR_N" ] || [ "$pin" == "$PIN_USB_PWR_EN" ]; then
            			echo "IO Exp GPIO $pin is already set in kernel!!"
			else
                		# Direction is output
                		echo out > /sys/class/gpio/gpio$pin/direction
                	#if [ "$pin" == "$PIN_CAM_GENERAL_IO0" ] || [ "$pin" == "$PIN_VCC_FAN_V12P0_EN" ] || [ "$pin" == "$PIN_GNSS_SEL1" ]; then
					#JStinnett 20210309: Disable fan for RSU product
					if [ "$pin" == "$PIN_CAM_GENERAL_IO0" ] || [ "$pin" == "$PIN_GNSS_SEL1" ]; then
                    		# Default output is high
                    		echo 1 > /sys/class/gpio/gpio$pin/value
                	else
                    		# Default output is low
                    		echo 0 > /sys/class/gpio/gpio$pin/value
                	fi
            	fi
        	fi
		fi
        num=$(($num + 1))
    	done
}
if [ "$1" == "init" ] ; then
    init
elif [ "$2" == "r" ] ; then
    cat /sys/class/gpio/gpio$((PIN_$1))/value
elif [ "$2" == "w" ] ; then
    echo $3 > /sys/class/gpio/gpio$((PIN_$1))/value
else
    usage
fi
