#!/usr/bin/env bash

CC=
PROG="pkgmgr"
PROGPATH="$HOME/$PROG"
FLAGS="-std=gnu99 -Wall -Wextra -pedantic -Wno-nonnull"

if [ "$CFLAGS" != "" ]; then
   FLAGS="$FLAGS $CFLAGS"
fi

#check if someone builds in the wrong place
if [ "$PWD" == "$PROGPATH" -o "$PWD" == "/usr$PROGPATH" ]; then
   echo "Building inside $PROGPATH is not allowed"
   cd ..
   mv $PROGPATH $PROGPATH"_build"
   cd $PROGPATH"_build"
   exec ./build.sh
fi

#building procedure
if [ "$1" == "help" ];then
   echo "Usage: $0 [cmd]"
   echo "       all: builds $PROG"
   echo "     noawk: builds $PROG without awk used in list function"
   echo "   install: installs $PROG in $PROGPATH"
   echo "     clean: cleans the build dir"
   echo "   alias to 'all' is $0 w/o any cmd"
elif [ "$1" == "" -o "$1" == "all" -o "$1" == "noawk" ]; then
   #set compiler
   echo "Setting compiler..."
   if [ -f /usr/bin/gcc -o -f /usr/local/bin/gcc ]; then
      CC=gcc
      echo "   using gcc"
   elif [ -f /usr/bin/clang -o -f /usr/local/bin/clang ]; then
      CC=clang
      echo "   using clang"
   else
      echo "   Neither clang nor gcc was found on your system!"
      exit 255
   fi
   echo "   DONE"

   #dependency check
   echo "Checking dependencies..."
   if [ ! -f /usr/bin/wget -a ! -f /usr/local/bin/wget ]; then
      echo "   wget was not found on your system"
      echo "   Please install wget to use $PROG!"
      exit 177
   fi
   if [ "$1" == "noawk" -o ! -f /usr/bin/awk -a ! -f /usr/local/bin/awk ]; then
      echo "   not using awk"
      FLAGS="$FLAGS -DNO_AWK"
   fi
   echo "   DONE"

   #it's showtime
   echo
   echo "$CC $FLAGS $PROG.c -o $PROG"
   $CC $FLAGS $PROG.c -o $PROG
   echo

#install/copy to the program folder at $PROGPATH
elif [ "$1" == "install" -a -f $PROG ]; then
   if [ ! -f $PROGPATH/$PROG ];then
      #initial execution (creates folders and such)
      echo "First execution"
      echo
      ./$PROG "-h"
      echo
   fi
   echo "$PROG is copied to $PROGPATH"
   cp $PROG $PROGPATH

#or build first if it wasn't build
elif [ "$1" == "install" -a ! -f $PROG ]; then
   $0 && $0 install

#clean the build dir of binaries
elif [ "$1" == "clean" ];then
   rm -rf *.o
   rm -rf $PROG
fi
#echo "All done, have fun!"
