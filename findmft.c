// findmft.c
// Find MFT or its mirror in corrupted NTFS disk/partition
//
// Author : Weipeng He <heweipeng@gmail.com>
// Copyright (c) 2013, All rights reserved.

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <wchar.h>

#define SECTOR_SIZE 512
#define RECORD_SIZE 1024

const char* IDENTIFIER = "FILE"; 
#define ID_LEN 4

// Determine if a 1024 byte data block is MFT first record.
int ismft(char* record) {
  if (strncmp(IDENTIFIER, record, ID_LEN) != 0) {
    return 0;
  }
  return 1;
}

// Find the next MFT.
// pos : starting sector; set to position of the MFT;
// jump : jump number of sectors; should be # of sectors per cluster.
// return : 1 if found; 0 if not found; -1 if error occured 
int findmft(const char* device, unsigned long long* pos, unsigned int jump) {
  int fd = open(device, O_RDONLY);
  if (fd < 0) {
    return -1;
  }

  char* buffer = (char*) malloc(RECORD_SIZE);
  off_t ptr = *pos * SECTOR_SIZE;
  ssize_t rn;
  int found = 0;
  long unsigned int counter = 0;

  while (!found) {
    if (counter % 1000 == 0) {
      printf("\r%lu", counter);
    }
    counter++;

    rn = pread(fd, buffer, SECTOR_SIZE, ptr);
    if (rn < 0) {
      return -1;
    }
    else if (rn == 0) {
      return 0;
    }

    if (ismft(buffer)) {
      *pos = *pos + (ptr - *pos * SECTOR_SIZE) / SECTOR_SIZE;
      found = 1;
    }
    else {
      ptr += jump * SECTOR_SIZE;
    }
  }
  return 1;
}

int main(int argc, char** argv) {
  unsigned long long loc = 0;
  int r;
  while ((r = findmft(argv[1], &loc, 8)) > 0) {
    printf("\nFound at %llu\n", loc);
    loc += 8;
  }
  if (r < 0) {
    perror("Error");
    return EXIT_FAILURE;
  }
  return 0;
}

