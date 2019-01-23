#Â!usr/bin/env bash

CC=
PROG="cpkgmgr"
PROGPATH="$HOME/$PROG"
FLAGS="-std=gnu99 -Wall -Wextra -pedantic"

#check if someone builds in the wrong place
if [ "$PWD" == "$PROGPATH" -o "$PWD" == "/usr$PROGPATH" ]; then
   echo "Building inside $PROGPATH is not allowed"
   cd ..
   mv $PROGPATH $PROGPATH"_build"
   cd $PROGPATH"_build"
   exec ./build.sh
fi

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
echo "   DONE"

#it's showtime
echo
echo "$CC $FLAGS $PROG.c -o $PROG"
$CC $FLAGS $PROG.c -o $PROG
echo

#initial execution (creates folders and such)
echo "First execution"
echo
./$PROG "-h"
echo
echo "$PROG is copied to $PROGPATH"
cp $PROG $PROGPATH
echo "All done, have fun!"
