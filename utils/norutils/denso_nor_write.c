/*
 * denso_nor_write.c
 */

#define PROGRAM_NAME "denso_nor_write"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>

#define OTP_MAX_LEN         65          // 64 byte array plus one control byte
#define OPT_CONTROL_OFF     64          // control byte location
#define LOCK_PARTITION  "/dev/mtd9"
#define MAX_FILE_SIZE   2048


/*
 * MEMERASE
 */
static int memerase(int fd, struct erase_info_user *erase)
{
	return ioctl(fd, MEMERASE, erase);
}


int erase_flash(int fd, u_int32_t offset, u_int32_t bytes)
{
	int err;
	struct erase_info_user erase;
	erase.start = offset;
	erase.length = bytes;
	err = memerase(fd, &erase);
	if (err < 0) {
		return 1;
	}
	//fprintf(stderr, "Erased %d bytes from address 0x%.8x in flash\n", bytes, offset);
	return 0;
}


int main(int argc,char *argv[])
{
	int err = 0, fd, offset;
	struct mtd_info_user mtdInfo;
	struct erase_info_user mtdLockInfo;
	unsigned char buf[128];
        char file_buf[MAX_FILE_SIZE];
	FILE *file;
        int i, token_count = 0;
        const char s[2] = ";";
        
	if (argc < 2){
		printf("Usage:\n denso_nor_write infile\n");
		printf("Example: To write and lock \n denso_otp_write test_otp.csv\n");
		exit(0);
	}

	/* Set buffer to 0xff */
	memset(buf,0xff,128);
        

        /* Open Lockable nor partition */
	if ((fd = open(LOCK_PARTITION, O_SYNC | O_RDWR)) < 0){
      printf("lock partition could not be open\n");
      return errno;
  }
        

  /* Check to see if lockable nor partition is lock 
   * - if already locked, notified user and exit
   */
	if (ioctl(fd, MEMGETINFO, &mtdInfo)){
      printf("could not get mtd info\n");
      return errno;
  }
                
  mtdLockInfo.start = 0;
  mtdLockInfo.length = mtdInfo.size; 

  /* Check if partition is already locked */
  if (ioctl(fd, MEMISLOCKED, &mtdLockInfo)){
      printf("\n***Error! Nor partion is locked!\n   Use unlock utility before writing to nor partition.\n\n");
      return errno;
  }
        

	/*== Open CSV input file ==*/
	if ((file = fopen(argv[1], "r")) == NULL) {
		return 1;
	}

  /*
   * Erase partition - to be investigate why we need erase
   */
  err = erase_flash(fd, 0, 131072);
	
	/* Read file */
	//fscanf(file, "%[^\n]", line);
  for (i = 0; i < MAX_FILE_SIZE; ++i){
      int c = getc(file);
      if (c == EOF){
          //file_buf[i] = 0x00;
          break;
      }
      file_buf[i] = c;
  }        
        
  /* get tokens */
  char *token;
        
  /* first token */
  token = strtok(file_buf, s);
  /* get the rest */
  while (token !=NULL) {
      buf[token_count++] = strtol(token, NULL, 16);
      token = strtok(NULL, s);
  }

  /* print out value and confirm before proceed */
	offset = 0;
  printf("\nNote: %d hex bytes to be written to lockable nor partition\n      The remaining %d bytes will be set to 0xff \n", token_count, 64-token_count);

  while (offset < token_count) {
      printf("0x%04x:", offset);
      for (i = offset; i < offset+16; i++){
          printf(" %02x", buf[i]);
          if (i >= token_count-1) break;    
      }
      printf("\n");
      offset += 16;
  }
        
  /*=== Write to nor ===*/
	if (0 != lseek(fd, 0, SEEK_SET)) {
		return 1;
	}
  write(fd, buf, 64);
  printf("\nWrote %d bytes of user data to lockable nor partition\n", 64);

  /* Lock partition */
  printf("Nor partion is now locked\n");
	ioctl(fd, MEMLOCK, &mtdLockInfo);
        
  fclose(file); 
	close(fd);
  return err;
}

