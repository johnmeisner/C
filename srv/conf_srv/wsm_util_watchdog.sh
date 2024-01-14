#
# Watchdog for webgui's wsm utility
#   Watches for when user leaves wsm_util page and reboots RSU

WATCHDOG_PAT_FILE=/tmp/ztmp.util_wsm_want_data
WATCHDOG_LOG=/tmp/zdbg.watchdog_log.txt

echo `date +%H:%M:%S` - Watchdog started >> $WATCHDOG_LOG

# Safety Check - Verify i2v_app is actually gone
I2VPID=`ps x | grep i2v_app | grep -v grep`
if [ x"$I2VPID" != "x" ]; then
    echo `date +%H:%M:%S` - WARNING: The i2v_app is still running - pid $I2VPID '!!!' >> $WATCHDOG_LOG
    ps uwwx | grep _app >> $WATCHDOG_LOG
fi

# Wait for a pat to start
while [ ! -f $WATCHDOG_PAT_FILE ]; do
    sleep 1
    echo `date +%H:%M:%S` - waiting fer the first pat >> $WATCHDOG_LOG
done

# Keep watch until WATCHDOG is no longer getting pats; ie pat-file has disappeared for 15 seconds
NOPATCOUNT=0
while [ $NOPATCOUNT -lt 15 ]; do
    # Delay a little
    sleep 1
    # Detect if webgui has given us a pat
    if [ -f $WATCHDOG_PAT_FILE ]; then
        rm $WATCHDOG_PAT_FILE
        if [ $NOPATCOUNT -gt 0 ]; then
            echo `date +%H:%M:%S` - Watchdog pat-file exists again, resetting NOPATCOUNT to zero >> $WATCHDOG_LOG
            NOPATCOUNT=0
        fi
        # The webgui refreshes at slightly slower than 1hz, so we give it extra
        # time, since we just saw the pat file
        sleep 1
    # No Pat!
    else
        let NOPATCOUNT++
        echo `date +%H:%M:%S` - Watchdog pat-file missing, now NOPATCOUNT = $NOPATCOUNT >> $WATCHDOG_LOG
    fi
done

echo `date +%H:%M:%S` - Watchdog loop exited at NOPATCOUNT = $NOPATCOUNT >> $WATCHDOG_LOG

# Time to reboot
echo `date +%H:%M:%S` - Watchdog rebooting RSU >> $WATCHDOG_LOG
/sbin/reboot

# All done
echo `date +%H:%M:%S` - Watchdog exiting >> $WATCHDOG_LOG
