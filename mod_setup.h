#ifndef _MOD_SETUP_H
#define _MOD_SETUP_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>

extern void editcfg(char cfgpath[512]){
 /* Var Init */
 char editor[16] = "";

 /* set editor */
 if ((access("/bin/nano", F_OK)) != -1)
  {strcpy(editor, "nano");}
 else if ((access("/usr/bin/nano", F_OK)) != -1)
  {strcpy(editor, "nano");}
 else if ((access("/usr/local/bin/nano", F_OK)) != -1)
  {strcpy(editor, "nano");}
 else if ((access("/bin/vi", F_OK)) != -1)
  {strcpy(editor, "vi");}
 else if ((access("/usr/bin/vi", F_OK)) != -1)
  {strcpy(editor, "vi");}
 else if ((access("/usr/local/bin/vi", F_OK)) != -1)
  {strcpy(editor, "vi");}
 else if ((access("/bin/vim", F_OK)) != -1)
  {strcpy(editor, "vim");}
 else if ((access("/usr/bin/vim", F_OK)) != -1)
  {strcpy(editor, "vim");}
 else if ((access("/usr/local/bin/vim", F_OK)) != -1)
  {strcpy(editor, "vim");}
 else if ((access("/bin/emacs", F_OK)) != -1)
  {strcpy(editor, "emacs");}
 else if ((access("/usr/bin/emacs", F_OK)) != -1)
  {strcpy(editor, "emacs");}
 else if ((access("/usr/local/bin/emacs", F_OK)) != -1)
  {strcpy(editor, "emacs");}
 else
  {printf("No editor (nano, vi, vim, emacs) found!"); exit(225);}

 /* execute editor to edit cfg file */
 execlp(editor, "edit config", cfgpath, NULL);
}
#endif
