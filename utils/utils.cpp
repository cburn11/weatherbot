#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int write_fifo_msg(int fd, const char * szMsg) {

  if( szMsg == NULL)
    return 0;
  
  short cbMsg = strlen(szMsg) + 1;
  char * byMsg = (char *) malloc(cbMsg + 2);

  int cbWritten = 0;
  
  if( byMsg ) {

    memcpy(byMsg, &cbMsg, 2);
    memcpy(byMsg + 2, szMsg, cbMsg);

    cbWritten = write(fd, byMsg, cbMsg + 2);
    //printf("utils write: %s\n", szMsg);
    free(byMsg);
  }

  return cbWritten;
}

int read_fifo_msg(int fd, char ** pszMsg) {

  if( pszMsg == NULL )
    return 0;
  
  *pszMsg = NULL;
  
  int cbRead = 0;
  short cbMsg = 0;
  
  if( read(fd, &cbMsg, sizeof(cbMsg) ) == sizeof(cbMsg) ) {

    char * szMsg = (char *) malloc(cbMsg);
    if( szMsg ) {

      cbRead = read(fd, szMsg, cbMsg);
      //printf("utils read: %s\n", szMsg);
      *pszMsg = szMsg;
    }
  }

  return cbRead;
}
