/* main.cpp
 * The beginning, and the end, but not quite the middle
 * Copyright (c) 2002 Alien Internet Services
 */

#include "config.h"

#include <unistd.h>
#include <iostream.h>
#include <stdio.h>

#include "daemon.h"

/* main
 * Original 18/02/2002 simonb
 */
int main(void)
{
#ifndef DEBUG
   switch (fork()) {
    case -1:
      cout << "Could not run in the background. Exiting..." << endl;
      perror("fork");
      exit(1);
    case 0:
      break;
    default:
      cout << "Running in the background..." << endl;
      exit(0);
   }
#else
   cout << "Running in the foreground (debugging)..." << endl;
#endif
   
   // Initialise the server
   Daemon::initDaemon();

   // Shoot!
   Daemon::run();
   
   // Done. Return happily
   return 0;
}

