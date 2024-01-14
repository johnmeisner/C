# This script creates .o files  for all the .c files
#
# Makefile created from lazy script writer's work
#

#TARGET = -Vgcc_ntoarmv7le

CFLAGS += -fPIC -D_OPTIMIZED -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DCPP -DSMI_ENABLED 
CFLAGS += -I../ppclib/ -L../ppclib/c/lib/

all:
	#qcc $(CFLAGS) -c *.c
	$(CC) $(CFLAGS) -c *.c
# Shared Library
	#qcc -fPIC -shared -Wl  -WWall -W $(CFLAGS) -w -o libv2vuperasn1.so *.o -lasn1oer
	#$(CC) -fPIC -shared -W $(CFLAGS) -w -o libv2vuperasn1.so *.o -lasn1oer
	$(CC) -fPIC -shared -W $(CFLAGS) -w -o libv2vuperasn1.so *.o 

clean:
	rm -f *.o libv2vuperasn1.so 
