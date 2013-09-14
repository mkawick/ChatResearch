
#include "../Platform.h"
#include "../Logging/server_log.h"

#include <cstdlib>
#include <cstdio>
//#include <list>
//#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <string>
#include <iostream>


#if PLATFORM != PLATFORM_WINDOWS
//#include "config.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#else
#endif

using namespace std;

void daemonize( const char* PACKAGE_NAME )
{
   cout << "daemonize started" << endl;

   string LOCKFILE = "/var/run/";
   LOCKFILE += PACKAGE_NAME;
   LOCKFILE += ".pid";

   cout << "daemonize lockfile: " << LOCKFILE << endl;

   string err = "Can't open lockfile ";
   err += LOCKFILE;
   err += " err=";
   err += strerror(errno);

#if PLATFORM != PLATFORM_WINDOWS
   

   LogMessage(LOG_PRIO_ERR, "Fork\n");

   pid_t pid = fork();
   if (pid < 0)
   {
      fprintf(stderr, "Could not fork\n");
      exit(-1);
   }
   else if (pid > 0)
   {
      cout << "Fork success" << endl;
      LogMessage(LOG_PRIO_ERR, "Fork Success\n");
      exit(0); //parent
   }

   setsid();

   LogOpen();

   // close all open files
   struct rlimit rl;
   if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
   {
      LogMessage(LOG_PRIO_ERR, "Can't get file limit");
      exit(1);
   }

   int rlim_max = rl.rlim_max;
   for (int i = 0; i < rlim_max; ++i)
   {
      //close(i);
   }

   // attach the first 3 file descriptors to /dev/null
   //open("/dev/null", O_RDWR);
   //dup(0);
   //dup(0);

   cout << "opening lock file" << endl;
   LogMessage(LOG_PRIO_ERR, "opening lock file\n");
   int fd = open(LOCKFILE.c_str(), O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
   if (fd < 0)
   {
      cout << err << endl;
      LogMessage(LOG_PRIO_ERR, err.c_str() );
      exit(1);
   }

   struct flock fl;
   fl.l_type = F_WRLCK;
   fl.l_start = 0;
   fl.l_whence = SEEK_SET;
   fl.l_len = 0;

   if (fcntl(fd, F_SETLK, &fl) < 0)
   {
      if (errno == EACCES || errno == EAGAIN)
      {
         LogMessage(LOG_PRIO_ERR, "Daemon is already running");
         exit(1);
      }
   }
   
   ftruncate(fd, 0);

   char buf[12];
   sprintf(buf, "%d", getpid());
   write(fd, buf, strlen(buf));
   
   //TODO: setuid to non-root

   string dir = "/usr/local/share/";
   dir += PACKAGE_NAME;
   chdir( dir.c_str() ); 
   
#endif

   cout << "daemonize finished" << endl;
   LogMessage(LOG_PRIO_ERR, "daemonize finished\n");
}

