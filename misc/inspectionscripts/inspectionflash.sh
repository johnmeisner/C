#!/bin/bash

echo inspection build running...
while [[ ! -e /rwflash ]]; do
    sleep 1
done

if [[ ! -z "`file /rwflash/configs | grep link`" ]]; then
    echo Preparing rwflash/configs directory
    rm -f /rwflash/configs
    mkdir /rwflash/configs
    cp -r /usr/config/* /rwflash/configs
fi

