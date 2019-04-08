/*
 pkgmgr - a port of pkgmgr to c
 Copyright (c) 2019 Jan-Daniel Kaplanski
 MIT/X11 LICENSE
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "mod_dbinterpret.h"
#include "mod_installer.h"
#include "mod_setup.h"
#define PKGDIR "/pkgmgr"
#define INDIR "/bin"
#define ARCHDIR "/ARCH"
#define REPO "https://gitup.uni-potsdam.de/kaplanski/pkgmgr/raw/master/repo"

/* Turn on debug/verbose output */
//#define DEBUG

/* Revert back from installed.db to old installed_$arch.db */
//#define OLD

/* initial creation of various folders and the instlld file */
void first_run(char pkgdir[512], char indir[512], char archdir[512], \
               char arch[16], char instlld[512], char cfgfile[512], \
               int sup_arch __attribute__((__unused__)), char indexf[512], char repo[1024]){
 int instlldfd = -1, cfgfd_creat = -1;

 /* create folders + installed db */
 if(access(pkgdir, F_OK) == -1)
  {mkdir(pkgdir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
   printf("Initial pkgfldr created!\n");}
 if(access(indir, F_OK) == -1)
  {mkdir(indir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
   printf("Initial infldr created!\n");}
 if(access(archdir, F_OK) == -1)
  {mkdir(archdir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
   printf("Initial ARCHfldr created!\n");}
 #ifdef OLD
 if((access(instlld, F_OK) == -1) && (sup_arch == 1))
 #endif
 #ifndef OLD
 if (access(instlld, F_OK) == -1)
 #endif
  {
   instlldfd = open(instlld, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
   if (instlldfd != -1)
    {
     write(instlldfd,"[PKGID:Name:Version]\n", strlen("[PKGID:Name:Version]\n"));
     close(instlldfd);
     #ifdef OLD
     printf("Initial installed_%s.db created!\n", arch);
     #endif
     #ifndef OLD
     printf("Initial installed.db created!\n");
     #endif
    }
   else
    {
     #ifdef OLD
     printf("Failed to create installed_%s.db\n", arch);
     #endif
     #ifndef OLD
     printf("Failed to create installed.db\n");
     #endif
    }
  }

 /* create config file */
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

 /* download Package index */
 if((access(indexf, F_OK) == -1))
  {download(repo, arch, "index.db", indexf);}
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
   for (i = 0; i < (int)(sizeof(end)/sizeof(*end)); i++)
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

 char syscall[1024];
 printf("Cleaning %s... ", archdir);
 strcpy(syscall, "rm -rf ");
 strcat(syscall, archdir);
 strcat(syscall, "/*"); /**/
 system(syscall);
 printf("[Done]\n");

}

/* just an info display */
void info(void){
 printf("+-----------------------------------------------+\n" \
        "|             pkgmgr - pkgmgr in c              |\n" \
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
        "setup: change your architecture or repo\n" \
        "  run: execute an installed app\n" \
        "Current binary folder: %s\n" \
        "Current pkgmgr folder: %s\n" \
        "Current architecture: %s\n", prg, arch, arch, indir, pkgdir, arch);
}

int main(int argc, char *argv[]){
 /* Var Init */
 int i = 0, j = 0, sup_arch = 0, cfgfd = -1, arch_set = 0, \
     repo_set = 0, pkgverlen = 0, instlldfd = -1;
 char pkgfldr[512] = "", infldr[512] = "", archfldr[512] = "", \
      instlld[512] = "", arch[16] = "python2", repo[1024] = REPO, \
      cfgfile[512] = "", cfgbuf[32] = "", cfgln[512] = "", cfgtmp[512], \
      indexfile[512] = "", intmp[256] = "", intmp2[256] = "", syscall[1024], \
      lctmp[12] = "", lctmp2[256] = "", pkgver[32] = "";
 const char *home = getenv("HOME");
 const char *archlst[4];
 archlst[0] = "python2";
 archlst[1] = "python3";
 archlst[2] = "i386";
 archlst[3] = "amd64";

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
         if ((strncmp(cfgtmp, "#", 1)) == 0)
          {
           /* do nothing */
           sleep(0);
          }
         else if (((strncmp(cfgtmp, "repo=", 5)) == 0) && (repo_set == 0))
          {
           strcpy(repo, cfgtmp+5);
           repo_set = 1;
           #ifdef DEBUG
           printf("repo set!\n");
           #endif
          }
         else if (((strncmp(cfgtmp, "arch=", 5)) == 0) && (arch_set == 0))
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

 for (i = 0; i < (int)(sizeof(archlst)/sizeof(*archlst)); i++)
  {
   if ((strcmp(archlst[i], arch)) == 0)
    {sup_arch = 1; break;}
  }

 /* finally we can use arch in our vars */
 strcpy(instlld, pkgfldr);
 #ifdef OLD
 strcat(instlld, "/installed_");
 strcat(instlld, arch);
 strcat(instlld, ".db");
 #endif
 #ifndef OLD
 strcat(instlld, "/installed.db");
 #endif

 strcpy(indexfile, pkgfldr);
 strcat(indexfile, "/index_");
 strcat(indexfile, arch);
 strcat(indexfile, ".db");

 /* most important thing first: the info */
 if (((argc > 1) && ((strcmp(argv[1], "run")) == 0)) || ((argc > 3) && ((strcmp(argv[3], "-ri")) == 0)))
  {sleep(0);}
 else
  {info();}

 /* create initial folders/files */
 first_run(pkgfldr, infldr, archfldr, arch, instlld, \
           cfgfile, sup_arch, indexfile, repo);

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
    {
     printf("Available Packages:\n");
     read_db(indexfile, 1, 0, NULL, NULL);
    }

   /* display all installed packages */
   else if ((strcmp(argv[1], "-di")) == 0)
    {
     printf("Installed Packages:\n");
     read_db(instlld, 1, 0, NULL, NULL);
    }

   /* install a new package */
   else if ((strcmp(argv[1], "-i")) == 0)
    {
     if (argc < 3)
      {
       printf("Usage: %s %s [pkg]\n", argv[0], argv[1]);
       exit(222);
      }
     else
      {
       if ((argc != 4) && (read_db(instlld, 1, 1, argv[2], &intmp) > 0))
        {
         printf("%s is already installed! (Did you mean: -ri)\n", argv[2]);
         exit(201);
        }
       else if ((argc == 4) && (strcmp("-ri", argv[3]) != 0))
        {execlp(argv[0], argv[0], "-i", argv[2], "-ri", NULL);}
       else if ((argc == 3) || (strcmp("-ri", argv[3]) == 0))
        {
         if (read_db(indexfile, 1, 1, argv[2], &intmp) == 1)
          {
           if(access(intmp, F_OK) != -1)
            {printf("Using cached package...\n");}
           else
            {
             printf("Downloading %s...\n", argv[2]);
             chdir(archfldr);
             download(repo, arch, intmp, NULL);
            }

           strncpy(intmp2, intmp, strlen(intmp)-4);
           intmp2[strlen(intmp)-3] = '\0';
           install(intmp2, pkgfldr, infldr, argv[2]);

           /* add to instlld */
           /* -6 = _v + .tgz */
           pkgverlen = (strlen(intmp) - strlen(argv[2]) - 6);
           strncpy(pkgver, intmp+strlen(argv[2])+2, pkgverlen);
           pkgver[pkgverlen+1] = '\0';

           /* generate the entry */
           /* get new ID via line count */
           sprintf(lctmp, "%d", read_db(instlld, 0, 0, NULL, NULL));
           if (strlen(lctmp) > 1)
            {strcpy(lctmp2, "{0");}
           else
            {strcpy(lctmp2, "{00");}
           strcat(lctmp2, lctmp);
           strcat(lctmp2, ":");
           strcat(lctmp2, argv[2]);
           strcat(lctmp2, ":");
           strcat(lctmp2, pkgver);
           strcat(lctmp2, "}\n");

           #ifdef DEBUG
           printf("lctmp2 = %s", lctmp2);
           #endif

           if (argc != 4)
            {instlldfd = open(instlld, O_WRONLY | O_APPEND);}
           if (instlldfd != -1)
            {
             write(instlldfd, lctmp2, strlen(lctmp2));
             close(instlldfd);
            }
           else
            {printf("Could not open %s!\n", instlld);}
          }
         else
          {
           printf("%s was not found on the index...\n" \
                  "Similar sounding packages:\n", argv[2]);
           read_db(indexfile, 0, 1, argv[2], NULL);
          }
        }
      }
    }
   /* remove a package */
   else if ((strcmp(argv[1], "-r")) == 0)
    {
     if (argc < 3)
      {
       printf("Usage: %s %s [pkg]\n", argv[0], argv[1]);
       exit(222);
      }
     else if (read_db(instlld, 1, 1, argv[2], &intmp) == 1)
      {
       chdir(infldr);
       if(access(argv[2], F_OK) != -1)
        {

         /* physical removal */
         strcpy(syscall, "rm -rf ");
         strcat(syscall, infldr);
         strcat(syscall, "/");
         strcat(syscall, argv[2]);
         system(syscall);

         /* db removal */
         rem_db(instlld, argv[2]);

         printf("Done!\n");
        }
       else
        {
         printf("%s was not found in %s!\n" \
                "Similar sounding packages:\n", argv[2], infldr);
         read_db(instlld, 0, 1, argv[2], NULL);
        }
      }
     else
      {
       printf("%s was not found on the index...\n" \
              "Similar sounding packages:\n", argv[2]);
       read_db(instlld, 0, 1, argv[2], NULL);
      }
    }

   /* search for a package */
   else if ((strcmp(argv[1], "-s")) == 0)
    {
     if (argc < 3)
      {
       printf("Usage: %s %s [pkg]\n", argv[0], argv[1]);
       exit(222);
      }
     else
      {
       printf("Search Result:\n");
       read_db(indexfile, 0, 1, argv[2], NULL);
      }
    }

   /* update the package index */
   else if ((strcmp(argv[1], "-u")) == 0)
    {
     printf("Updating index for %s... ", arch);
     download(repo, arch, "index.db", indexfile);
     printf("[Done]\n");
    }

   /* reinstall a (currently installed) package */
   else if ((strcmp(argv[1], "-ri")) == 0)
    {
     execlp(argv[0], argv[0], "-i", argv[2], "-ri", NULL);
    }

   /* set arch or repo */
   else if ((strcmp(argv[1], "setup")) == 0)
    {editcfg(cfgfile);}

   /* execute an installed package */
   else if ((strcmp(argv[1], "run")) == 0)
    {
     if (argc < 2)
      {
       printf("Usage: %s %s [pkg]\n", argv[0], argv[1]);
       exit(222);
      }
     else
      {
       if (read_db(instlld, 1, 1, argv[2], &intmp) == 1)
        {run_app(infldr, argv[2]);}
       else
        {printf("%s was not found in %s!\n", argv[2], instlld);}
      }
    }
   else
    {printf("Unknown option!\n");}
  }

return 0;}
