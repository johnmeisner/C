
BEFORE=`dcm -g I2VGlobalDebugFlag`
dcm -s I2VGlobalDebugFlag -f i2v.conf -v 1
AFTER1=`dcm -g I2VGlobalDebugFlag -f i2v.conf`
dcm -s I2VGlobalDebugFlag -v 2
AFTER2=`dcm -g I2VGlobalDebugFlag`
dcm -s I2VGlobalDebugFlag -v $BEFORE

if [ $AFTER1 == $AFTER2 ]; then
   echo CHECK FAILED
   echo "   ==> set1 = $AFTER1 (want 1) and set2 = $AFTER1 (want 2)"
else
   echo PASS
fi
