#ifndef _MOD_INSTALLER_H
#define _MOD_INSTALLER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void read_display(char archdir[512], char pkg[256], char app[64]){

 int displayfd = -1, i = 0;
 char fname[512], buffer[32], line[512], fline[512];

 /* set file path */
 strcpy(fname, archdir);
 strcat(fname, "/");
 strcat(fname, pkg);
 strcat(fname, "/");
 strcat(fname, app);
 strcat(fname, "_display.txt");

 if(access(fname, F_OK) != -1)
  {
   displayfd = open(fname, O_RDONLY);
   if (displayfd != -1)
    {
     printf("\n");
     while ((read(displayfd, buffer, 1)) != 0)
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
         printf("%s\n", fline);
         i = 0;
        }
      }
    }
  }
}

extern void install(char pkg[256], char pkgdir[512], char indir[512], char app[64], int ri){

 int i = 0;

 char inscript[512] = "", pname[128] = "", tmp[512] = "", \
      destdir[512] = "", pkgsrcdir[512] = "", archdir[512] = "", \
      tgz[512] = "", syscall[1024] = "";

 char *end[4];
 end[0] = "";
 end[1] = ".sh";
 end[2] = ".bin";
 end[3] = ".py";

 /* set archdir */
 strcpy(archdir, pkgdir);
 strcat(archdir, "/ARCH");

 /* set pkgsrc path */
 strcpy(pkgsrcdir, archdir);
 strcat(pkgsrcdir, "/");
 strcat(pkgsrcdir, pkg);

 /* installer script path */
 strcpy(inscript, pkgsrcdir);
 strcat(inscript, "/");
 strcat(inscript, app);
 strcat(inscript, "_install.sh");

 /* set processname */
 strcpy(pname, app);
 strcat(pname, " - installer");

 /* set destdir */
 strcpy(destdir, indir);
 strcat(destdir, "/");
 strcat(destdir, app);

 /* set syscall pkgname */
 strcpy(syscall, "tar -xzf ");
 strcpy(tgz, archdir);
 strcat(tgz, "/");
 strcat(tgz, pkg);
 strcat(tgz, ".tgz");
 strcat(syscall, tgz);

 if(access(destdir, F_OK) == -1)
  {
   if (strcmp(app, "pkgmgr") != 0)
    {mkdir(destdir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);}
  }
 else
  {
   if (ri == 0)
    {
     printf("%s is already installed! (did you mean: -ri)\n", app);
     exit(42);
    }
   else if (ri == 1)
     {
      printf(">Removing previous version... ");
      fflush(stdout);
      strcpy(syscall, "rm -rf ");
      strcat(syscall, destdir);
      strcat(syscall, "/*"); /**/
      system(syscall);
      printf("[DONE]\n");
     }
   else if (ri == 2)
    {
     printf(">Updating now...\n");
    }
  }

 /* unpacking the archive */
 printf("Unpacking... ");
 fflush(stdout);

 chdir(archdir);
 if(access(tgz, F_OK) != -1)
  {
   system(syscall);
   chdir(pkgsrcdir);
   printf("[DONE]\n");
  }
 else
  {
   printf("[FAIL]\nArchive not present in %s!\n", archdir);
   printf("tgz = %s\n", tgz);
   exit(177);
  }

 /* generic install */
 printf("Installing %s...\n", app);

 /* depreciated, c port of pkgmgr does execute files with endings */
 //if [ -f "$2" -o -f "$2.bin" -o -f "$2.sh" -o -f "$2.py" ]; then
 //if [ ! -f "nofile" ]; then

 /* new way to handle generic install */
 if(access(pkgsrcdir, F_OK) != -1)
  {
   for (i=0; i<3; i++)
    {
     strcpy(tmp, pkgsrcdir);
     strcat(tmp, "/");
     strcat(tmp, app);
     strcat(tmp, end[i]);
     if(access(tmp, F_OK) != -1)
      {
       strcpy(syscall, "cp ");
       strcat(syscall, tmp);
       strcat(syscall, " ");
       strcat(syscall, destdir);
       system(syscall);
       break;
      }
    }
  }

 /* calling external installer script */
 if(access(inscript, F_OK) != -1)
 {
  strcpy(syscall, inscript);
  strcat(syscall, " ");
  strcat(syscall, pkgdir);
  strcat(syscall, " ");
  strcat(syscall, indir);
  strcat(syscall, " ");
  strcat(syscall, app);
  system(syscall);
 }

 printf("[DONE]\n");
 read_display(archdir, pkg, app);

 /* cleanup */
 strcpy(syscall, "rm -rf ");
 strcat(syscall, pkgsrcdir);
 system(syscall);
}

void download(char repo[1024], char arch[16], char dwfile[512], char saveas[512]){

 if (system("ping 8.8.8.8 -c 1 1>/dev/null 2>/dev/null") != 0)
  {printf("[FAIL]\nCheck network connection!\n");}
 else
  {
   char syscall[4096] = "";

   #ifndef OLD_WGET
   strcpy(syscall, "wget -q -t 1 --show-progress ");
   #endif
   #ifdef OLD_WGET
   strcpy(syscall, "wget -t 1 ");
   #endif
   strcat(syscall, repo);
   strcat(syscall, "/");
   strcat(syscall, arch);
   strcat(syscall, "/");
   strcat(syscall, dwfile);
   if (saveas != NULL)
    {
     strcat(syscall, " -O ");
     strcat(syscall, saveas);
    }
   else
    {
     strcat(syscall, " -O ");
     strcat(syscall, dwfile);
    }
   #ifdef DEBUG
   printf("syscall (download) = %s\n", syscall);
   #endif
   system(syscall);
   printf("[DONE]\n");
  }
}

#endif
