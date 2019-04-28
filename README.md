# pkgmgr - a lightweight package manager

**pkgmgr** is a lightweight package manager which installs  
packages to a folder inside the user's home directory.  
It's purpose is to have ones favorite software installed  
without needing root access or changing the system.  

*Functions:*  
- search, install, reinstall and remove packages  
- list available and installed packages  
- list and run installed apps  
- change repo and architecture  
  - repo: https://gitup.uni-potsdam.de/kaplanski/pkgmgr/tree/master/repo  
  - arch: python2, python3, i386, amd64  

*Installation:*  
git clone https://gitup.uni-potsdam.de/kaplanski/cpkgmgr.git  
./build.sh install  
  
*Dependencies:*  
- wget (package download)  
- awk (optional - formats output of 'list' function)  
  
*Upcomming:*  
- dependency check for packages (and auto install of deps) (priority: high)  
- repo will be restructured (priority: low)  
  
*Bugs:*  
- removal of packages from database: same as in bugfix commit f2686bdf and 11267877 in mod_dbinterpret.h  
  
**pkgmgr** Who needs root when you are $USER?
