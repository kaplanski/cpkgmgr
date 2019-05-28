/*
 a fast non-cryptographic hash using then crc64-iso
 polynome x^64 + x^4 + x^3 + x + 1 bytewise
 Copyright (c) 2019 Jan-Daniel Kaplanski
 MIT/X11 LICENSE
*/

#ifndef _MOD_HASH_H
#define _MOD_HASH_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>

unsigned long long crc64(char *tohash, long length){
 int i=0;
 unsigned long long x64=0, x4=0,x3=0, num=0, buf=0, rval=0;

 for(i=0;i<length;i++)
  {
   num = (unsigned long long) tohash[i];
   x64 = pow(num,64);
   x4 = pow(num,4);
   x3 = pow(num,3);
   buf = x64 + x4 +x3 + num + 1;
   rval = rval + buf;
  }
 //printf("%llx", rval);
 return rval;
}

int chkhsh(char tgz[512], char hfile[512]){
 char * tohash = 0;
 char * prehash = 0;
 long lenhfile = 0, lentohash = 0;
 unsigned long long hash = 0;

 if((access(tgz, F_OK) != -1) && (access(hfile, F_OK) != -1))
  {
   FILE * ftgz = fopen(tgz, "rb");
   FILE * fhfile = fopen(hfile, "rb");

   if (ftgz)
    {
     fseek(ftgz, 0, SEEK_END);
     lentohash = ftell(ftgz);
     fseek(ftgz, 0, SEEK_SET);
     tohash = malloc(lentohash);
     if (tohash)
      {
       fread (tohash, 1, lentohash, ftgz);
      }
     fclose (ftgz);
    }

   if (fhfile)
    {
     fseek(fhfile, 0, SEEK_END);
     lenhfile = ftell(fhfile);
     fseek(fhfile, 0, SEEK_SET);
     prehash = malloc(lenhfile);
     if (prehash)
      {
       fread (prehash, 1, lenhfile, fhfile);
      }
     fclose (fhfile);
     hash = strtoull(prehash, NULL, 16);
    }

   if (tohash && hash)
    {
     if (hash == (crc64(tohash, lentohash)))
      {return 0;}
     else
      {return 1;}
    }
   else
    {
     fprintf(stderr, "failed to read into buffer\n");
     return 2;
    }
  }
 else
  {
   perror("fail");
   return 2;
  }
 return 1;
}
#endif
