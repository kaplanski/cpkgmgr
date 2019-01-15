/*
 cpkgmgr - a port of pkgmgr to c
 Copyright (c) 2019 Kaplanski
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#define PKGDIR "/cpkgmgr"
#define INDIR "/bin"
#define ARCHDIR "/ARCH"
#define REPO "https://gitup.uni-potsdam.de/kaplanski/pkgmgr/raw/master/repo"

/* #define DEBUG */
/* #define PY2DBG */

/* initial creation of various folders and the instlld file */
void first_run(char pkgdir[512], char indir[512], char archdir[512], \
               char arch[16], char instlld[512], char cfgfile[512], int sup_arch){
 int instlldfd = -1, cfgfd_creat = -1;

 if(access(pkgdir, F_OK) == -1)
  {mkdir(pkgdir, S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
   printf("Initial pkgfldr created!\n");}
 if(access(indir, F_OK) == -1)
  {mkdir(indir, S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
   printf("Initial infldr created!\n");}
 if(access(archdir, F_OK) == -1)
  {mkdir(archdir, S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
   printf("Initial ARCHfldr created!\n");}
 if((access(instlld, F_OK) == -1) && (sup_arch == 1))
  {
   instlldfd = open(instlld, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
   if (instlldfd != -1)
    {
     write(instlldfd,"[PKGID:Name:Version]\n", strlen("[PKGID:Name:Version]\n"));
     close(instlldfd);
     printf("Initial installed_%s.db created!\n", arch);
    }
   else
    {printf("Failed to create installed_%s.db\n", arch);}
  }

if((access(cfgfile, F_OK) == -1))
  {
   cfgfd_creat = open(cfgfile, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
   if (cfgfd_creat != -1)
    {
     write(cfgfd_creat,"repo=", strlen("repo="));
     write(cfgfd_creat,REPO, strlen(REPO));
     write(cfgfd_creat,"\narch=python2\n", strlen("\narch=python2\n"));
     write(cfgfd_creat,"#arch=python3\n", strlen("#arch=python3\n"));
     write(cfgfd_creat,"#arch=i386\n", strlen("#arch=i386\n"));
     write(cfgfd_creat,"#arch=amd64\n", strlen("#arch=amd64\n"));
     close(cfgfd_creat);
     printf("Initial pkgmgr.cfg created!\n");
    }
   else
    {printf("Failed to create pkgmgr.cfg\n");}
  }
}

/* execute an installed app */
void run_app(char indir[512], char app[]){
 int i = 0, app_present = 0;
 char app_path[512] = "", tmp[512] = "";
 char *end[4];
 end[0] = "";
 end[1] = ".sh";
 end[2] = ".bin";
 end[3] = ".py";
 strcpy(app_path, indir);
 strcat(app_path, "/");
 strcat(app_path, app);
 strcat(app_path, "/");

 if(access(app_path, F_OK) == -1)
  {printf("App %s is not installed!\n", app);}
 else
  {
   for (i = 0; i < (sizeof(end)/sizeof(*end)); i++)
    {
     strcpy(tmp, app_path);
     strcat(tmp, app);
     strcat(tmp, end[i]);
     if(access(tmp, F_OK) != -1)
      {app_present = 1; break;}
    }
   if (app_present == 1)
    {execlp(tmp, app, NULL);}
   else
    {printf("App not present in app folder!\n"); exit(133);}
  }
}

/* clean the ARCH folder */
void clean(char archdir[512]){

 char tmp[512];
 printf("Cleaning ARCH folder... ");

 DIR *d;
 struct dirent *dir;
 d = opendir(archdir);
 if (d)
  {
   while ((dir = readdir(d)) != NULL)
    {
     if ((strcmp(dir->d_name, ".")) != 0 && ((strcmp(dir->d_name, "..")) != 0))
      {
       strcpy(tmp, archdir);
       strcat(tmp, "/");
       strcat(tmp, dir->d_name);
       remove(tmp);
      }
    }
   closedir(d);
  }

 printf("[Done]\n");

}

/* just an info display */
void info(void){
 printf("+-----------------------------------------------+\n" \
        "|             cpkgmgr - pkgmgr in c             |\n" \
        "|    Copyright (c) 2019 Jan-Daniel Kaplanski    |\n" \
        "+-----------------------------------------------+\n\n");
}

/* argument help */
void help(char *prg, char pkgdir[512], char indir[512], char arch[16]){

 printf("Usage: %s [-c|-da|-di|-h|-i|-r|-s|-u] [pkg]\n" \
        "   -c: cleans the package folder of downloaded packages\n" \
        "   -h: displays this help\n" \
        "   -u: updates the package index\n" \
        "   -i [pkg]: installs a package (-ri: reinstall)\n" \
        "   -r [pkg]: removes a package\n" \
        "   -s [pkg]: searches for a package in the package index\n" \
        "  -da: list all available packages for %s\n" \
        "  -di: list all installed packages for %s\n" \
        "  -ca [arch]: change your architecture\n" \
        "Current binary folder: %s\n" \
        "Current pkgmgr folder: %s\n" \
        "Current architecture: %s\n", prg, arch, arch, indir, pkgdir, arch);
}

int main(int argc, char *argv[]){
 /* Var Init */
 int i = 0, j = 0, sup_arch = 0, cfgfd = -1, arch_set = 0, repo_set = 0;
 char pkgfldr[512] = "", infldr[512] = "", archfldr[512] = "", \
      instlld[512] = "", arch[16] = "python2", repo[1024] = REPO, \
      cfgfile[512] = "", cfgbuf[32] = "", cfgln[512] = "", cfgtmp[512];
 const char *home = getenv("HOME");
 const char *archlst[4];
 archlst[0] = "python2";
 archlst[1] = "python3";
 archlst[2] = "i386";
 archlst[3] = "amd64";

 #ifdef PY2DBG
 strcpy(arch, "python2");
 #endif

 /* pfad var init */
 strcpy(pkgfldr, home);
 strcat(pkgfldr, PKGDIR);
 strcpy(infldr, pkgfldr);
 strcat(infldr, INDIR);
 strcpy(archfldr, pkgfldr);
 strcat(archfldr, ARCHDIR);
 strcpy(cfgfile, pkgfldr);
 strcat(cfgfile, "/pkgmgr.cfg");

 /* import config if present */
 if(access(cfgfile, F_OK) != -1)
  {
   #ifdef DEBUG
   printf("opening config file...\n");
   #endif
   cfgfd = open(cfgfile, O_RDONLY);
   if (cfgfd != -1)
    {
     while ((read(cfgfd, cfgbuf, 1)) != 0)
      {
       if ((strcmp(cfgbuf, "\n")) != 0)
        {
         cfgln[j] = cfgbuf[0];
         j++;
        }
       else
        {
         strncpy(cfgtmp, cfgln, j);
         cfgtmp[j] = '\0';
         #ifdef DEBUG
         printf("cfgtmp: %s\n", cfgtmp);
         #endif
         if ((strstr(cfgtmp, "#")) != NULL)
          {}
         else if (((strstr(cfgtmp, "repo=")) != NULL) && (repo_set == 0))
          {
           strcpy(repo, cfgtmp+5);
           repo_set = 1;
           #ifdef DEBUG
           printf("repo set!\n");
           #endif
          }
         else if (((strstr(cfgtmp, "arch=")) != NULL) && (arch_set == 0))
          {
           strcpy(arch, cfgtmp+5);
           arch_set = 1;
           #ifdef DEBUG
           printf("arch set!\n");
           #endif
          }

         /* reset counter */
         j = 0;
        }
      }

     /* close if successfully opened */
     close(cfgfd);
    }
  }

 for (i = 0; i < (sizeof(archlst)/sizeof(*archlst)); i++)
  {
   if ((strcmp(archlst[i], arch)) == 0)
    {sup_arch = 1; break;}
  }

 /* finally we can use arch in our vars */
 strcpy(instlld, pkgfldr);
 strcat(instlld, "/installed_");
 strcat(instlld, arch);
 strcat(instlld, ".db");

 /* most important thing first: the info */
 info();

 /* create initial folders/files */
 first_run(pkgfldr, infldr, archfldr, arch, instlld, cfgfile, sup_arch);

 if (argc == 1)
  {
   printf("At least one argument required!\n" \
          "Try: %s -h (or specifiy a file)!\n", argv[0]);
  }
 else if (argc > 1)
  {
   /* CODE */
   #ifdef DEBUG
   printf("pkgfldr = %s\n", pkgfldr);
   printf("infldr = %s\n", infldr);
   printf("archfldr = %s\n", archfldr);
   printf("instlld = %s\n", instlld);
   printf("arch = %s\n", arch);
   #endif

   /* display help */
   if ((strcmp(argv[1], "-h")) == 0)
    {help(argv[0], pkgfldr, infldr, arch);}

   /* clean ARCH folder */
   else if ((strcmp(argv[1], "-c")) == 0)
    {clean(archfldr);}

   /* display all available packages */
   else if ((strcmp(argv[1], "-da")) == 0)
    {}

   /* display all installed packages */
   else if ((strcmp(argv[1], "-di")) == 0)
    {}

   /* install a new package */
   else if ((strcmp(argv[1], "-i")) == 0)
    {}

   /* remove a package */
   else if ((strcmp(argv[1], "-r")) == 0)
    {}

   /* search for a package */
   else if ((strcmp(argv[1], "-s")) == 0)
    {}

   /* update the package index */
   else if ((strcmp(argv[1], "-u")) == 0)
    {}

   /* reinstall a (currently installed) package */
   else if ((strcmp(argv[1], "-ri")) == 0)
    {}

   /* execute an installed package */
   else if ((strcmp(argv[1], "run")) == 0)
    {
     if (argc > 2)
      {run_app(infldr, argv[2]);}
     else
      {printf("Usage: %s %s [pkg]\n", argv[0], argv[1]);}
    }
   else
    {printf("Unknown option!\n");}
  }

return 0;}
