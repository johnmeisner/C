/*
 * nor_unlock.c
 *
 * utilities for locking/unlocking the last sector (128KB)
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <string.h>
#include <errno.h>

#include <mtd/mtd-user.h>

#define LOCK_PARTITION  "/dev/mtd9"

int main(int argc, char *argv[])
{
	int fd;
	struct mtd_info_user mtdInfo;
	struct erase_info_user mtdLockInfo;
	const char *dev;

	dev = LOCK_PARTITION;

	/* Get the device info to compare to command line sizes */
	fd = open(dev, O_RDWR);
	if (fd < 0){
      printf("lock partition could not be open\n");
      return errno;
   }

	if (ioctl(fd, MEMGETINFO, &mtdInfo)){
      printf("could not get mtd info\n");
      return errno;
  }
                
  mtdLockInfo.start = 0;
  mtdLockInfo.length = 1; 
        
  /* Finally do the operation */
	ioctl(fd, MEMUNLOCK, &mtdLockInfo);

	return 0;
}
