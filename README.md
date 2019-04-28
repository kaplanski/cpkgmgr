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
- "pkgmgr -i pkgmgr" grabs the newest version, compiles and replaces the current  

*Installation:*  
git clone https://gitup.uni-potsdam.de/kaplanski/cpkgmgr.git  
cd cpkgmgr && ./build.sh install  
  
*Dependencies:*  
- wget (package download)  
- awk (optional - formats output of 'list' function)  
  
*Upcomming:*  
- dependency check for packages (and auto install of deps) (priority: high)  
- updating packages (priority: medium)  
- repo will be restructured (priority: low)  
  
*Bugs:*  
no known bugs  
  
**pkgmgr** Who needs root when you are $USER?
