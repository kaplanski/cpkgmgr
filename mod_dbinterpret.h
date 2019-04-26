#ifndef _MOD_DBINTERPRET_H
#define _MOD_DBINTERPRET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#define UUA __attribute__((__unused__))

void prep_workln(char workln[512], char (*pkgid)[32], \
                 char (*pkgname)[256], char (*pkgver)[32]){

 /* Var Init */
 int i = 0;
 char *workln_ptr;

 workln_ptr = strtok(workln, ":");

 for (i=0; i<3; i++)
  {
   if (i == 0)
    {strcpy(*pkgid, workln_ptr);}
   else if (i == 1)
    {strcpy(*pkgname, workln_ptr);}
   else if (i == 2)
    {strcpy(*pkgver, workln_ptr);}
   workln_ptr = strtok(NULL, ":");
  }
}

extern int read_db(char fname[512], int display_all, int search, char searchval[256], char (*rval)[256]){
 /* Var Init */
 int indexfd = -1, i = 0, fline_len = 0, scount = 0;
 char buffer[32], line[512], fline[512], workln[512];
 char pkgid[32] = "", pkgname[256] = "", pkgver[32] = "";

 /* open file */
 indexfd = open(fname, O_RDONLY);

 if (indexfd != -1)
  {
   /*
   if ((display_all == 1) || (search == 1))
    {
     printf("Name - Version:\n");
    }
   */
   while ((read(indexfd, buffer, 1)) != 0)
    {
     if ((strncmp(buffer, "\n", 1)) != 0)
      {
       line[i] = buffer[0];
       i++;
      }
     else
      {
       strncpy(fline, line, i);
       fline[i] = '\0';
       fline_len = strlen(fline);
       #ifdef DEBUG
       printf("fline: %s\n", fline);
       printf("fline[0]: %c\n", fline[0]);
       printf("fline+fline_len-1: %s\n", fline+fline_len-1);
       #endif
       if ((strncmp(fline, "#", 1)) == 0)
        {
         /* do nothing */
         sleep(0);
        }
       else if ((strncmp(fline,"{", 1) == 0) && (strncmp(fline+fline_len-1,"}", 1) == 0))
        {
         #ifdef DEBUG
         printf("ArgLine!\n");
         #endif

         /* remove { and } */
         strncpy(workln, fline+1, fline_len-1);
         workln[fline_len-2] = '\0';

         /* split by : */
         prep_workln(workln, &pkgid, &pkgname, &pkgver);

         /* printout the whole list */
         if ((display_all == 1) && (search == 0))
          {printf("%s - %s\n", pkgname, pkgver);}

         /* search + printout */
         else if ((search == 1) && (display_all == 0))
          {
           if ((strcasestr(pkgname, searchval)) != NULL)
            {printf("%s - %s\n", pkgname, pkgver);}
          }

         /* same as above, does not output, case sensitive, used in -i */
         else if ((search == 1) && (display_all == 1))
           {
            if ((strcmp(pkgname, searchval)) == 0)
             {
              printf("pkgname: %s\nsearchval: %s\n", pkgname, searchval);
              strcpy(*rval, pkgname);
              strcat(*rval, "_v");
              strcat(*rval, pkgver);
              strcat(*rval, ".tgz");
              scount++;
             }
           }
        }

       /* reset counter */
       i = 0;
      }
    }
   /* close if successfully opened */
   close(indexfd);
  }

if ((search == 0) && (display_all == 0))
  {return atoi(pkgid)+1;}
 else
  {return scount;}
}

extern int rem_db(char fname[512], char searchval[256]){

 /* Var Init */
 int instlldfd = -1, i = 0, fline_len = 0;
 char buffer[32], line[512], fline[512], workln[512];
 char UUA pkgid[32] = "", pkgname[256] = "", UUA pkgver[32] = "";

 instlldfd = open(fname, O_RDWR);
 if (instlldfd != -1)
  {
   while ((read(instlldfd, buffer, 1)) != 0)
    {
     if ((strncmp(buffer, "\n", 1)) != 0)
      {
       line[i] = buffer[0];
       i++;
      }
     else
      {
       strncpy(fline, line, i);
       fline[i] = '\0';
       fline_len = strlen(fline);
       if ((strncmp(fline,"{", 1) == 0) && (strncmp(fline+fline_len-1,"}", 1) == 0))
        {
         #ifdef DEBUG
         printf("ArgLine!\n");
         #endif

         /* remove { and } */
         strncpy(workln, fline+1, fline_len-1);
         workln[fline_len-2] = '\0';

         /* split by : */
         prep_workln(workln, &pkgid, &pkgname, &pkgver);

         if ((strstr(pkgname, searchval)) != NULL)
          {
           lseek(instlldfd, -(strlen(pkgname) + strlen(pkgid) + strlen(pkgver) + 5), SEEK_CUR);
           write(instlldfd, "#", 1);
          }
        }
       /* reset counter */
       i = 0;
      }
    }
   close(instlldfd);
  }

 return 0;
}
#endif
