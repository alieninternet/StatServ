/* main.cpp
 * The beginning, and the end, but not quite the middle
 * Copyright (c) 2002 Alien Internet Services
 */

#include "config.h"

#include <unistd.h>
#include <iostream.h>
#include <stdio.h>
#include <signal.h>

#include "daemon.h"
#include "sender.h"

#ifdef WITH_SNMP
# include "snmp.h"
#endif


/* sigHandler - Handler signals
 * Original 11/08/2001 simonb
 * 19/02/2002 simonb - Re-adapted
 */
void sigHandler(int sig)
{
#ifdef DEBUG
   cout << "Caught signal " << sig << " (" << sys_siglist[sig] << ")" << endl;
#endif
   
   switch (sig) {
      // Rehash... well.. checkpoint
    case SIGHUP:
      Daemon::checkpoint();
      break;
      // Die violently
    case SIGILL:
    case SIGTRAP:
#ifdef SIGEMT
    case SIGEMT:
#endif
#ifdef SIGBUS
    case SIGBUS:
#endif
#ifdef SIGSEGV
    case SIGSEGV:
#endif
#ifdef SIGSYS
    case SIGSYS:
#endif
#ifdef SIGURG
    case SIGURG:
#endif
    case SIGFPE:
#ifdef DEBUG
      cout << "Down I go :(" << endl;
#endif
      exit(1); // This could be a little nicer..
      break;

      // Die gracefully
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
    case SIGABRT:
#ifdef SIGXCPU
    case SIGXCPU: // should this be in the die violently section?
#endif
#ifdef SIGXFSZ
    case SIGXFSZ: // should this be in the die violently section?
#endif
      Daemon::shutdown(sys_siglist[sig]);
      break;
      
      // Everything else we ignore.
//    default:
      // ?!
   }
   
   // Reset the signal for future handling. Some os's do not need this??
   signal(sig, sigHandler);
}

/* main
 * Original 18/02/2002 simonb
 */
int main(void)
{
   // Initialise our 'bits' :)
   Daemon::init();
   Sender::init();
   SNMP::init();
   
   // Set up the signal handler happily
   for (register unsigned int i = NSIG; i--;) {
      signal(i, sigHandler);
   }
   
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
   
   // Shoot!
   Daemon::run();

   // De-init stuff
   Daemon::deinit();
   SNMP::deinit();
   
   // Clean up the signals (to be friendly)
   for (register unsigned int i = NSIG; i--;) {
      signal(i, SIG_DFL);
   }
   
   // Done. Return happily
   return 0;
}

