/* daemon.cpp
 * Daemon class (Main/Top-Level class)
 * Copyright (c) 2002 Alien Internet Services
 */

#include "config.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <fstream>
#include <iostream>

#include "daemon.h"
#include "parser.h"
#include "sender.h"

#ifdef WITH_SNMP
# include "snmp.h"
#endif


// The variables!
namespace Daemon {
   int sock = -1;
   int maxSock = -1;
   
   char *inputBuffer = 0;
   unsigned int inputBufferSize = BUFFER_CHUNK_SIZE;
   unsigned int inputBufferPosition = 0;

   struct sockaddr_in addr;

   queue <String> outputQueue;
   Daemon::versions_map_t versions;
   Daemon::version_request_map_t versionRequests;
   set <String> burstServers;
   Daemon::ignore_set_t ignoreList;
   
   bool connected = false;
   bool stopping = false;
   bool sentPing = false;
   bool burstOk = false;
   
   time_t startTime;		
   time_t lastPing;		
   time_t lastCheckpoint;	
   time_t serverLastSpoke;
   time_t disconnectTime = 0;	// Make us want to connect upon startup
   time_t currentTime;		

   unsigned long countVersions = 0;
   unsigned long countVersionsTotal = 0;
   unsigned long countVersionSpoofs = 0;
   unsigned long countUserConnects = 1;		// Include us again..
   unsigned long countTx = 0;
   unsigned long countRx = 0;
};


/* init - Initialise the daemon
 * Original 18/02/2002 simonb
 */
void Daemon::init(void)
{
#ifdef DEBUG
   cout << "Initialising Daemon..." << endl;
#endif
   
   // Set up the time variables
   startTime = lastPing = lastCheckpoint = serverLastSpoke = currentTime =
     time(NULL);

   // Wipe the lists we have
   queueKill();
   versions.clear();
   versionRequests.clear();
   burstServers.clear();
   ignoreList.clear();

   ifstream file;
   String line = "";

   // Load the version data file
   file.open(FILE_VERSIONS);
   
   if (!file) {
      cout << "Could not open " FILE_VERSIONS " for reading" << endl;
   } else {
      // Run through the file and read each line
      while (!file.eof()) {
	 file >> line;
	 
	 // Don't bother with the line if it is blank or has a # at the start
	 if (line.length() && (line[0] != '#')) {
	    // Break the line up
	    StringTokens st(line);
	    long count = st.nextToken().toLong();
	    
	    // Make sure that integer conversion worked (or it is not 0)
	    if (!count) {
	       continue;
	    }
	    
	    versions[st.rest()] = count;
	    countVersionsTotal += count;
	 }
      }
   }
   file.close();
   
   // Load the ignore list data
   file.open(FILE_IGNORES);

   // Make sure we did actually open that file
   if (!file) {
      cout << "Could not open " FILE_IGNORES " for reading" << endl;
   } else {
      // Run through the file and read each line
      while (!file.eof()) {
	 file >> line;
	 
	 // Don't bother with the line if it is blank or has a # at the start
	 if (line.length() && (line[0] != '#')) {
	    addIgnore(line);
	 }
      }
   }
   file.close();

   // Do the initial malloc() for the input buffer
   if (!(inputBuffer = (char *)malloc(inputBufferSize))) {
      cout << "Could not allocate input buffer memory" << endl;
      perror("malloc");
      exit(1);
   }
   
   // Set up the address we are to connect to
   memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(CONNECT_SERVER);
   addr.sin_port = htons(CONNECT_PORT);
}


/* deinit - De-init the daemon
 * Original 18/02/2002 simonb
 */
void Daemon::deinit(void)
{
#ifdef DEBUG
   cout << "Killing myself..." << endl;
#endif   
   
   // Wipe the queue
   queueKill();

   // Kill the input buffer allocation
   free(inputBuffer);
   
   // Close the socket
   ::close(sock);
}


/* gotVersion - Process a received version line, if we need to
 * Original 18/02/2002 simonb
 */
void Daemon::gotVersion(String const &origin, String const &data)
{
   if ((versionRequests[origin.IRCtoLower()] + VERSION_REPLY_TIMEOUT) >=
       currentTime) {
#ifdef DEBUG
      cout << "Version Reply: " << data << endl;
#endif
      countVersions++;
      countVersionsTotal++;
      versions[data]++;
   } else {
#ifdef DEBUG
      cout << "Spoof version? From " << origin << " (" << data << ')' << endl;
#endif
      countVersionSpoofs++;
   }
   
   // Remove the nickname from the version request map
   versionRequests.erase(origin.IRCtoLower());
}


/* checkpoint - Save data we have
 * Original 18/02/2002 simonb
 */
void Daemon::checkpoint(void)
{
#ifdef DEBUG
   cout << "Checkpoint" << endl;
#endif

   ofstream file;

   // Write out the version data, as we see it.
   file.open(FILE_VERSIONS);
   
   // Check...
   if (file) {
      for (Daemon::versions_map_t::iterator it = versions.begin();
	   it != versions.end(); it++) {
	 file << (*it).second << " " << (*it).first << endl;
      }
#ifdef DEBUG
   } else {
      cout << "Could not open " FILE_VERSIONS " for writing" << endl;
#endif
   }
   file.close();
   
#ifdef IGNORE_ALLOWED
   // Write out the ignore data, if we are accepting new ignores
   file.open(FILE_IGNORES);
   
   // Check. Ignore the state if we didn't...
   if (file) {
      for (Daemon::ignore_set_t::iterator it = ignoreList.begin();
	   it != ignoreList.end(); it++) {
	 file << *it << endl;
      }
# ifdef DEBUG
   } else {
      cout << "Could not open " FILE_IGNORES " for writing" << endl;
# endif
   }
   file.close();
#endif

   // Wipe out expired entries in the version request map
   for (;;) {
again:
      for (Daemon::version_request_map_t::iterator it = 
	   versionRequests.begin(); it != versionRequests.end(); it++) {
	 if (((*it).second + VERSION_REPLY_TIMEOUT) >= currentTime) {
	    versionRequests.erase(it);
	    goto again;
	 }
      }
      
      // If the loop ended successfully, break out of the nearly-endless-loop
      break;
   }
   
   // Finally, update the checkpoint time
   lastCheckpoint = currentTime;
}


/* shutdown - Start the server shutdown thingy
 * Original 19/02/2002 simonb
 */
void Daemon::shutdown(String const &reason)
{
#ifdef DEBUG
   cout << "Shutting down: " << reason << endl;
#endif
   
   // Tell ourselves it is time to go bye byes
   stopping = true;
   
   // Queue an SQUIT for ourselves too
   Sender::sendSQUIT(reason);
}

  
/* connect - Connect to the server
 * Original 18/02/2002 simonb
 */
bool Daemon::connect(void)
{
#ifdef DEBUG
   cout << "Connecting..." << endl;
#endif

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
   burstOk = sentPing = connected = false;
   gotSOB(CONNECT_SERVERNAME);
   
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
      cout << "Connection called to " << 
	inet_ntoa(addr.sin_addr) << ", port " << 
	CONNECT_PORT << endl;
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
   queueAdd("SERVER " MY_SERVERNAME " 1 " DODGEY_SERVER_TS_1 " "
	    DODGEY_SERVER_TS_2 " J13 :" 
	    MY_SERVERDESC);
   Sender::sendBurst();
   
   // If we got here, she must be apples.
   connected = true;
   return true;
}


/* disconnect - Disconnect from the server
 * Original 19/02/2002 simonb
 */
void Daemon::disconnect(void)
{
   // Close the socket
   close(sock);
   sock = -1;
   
   // Make sure we 'know' we are disconnected
   connected = false;
   disconnectTime = currentTime;
}


/* handleInput - Chew on some data
 * Original 18/02/2002 simonb
 */
bool Daemon::handleInput(void)
{
   int readBytes;
   char c = 0;
 
   // Loop until we get to the end of a line
   while (c != '\n') {
      readBytes = ::read(sock, &c, 1);
      switch(readBytes) {
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
      
      // Do we need to reallocate the input buffer?
      if ((inputBufferPosition + 1) >= inputBufferSize) {
	 inputBufferSize += BUFFER_CHUNK_SIZE;
#ifdef DEBUG
	 cout << "Reallocating inputBuffer to " << inputBufferSize << 
	   " bytes"<< endl;
#endif
	 if (!(inputBuffer = 
	       (char *)realloc((char *)inputBuffer, inputBufferSize))) {
#ifdef DEBUG
	    cout << "Could not reallocate more memory for the input buffer!" <<
	      endl;
#endif
	    exit(1);
	 }
      }
      
      // Add the character if it was read ok
      if (readBytes) {
	 inputBuffer[inputBufferPosition++] = c;
      }
   }
   
   /* Check if the string has a \r\n style ending or the normal \r, and
    * replace that with a null char
    */
   if ((inputBufferPosition > 1) && 
       (inputBuffer[inputBufferPosition - 2] == '\r')) {
      inputBuffer[inputBufferPosition - 2] = '\0';
   } else {
      inputBuffer[inputBufferPosition - 1] = '\0';
   }
   
   // Reset the input buffer position
   inputBufferPosition = 0;
   
   // Make a nice useful string out of the buffer
   String line(inputBuffer);
   
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
   cout << "Entering main loop..." << endl;
#endif
   
   // The main loop.. not very exciting is it?
   for (;;) {
      // Grab the current time
      time(&currentTime);

      // Reset the timer
      timer.tv_sec = 1;
      timer.tv_usec = 0;

      // Reset the file descriptor sets
      FD_ZERO(&inputSet);
      FD_ZERO(&outputSet);
      if (connected) {
	 // Only add the input set if we are not wanting to stop
	 if (!stopping) {
	    FD_SET(sock, &inputSet);
	 }
	 
	 // Only add to the output set if we need to output
	 if (queueReady()) {
	    FD_SET(sock, &outputSet);
	 }
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
	       cout << "Disconnecting... (Input handling error)" << endl;
#endif	   
	       disconnect();
	    }
	 }

	 // Are we allowed to send to the socket?
	 if (FD_ISSET(sock, &outputSet)) {
	    if (!queueFlush()) {
#ifdef DEBUG
	       cout << "Disconnecting... (Queue flushing error)" << endl;
#endif
	       disconnect();
	    }
	 }
      }
      
      // Ping the server and calculate our current client <-> server lag
      if (!sentPing &&
	  ((currentTime >= (time_t)(lastPing + PING_TIME)) ||
	   (currentTime >= (time_t)(serverLastSpoke + PING_TIME)))) {
#ifdef DEBUG
	 cout << "Pinging the remote server..." << endl;
#endif
	 Sender::sendPING();
	 lastPing = currentTime;
	 sentPing = true;
      }
   
      // If we are not stopping, run some checks
      if (!stopping) {
	 // If the server is ignoring us, it is probably dead. Reconnect.
	 if ((currentTime >= (time_t)(serverLastSpoke + TIMEOUT)) ||
	     (sentPing &&
	      (currentTime >= (time_t)(lastPing + PING_TIME)))) {
#ifdef DEBUG
	    cout << "Disconnecting... (Server timed out)" << endl;
#endif
	    sentPing = false;
	    disconnect();
	 }
	 
	 // Is it time for the checkpoint sequence to run?
	 if (currentTime >= (time_t)(lastCheckpoint + CHECKPOINT_TIME)) {
	    checkpoint();
	 }
	 
	 // If we are disconnected, is it time we reconnected?
	 if (!connected &&
	     (currentTime >= (time_t)(disconnectTime + RECONNECT_DELAY))) {
	    connect();
	 }
	 
#ifdef WITH_SNMP
	 // Give the SNMP agent module a slice of our time
	 SNMP::slice();
#endif
      } else if (!queueReady() || !connected) {
	 // We are stopping, and the output queue is empty: Break the loop
	 break;
      }
   }
   
   // Do our last checkpoint
   checkpoint();
   
   // Done!!
}

