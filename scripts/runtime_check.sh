#!/bin/sh
###########################################################################
## runtime_check.sh
# add other critical directories and files for permissions as needed
###########################################################################
# [perms=0755] /scripts/runtime_check.sh = {
#!/bin/sh

# KENN: What is this supposed to do?
exec 3>/dev/console

if [ ! -e /rwflash/1-2-3-4-5 ]; then
    echo "$0 INFO: performing runtime check of text files"
    #file format first
    for d in /rwflash/configs /rwflash/configs/v2x /rwflash/configs/Aerolink
    do
        for i in `ls $d`; do
           if [[ ! -z "`/usr/bin/file $d/$i | grep ASCII | grep CRLF`" ]]; then
                /usr/bin/dos2unix $d/$i
           fi
           if [[ ! -z "`/usr/bin/file $d/$i | grep Bourne | grep CRLF`" ]]; then
                /usr/bin/dos2unix $d/$i
           fi
        done
    done
    # permissions on key files
    /bin/chmod 755 /rwflash/configs/startup.sh
    # TODO: # Mark /rwflash/configs as runtime-checked ?
    # touch /rwflash/1-2-3-4-5
else
    echo "$0 INFO: runtime check skipped by manual force"
fi
