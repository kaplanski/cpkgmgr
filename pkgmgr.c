/*
 pkgmgr - a lightweight package manager
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
#include "mod_hash.h"
#define UUA __attribute__((__unused__))
#define VERSION "1.0"
#define PKGDIR "/pkgmgr"
#define INDIR "/bin"
#define ARCHDIR "/ARCH"
#define REPO "https://gitup.uni-potsdam.de/kaplanski/pkgmgr/raw/master/repo"

/* initial creation of various folders and the instlld file */
void first_run(char pkgdir[512], char indir[512], char archdir[512], \
               char arch[16], char instlld[512], char cfgfile[512], \
               int sup_arch UUA, char indexf[512], char repo[1024]){
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
 if (access(instlld, F_OK) == -1)
  {
   instlldfd = open(instlld, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
   if (instlldfd != -1)
    {
     write(instlldfd,"[PKGID:Name:Version]\n{001:pkgmgr:", strlen("[PKGID:Name:Version]\n{001:pkgmgr:"));
     write(instlldfd, VERSION, strlen(VERSION));
     write(instlldfd,"}\n", strlen("}\n"));
     close(instlldfd);
     printf("Initial installed.db created!\n");
    }
   else
    {
     printf("Failed to create installed.db\n");
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
     write(cfgfd_creat,"\narch=stable\n", strlen("\narch=stable\n"));
     close(cfgfd_creat);
     printf("Initial pkgmgr.cfg created!\n");
    }
   else
    {printf("Failed to create pkgmgr.cfg\n");}
  }

 /* download Package index */
 if((access(indexf, F_OK) == -1))
  {
   printf("Downloading index... ");
   fflush(stdout);
   download(repo, arch, "index.db", indexf);
  }
}

/* execute an installed app */
void run_app(char indir[512], char pkg[], char app[], int argc, char *argv[]){
 int i = 0, app_present = 0;
 char app_path[512] = "", tmp[512] = "", syscall[1024];
 char *end[4];
 end[0] = "";
 end[1] = ".sh";
 end[2] = ".bin";
 end[3] = ".py";
 strcpy(app_path, indir);
 strcat(app_path, "/");
 strcat(app_path, pkg);
 strcat(app_path, "/");

 if(access(app_path, F_OK) == -1)
  {printf("App %s is not installed!\n", app);}
 else
  {
   if (strcmp(argv[1], "list") == 0)
    {
     strcpy(syscall, "cd ");
     strcat(syscall, app_path);
     strcat(syscall, " && ls -1 -F | grep '*$'");
     #ifndef NO_CUT
     strcat(syscall, " 2>/dev/null | cut -d '*' -f1 | cut -d '.' -f1");
     #endif
     #ifdef NO_CUT
     strcat(syscall, " 2>/dev/null");
     #endif
     system(syscall);
     exit(0);
    }
   else if (strcmp(argv[1], "run") == 0)
    {
     if (argc<4)
      {
       printf("Usage: %s %s [pkg] [app]", argv[0], argv[1]);
       exit(255);
      }
     for (i = 0; i < (int)(sizeof(end)/sizeof(*end)); i++)
      {
       strcpy(tmp, app_path);
       strcat(tmp, app);
       strcat(tmp, end[i]);
       if(access(tmp, F_OK) != -1)
        {app_present = 1; break;}
      }
    }
  }

  if (app_present == 1)
   {
    if (argc<5)
     {system(tmp);} //some apps react to execvp(tmp, NULL); with segfault
    else
     {execvp(tmp, argv + 3);} //this however works fine
   }
   else
    {printf("App not present in app folder!\n"); exit(133);}
}

/* clean the ARCH folder */
void clean(char archdir[512]){

 char syscall[1024];
 printf("Cleaning %s... ", archdir);
 fflush(stdout);
 strcpy(syscall, "rm -rf ");
 strcat(syscall, archdir);
 strcat(syscall, "/*"); /**/
 system(syscall);
 printf("[DONE]\n");

}

/* handle dependencies */
void handle_deps(char pkgdir[512], char *prog[], char indexf[512], char instlldf[512], char pkgdeps[4096]){

 /* var init */
 int i = 0, ln_len = 1;
 const char *tmp = pkgdeps;
 char fname1[512] = "", fname2[512] = "", fname3[512] = "", \
      installme[10240] = "", *workln_ptr, tem[4096] = "", tem2[256] = "";

 /* get list length */
 while ((tmp = strstr(tmp, ",")))
  {ln_len++; tmp++;}

 workln_ptr = strtok(pkgdeps, ",");

 for (i=0; i<ln_len; i++)
  {
   strcpy(fname1, "/bin/");
   strcpy(fname2, "/usr/bin/");
   strcpy(fname3, "/usr/local/bin/");
   strcat(fname1, workln_ptr);
   strcat(fname2, workln_ptr);
   strcat(fname3, workln_ptr);

   if ((access(fname1, F_OK) != -1) || (access(fname2, F_OK) != -1) || (access(fname3, F_OK)) != -1)
    {printf(" >dependency %s natively present\n", workln_ptr);}
   else if (read_db(instlldf, 1, 1, workln_ptr, &tem2, &tem) == 1)
    {printf(" >dependency %s present\n", workln_ptr);}
   else
    {
     if (read_db(indexf, 1, 1, workln_ptr, &tem2, &tem) == 1)
      {
       strcat(installme, pkgdir);
       strcat(installme, *prog+1);
       strcat(installme, " -i ");
       strcat(installme, workln_ptr);
       strcat(installme, ";");
       printf(" >dependency %s marked to be installed\n", workln_ptr);
      }
     else
      {printf(" >dependency %s not found. abort\n", workln_ptr); exit(137);}
    }
   workln_ptr = strtok(NULL, ",");
  }
 if (strlen(installme) > 0)
  {
   printf("installme: %s\n", installme);
   system(installme);
   printf("dependencies installed");
  }
}

/* just an info display */
void info(void){
 printf("+-----------------------------------------------+\n" \
        "|    pkgmgr  - a lightweight package manager    |\n" \
        "|    Copyright (c) 2019 Jan-Daniel Kaplanski    |\n" \
        "+-----------------------------------------------+\n\n");
}

/* argument help */
void help(char *prg, char pkgdir[512], char indir[512], char arch[16]){

 info();

 printf("Usage: %s [-c|-da|-di|-h|-i|-r|-s|-u] [pkg]\n" \
        "   -h: displays this help\n" \
        "   -v: prints version\n" \
        "   -c: cleans the package folder of downloaded packages\n" \
        "   -u: updates the package index\n" \
        "   -i [pkg]: installs a package (-ri: reinstall)\n" \
        "   -r [pkg]: removes a package\n" \
        "   -s [pkg]: searches for a package in the package index\n" \
        "  -ui [pkg]: updates a package if newer version available\n" \
        "  -da: list all available packages for %s\n" \
        "  -di: list all installed packages\n" \
        "setup: change your architecture or repo\n" \
        "  run [pkg] [app] [args]: execute an installed app\n" \
        "  list [pkg]: list all apps of a package\n" \
        "Current binary folder: %s\n" \
        "Current pkgmgr folder: %s\n" \
        "Current architecture: %s\n\n", prg, arch, indir, pkgdir, arch);
 printf("Who needs root when you are %s?\n", getenv("USER"));
}

int main(int argc, char *argv[]){
 /* Var Init */
 int i = 0, j = 0, sup_arch = 0, cfgfd = -1, arch_set = 0, \
     repo_set = 0, pkgverlen = 0, instlldfd = -1, ri = 0, ui = 0;
 char pkgfldr[512] = "", infldr[512] = "", archfldr[512] = "", \
      instlld[512] = "", arch[16] = "stable", repo[1024] = REPO, \
      cfgfile[512] = "", cfgbuf[32] = "", cfgln[512] = "", cfgtmp[512], \
      indexfile[512] = "", intmp[256] = "", intmp2[256] = "", syscall[1024], \
      lctmp[12] = "", lctmp2[256] = "", pkgver[32] = "", indeps[4096] = "", \
      pkginver[256] = "", pkgonver[256] = "", pkghashf[256] = "", inbuf;
 const char *home = getenv("HOME");
 const char *archlst[1];
 archlst[0] = "stable";

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
 strcat(instlld, "/installed.db");

 strcpy(indexfile, pkgfldr);
 strcat(indexfile, "/index_");
 strcat(indexfile, arch);
 strcat(indexfile, ".db");

 /* create initial folders/files */
 first_run(pkgfldr, infldr, archfldr, arch, instlld, \
           cfgfile, sup_arch, indexfile, repo);

 if (argc == 1)
  {
   info();
   printf("At least one argument required!\n" \
          "Try: %s -h\n", argv[0]);
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

   #ifdef DEBUG
   if ((strcmp(argv[1], "test")) == 0)
    {printf("result: %d\n", read_db(indexfile, 1, 1, argv[2], &intmp, &indeps));}
   #endif

   /* display help */
   #ifdef DEBUG
   else if ((strcmp(argv[1], "-h")) == 0)
   #endif
   #ifndef DEBUG
   if ((strcmp(argv[1], "-h")) == 0)
   #endif
    {help(argv[0], pkgfldr, infldr, arch);}

   /* clean ARCH folder */
   else if ((strcmp(argv[1], "-c")) == 0)
    {clean(archfldr);}

   /* display all available packages */
   else if ((strcmp(argv[1], "-da")) == 0)
    {
     printf("Available Packages:\n");
     read_db(indexfile, 1, 0, NULL, NULL, NULL);
    }

   /* display all installed packages */
   else if ((strcmp(argv[1], "-di")) == 0)
    {
     printf("Installed Packages:\n");
     read_db(instlld, 1, 0, NULL, NULL, NULL);
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
       installer:

       /*chk if pkg is on index*/
       if (read_db(indexfile, 1, 1, argv[2], &intmp, &indeps) == 1)
        {
         chdir(archfldr);
         if (strcmp(argv[2], "pkgmgr") == 0)
          {printf("to update pkgmgr use -ui\n"); exit(255);}

         /* dependency phase */
         printf("Checking dependencies...\n");
         handle_deps(pkgfldr, &argv[0], indexfile, instlld, indeps);

         /* set intmp2 and pkghashf */
         strncpy(intmp2, intmp, strlen(intmp)-4);
         intmp2[strlen(intmp)-3] = '\0';

         strcpy(pkghashf, intmp2);
         strcat(pkghashf, ".sha3");

         /* package phase */
         if(access(intmp, F_OK) != -1)
          {printf("Using cached package...\n");}
         else
          {
           force_dwn:
           printf("Downloading %s... ", argv[2]);
           fflush(stdout);
           /* download package */
           download(repo, arch, intmp, NULL);

           /* download hash */
           download(repo, arch, pkghashf, NULL);
          }

         /* check hash */
         if ((chkhsh(intmp, pkghashf)) != 0)
          {
           inval_chkhsh:
           printf("Package %s is invalid. Continue? [y/N]: ", argv[0]);
           fflush(stdout);
           scanf("%s", &inbuf);
           if (((strcmp(&inbuf, "n") == 0) || (strcmp(&inbuf, "N") == 0)))
            {printf("abort\n"); exit(7);}
           else if (((strcmp(&inbuf, "y") == 0) || (strcmp(&inbuf, "Y") == 0)))
            {sleep(0); /* do nothing exept continue*/}
           else
            {printf("invalid\n"); fflush(stdin); goto inval_chkhsh;}
          }

         if (ri == 1)
          {install(intmp2, pkgfldr, infldr, argv[2], 1);}
         else if (ui == 1)
          {
           install(intmp2, pkgfldr, infldr, argv[2], 2);
           rem_db(instlld, argv[2]);
           goto gen_db_entry;
          }
         else
          {
           install(intmp2, pkgfldr, infldr, argv[2], 0);

           gen_db_entry:
           /* add to instlld */
           /* -6 = _v + .tgz */
           pkgverlen = (strlen(intmp) - strlen(argv[2]) - 6);
           strncpy(pkgver, intmp+strlen(argv[2])+2, pkgverlen);
           pkgver[pkgverlen+1] = '\0';

           /* generate the entry */
           /* get new ID via line count */
           sprintf(lctmp, "%d", read_db(instlld, 0, 0, NULL, NULL, NULL));
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

           if (argc < 4)
            {
             instlldfd = open(instlld, O_WRONLY | O_APPEND);
             if (instlldfd != -1)
              {
               write(instlldfd, lctmp2, strlen(lctmp2));
               close(instlldfd);
              }
             else
              {
               printf("Unable to open instlldfd! Abbort\n");
               exit(117);
              }
            }
          }
        }
       else
        {
         printf("%s was not found on the index...\n" \
                "Similar sounding packages:\n", argv[2]);
         read_db(indexfile, 0, 1, argv[2], NULL, NULL);
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
     else if (read_db(instlld, 1, 1, argv[2], &intmp, &indeps) == 1)
      {
       chdir(infldr);
       if(access(argv[2], F_OK) != -1)
        {
         printf("Removing %s... ", argv[2]);
         fflush(stdout);

         /* physical removal */
         strcpy(syscall, "rm -rf ");
         strcat(syscall, infldr);
         strcat(syscall, "/");
         strcat(syscall, argv[2]);
         system(syscall);

         /* db removal */
         rem_db(instlld, argv[2]);
         printf("[DONE]\n");
        }
       else
        {
         printf("%s was not found in %s!\n" \
                "Similar sounding packages:\n", argv[2], infldr);
         read_db(instlld, 0, 1, argv[2], NULL, NULL);
        }
      }
     else
      {
       printf("%s was not found on the index...\n" \
              "Similar sounding packages:\n", argv[2]);
       read_db(instlld, 0, 1, argv[2], NULL, NULL);
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
       read_db(indexfile, 0, 1, argv[2], NULL, NULL);
      }
    }

   /* update the package index */
   else if ((strcmp(argv[1], "-u")) == 0)
    {
     printf("Updating index for %s... ", arch);
     fflush(stdout);
     download(repo, arch, "index.db", indexfile);
    }

   /* reinstall a (currently installed) package */
   else if ((strcmp(argv[1], "-ri")) == 0)
    {
     ri = 1;
     goto installer;
    }

   /* set arch or repo */
   else if ((strcmp(argv[1], "-ui")) == 0)
    {
     if (argc < 3)
      {printf("Usage: %s %s [pkg]\n", argv[0], argv[1]); exit(255);}
     else
      {
       if (read_db(instlld, 1, 2, argv[2], &pkginver, NULL) == 1)
        {
          if (read_db(indexfile, 1, 2, argv[2], &pkgonver, NULL) == 1)
           {
            if (strcmp(pkginver, pkgonver) == 0)
             {printf("%s (%s) is already up-to-date\n", argv[2], pkginver);}
            else
             {
              read_db(indexfile, 1, 1, argv[2], &intmp, &indeps);
              ui = 1;
              chdir(archfldr);
              goto force_dwn;
             }
           }
          else
           {printf("%s is not available\n", argv[2]);}
        }
       else
        {printf("%s is not installed! (did you mean: -i)\n", argv[2]);}
      }
    }
   else if ((strcmp(argv[1], "setup")) == 0)
    {editcfg(cfgfile);}

   /* execute an installed package */
   else if ((strcmp(argv[1], "run")) == 0)
    {
     if (argc < 3)
      {
       printf("Usage: %s %s [pkg] [app] [args]\n", argv[0], argv[1]);
       exit(222);
      }
     else
      {
       if (read_db(instlld, 1, 1, argv[2], &intmp, &indeps) == 1)
        {run_app(infldr, argv[2], argv[3], argc, argv);}
       else
        {printf("%s was not found in %s!\n", argv[2], instlld);}
      }
    }
   else if ((strcmp(argv[1], "list")) == 0)
    {
     if (argc != 3)
      {printf("Usage: %s %s [pkg]\n", argv[0], argv[1]);}
     else
      {
        if (read_db(instlld, 1, 1, argv[2], &intmp, &indeps) == 1)
         {run_app(infldr, argv[2], NULL, argc, argv);}
        else
         {printf("%s was not found in %s!\n", argv[2], instlld);}
      }
    }
   else if ((strcmp(argv[1], "-v")) == 0)
    {
     printf("pkgmgr %s - a lightweight package manager\n" \
            "Copyright (c) 2019 Jan-Daniel Kaplanski\n" \
            "MIT/X11 LICENSE\n", VERSION);
    }
   else
    {printf("Unknown option!\n");}
  }

return 0;}
