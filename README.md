# [Unmaintained] pkgmgr - a lightweight package manager

Update May/2023: I plan on doing a complete rewrite of the code base.   
I had originally written this project during my time at uni as a   
quick-and-dirty solution that only needed to work for me.   
I would not recommend to use it, as it is not written with security in mind.   
However, in the upcomming rewrite I plan to focus on security through best-practice.

**pkgmgr** is a lightweight package manager which installs  
packages into a folder inside the users home directory.  
It's purpose is to have ones favorite software installed  
without needing root access or changing the system.  

*Functions:*  
- search, install, reinstall and remove packages  
- list available and installed packages  
- list and run installed apps  
- change repo and architecture  
  - repo: https://github.com/kaplanski/pkgmgr/tree/master/repo  
  - arch: stable  
- dependency checks  
- "pkgmgr -ui pkgmgr" grabs the newest version, compiles and replaces the current  
- hashing via a crc64-iso implementation  

*Installation:*  
git clone https://github.com/kaplanski/cpkgmgr.git  
cd cpkgmgr && ./build.sh install  
  
*Dependencies:*  
- wget (package download)  
- awk (optional - formats output of 'list' function)   

*Bugs:*  
no known bugs  

**pkgmgr** Who needs root when you are $USER?
