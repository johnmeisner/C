#!/bin/bash

# this file is just to create the header file of the i2v types
# needed for spat data; it comes from the i2v_types.h
# file, but the entire file should not be provided

targetfile=sdk_types.h

rm -f $targetfile

echo "#ifndef SDK_TYPES_H" > $targetfile
echo "#define SDK_TYPES_H" >> $targetfile
echo "" >> $targetfile
echo "" >> $targetfile
cat ../common/i2v_types.h | grep SPAT_MAX | grep define >> $targetfile
echo "" >> $targetfile
cat ../common/i2v_types.h | sed -n '/do not add/,/i2vSPATDataType/p' >> $targetfile
sed -i '/do not add/d' $targetfile
# so file can be included by shm generator + for end users
sed -i 's/i2v/rsu/g' $targetfile
sed -i 's/SPAT_MAX/MAX_SPAT/g' $targetfile
#deletion of TM comment
sed -i 's/\(.*\)TM.*?\(.*\)/\1\2/' $targetfile
echo "" >> $targetfile
echo "#endif /* SDK_TYPES_H */" >> $targetfile
echo "" >> $targetfile

# now for a little extra love because QNX and Linux aren't compatible
if [ ! -e wsu_types.h ]; then
  cp ../../../../pal/wsu_types.h .
  cp ../../../../inc/types.h .
  sed -i 's#\.\.\/inc\/##' wsu_types.h
  sed -i '/int64_t/d' types.h
fi

