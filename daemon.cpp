/* daemon.cpp
 * Daemon class (Main/Top-Level class)
 * Copyright (c) 2002 Alien Internet Services
 */

#include "config.h"

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#ifdef DEBUG
# include <iostream>
#endif

#include "daemon.h"
#include "parser.h"
#include "sender.h"


// The variables!
namespace Daemon {
   int sock = -1;
   int maxSock = -1;
   
   struct sockaddr_in addr;

   queue<String> outputQueue;
   
   bool connected = false;
   bool stop = false;
   bool sentPing = false;
   bool burstOk = false;
   
   time_t startTime;		
   time_t lastPing;		
   time_t lastCheckpoint;	
   time_t serverLastSpoke;	
   time_t currentTime;		

   unsigned long countUsers = 0;
   unsigned long countVersions = 0;
   unsigned long countConnects = 0;
   unsigned long countDisconnects = 0;
   unsigned long countTx = 0;
   unsigned long countRx = 0;
};


/* ~Daemon - Shutdown procedure for the daemon
 * Original 18/02/2002 simonb
 */
Daemon::~Daemon()
{
#ifdef DEBUG
   cout << "Killing myself..." << endl;
#endif   
   
   // Wipe the queue
   queueKill();
   
   // Close the socket
   ::close(sock);
}


/* initDaemon - Initialise the daemon
 * Original 18/02/2002 simonb
 */
void Daemon::initDaemon(void)
{
   // Set up the time variables
   startTime = lastPing = lastCheckpoint = serverLastSpoke = currentTime =
     time(NULL);

   // Wipe the queue
   queueKill();
   
   // Set up the address we are to connect to
   memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = CONNECT_SERVER;
   addr.sin_port = htons(CONNECT_PORT);
   
#ifdef DEBUG
   cout << "Initialised!" << endl;
#endif
}


/* checkpoint - Save data we have
 * Original 18/02/2002 simonb
 */
void Daemon::checkpoint(void)
{
#ifdef DEBUG
   String stats = String("[DEBUG] Checkpointing; Stats: ") + 
     String(countUsers) + " online (" +
     String(countConnects) + " connections, " +
     String(countDisconnects) + " disconnections, " +
     String(countVersions) + " CTCP VERSION replies); TX: " +
     String(countTx / 1024) + "k, RX: " +
     String(countRx / 1024) + "k; Up " +
     String(currentTime - startTime) + " secs";
   Sender::sendWALLOPS(stats);
   cout << stats << endl;
#endif
   
   // Finally, update the checkpoint time
   lastCheckpoint = currentTime;
}


/* connect - Connect to the server
 * Original 18/02/2002 simonb
 */
bool Daemon::connect(void)
{
   // Force the belief that we are not connected (well, we should not be)
   connected = false;

   // Make sure whatever WAS the socket is no longer
   if (sock >= 0) {
#ifdef DEBUG
      cout << "Closing stale socket" << endl;
#endif
      close(sock);
      sock = -1;
   }
   
   // Wipe out the queue, it should be empty since we are connecting again
   queueKill();

   // Oh, and we have not received a completed burst yet, naturally
   burstOk = false;
   
   // Grab the socket
   if ((sock = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
#ifdef DEBUG
      perror("socket");
#endif
      return false;
#ifdef DEBUG
   } else {
      cout << "Using socket " << sock << endl;
#endif
   }
   
   // Connect!
   if (::connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
#ifdef DEBUG
      perror("connect");
#endif
      return false;
#ifdef DEBUG
   } else {
      cout << "Connection called to " << hex << CONNECT_SERVER << 
	", port " << dec << CONNECT_PORT << endl;
#endif
   }

   // Set non-blocking operation (get the flags, then modify them)
   long flags = 0;
   if (fcntl(sock, F_GETFL, &flags) < 0) {
#ifdef DEBUG
      perror("fcntl F_GETFL");
#endif
      return false;
   }
   if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
#ifdef DEBUG
      perror("fcntl F_SETFL");
#endif
      return false;
#ifdef DEBUG
   } else {
      cout << "Non-blocking flag set" << endl;
#endif
   }
   
   // Fix up maxSock for select()
   maxSock = sock + 1;

   // Finally, queue our connection lines
   queueAdd("PASS :" CONNECT_PASSWORD);
   queueAdd("SERVER " MY_SERVERNAME " 1 995639470 995691000 J13 :" 
	    MY_SERVERDESC);
   Sender::sendBurst();
   
   // If we got here, she must be apples.
   connected = true;
   return true;
}


/* handleInput - Chew on some data
 * Original 18/02/2002 simonb
 */
bool Daemon::handleInput(void)
{
   static char buf[1024];
   int pos = 0, nb = 0;
   char c = 0;
 
   // Loop until we get to the end of a line
   while (c != '\n') {
      nb = ::read(sock, &c, 1);
      switch(nb) {
       case 0: // nothing read
	 return true;
       case -1: // we broke perhaps?
	 if ((errno != EINTR) && (errno != EAGAIN)) {
	    return true;
	 }
#ifdef DEBUG
	 perror("read");
#endif
	 return false;
      }
      
      // Add the character if it was read ok
      if (nb) {
	 buf[pos++] = c;
      }
   }
   
   /* Check if the string has a \r\n style ending or the normal \r, and
    * replace that with a null char
    */
   if ((pos > 1) && buf[pos-2] == '\r') {
      buf[pos-2] = '\0';
   } else {
      buf[pos-1] = '\0';
   }
   
   // Make a nice useful string out of the buffer
   String line(buf);
   
   // Update the bytes RX counter
   countRx += line.length();
   
   // Send it to the parser
   Parser::parseLine(line);

   // 'Touch' the last spoke variable
   serverLastSpoke = currentTime;
   
   // Return peachy.
   return true;
}


/* writeData - Write a line of data to the socket
 * Original 18/02/2002 simonb
 */
bool Daemon::writeData(String &line)
{
   // Make sure we are connected...
   if (!sock) {
      return false;
   }
   
   // Update the bytes TX counter
   countTx += line.length();

   // Write the line
   if (::write(sock, (const char *)line, line.length()) +
       ::write(sock, "\n", 1) != line.length() + 1) {
      return false;
   }
   
   return true;
}


/* run - The command that loops
 * Original 18/02/2002 simonb
 */
void Daemon::run(void)
{
   fd_set inputSet, outputSet;
   struct timeval timer;
   
#ifdef DEBUG
   cout << "Connecting..." << endl;
#endif

   // Connect!
   connect();

#ifdef DEBUG
   cout << "Entering main loop..." << endl;
#endif
   
   // The main loop.. not very exciting is it?
   while (!stop) {
      // Grab the current time
      time(&currentTime);

      // Reset the timer
      timer.tv_sec = 1;
      timer.tv_usec = 0;

      // Reset the file descriptor sets
      FD_ZERO(&inputSet);
      FD_ZERO(&outputSet);
      FD_SET(sock, &inputSet);
      if (queueReady()) {
	 FD_SET(sock, &outputSet);
      }
      
      // Check for input/output readiness
      switch (select(maxSock, &inputSet, &outputSet, NULL, &timer)) {
       case 0: // Select timed out
	 break;
       case -1: // Select broke
#ifdef DEBUG
	 cout << "Select prematurely returned!" << endl;
	 perror("select");
#endif
	 break;
       default: // Select says there is something to process
	 // Something from the socket?
	 if (FD_ISSET(sock, &inputSet)) {
	    if (!handleInput()) {
#ifdef DEBUG
	       cout << "Reconnecting... (Input handling error)" << endl;
#endif	   
	       connect();
	    }
	 }

	 // Are we allowed to send to the socket?
	 if (FD_ISSET(sock, &outputSet)) {
	    if (!queueFlush()) {
#ifdef DEBUG
	       cout << "Reconnecting... (Queue flushing error)" << endl;
#endif
	       connect();
	    }
	 }
      }
      
//      // Ping the server and calculate our current client <-> server lag
//      if (((currentTime >= (time_t)(lastPing + PING_TIME)) ||
//	   (currentTime >= (time_t)(serverLastSpoke + PING_TIME))) &&
//	  !sentPing) {
//	 sendPing();
//	 lastPing = currentTime;
//	 sentPing = true;
//   }
   
//      // If the server is ignoring us, it is probably dead. Time to reconnect.
//      if ((currentTime >= (time_t)(serverLastSpoke + TIMEOUT)) ||
//	  ((currentTime >= (time_t)(lastPing + PING_TIME)) &&
//	   sentPing)) {
//#ifdef DEBUG
//	 cout << "Reconnecting... (Server timed out)" << endl;
//#endif
//	 
//	 sentPing = false;
//	 connect();
//      }

      // Is it time for the checkpoint sequence to run?
      if (currentTime >= (time_t)(lastCheckpoint + CHECKPOINT_TIME)) {
	 checkpoint();
      }
   }
}

