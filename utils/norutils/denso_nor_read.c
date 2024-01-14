/*
 * denso_nor_read.c -- display One-Time-Programm data
 */

#define PROGRAM_NAME "denso_nor_read"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <mtd/mtd-user.h>

#define OTP_MAX_LEN         64          // 64 byte array plus one control byte

int main(int argc,char *argv[])
{
	int fd, i, offset;
	unsigned char buf[125];

        memset(buf,0xa5,sizeof(buf));
               
	fd = open("/dev/mtd9", O_SYNC | O_RDONLY);
	if (fd < 0) {
		return errno;
	}

        /*=== Read ===*/
	if (0 != lseek(fd, 0, SEEK_SET)) {
		return 1;
	}
        read(fd, buf, OTP_MAX_LEN);


        /* Print result */
	offset = 0;
  while (offset < 64) {
      printf("0x%04x:", offset);
      for (i = offset; i < offset+16; i++)
          printf(" %02x", buf[i]);
          printf("\n");
          offset += 16;
  }
        
	close(fd);
	return 0;
}
