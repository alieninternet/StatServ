/* daemon.h
 * Daemon class (Main/Top-Level class)
 * Copyright (c) 2002 Alien Internet Services
 */

#ifndef __DAEMON_H_
# define __DAEMON_H_

# include "config.h"

# include <time.h>
# include <unistd.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <queue>

# include "str.h"

class Daemon {
 private:
   static int sock;
   static int maxSock;
   
   static struct sockaddr_in addr;

   static queue<String> outputQueue;
   
   static bool connected;
   static bool stop;
   static bool sentPing;
   static bool burstOk;
   
   static time_t startTime;			// Time the daemon was started
   static time_t lastPing;			// Time we last pinged
   static time_t lastCheckpoint;		// Time we last checkpointed
   static time_t serverLastSpoke;		// Time the server last spoke
   static time_t currentTime;			// The time "now"

   static unsigned long countUsers;		// Current users on-line
   static unsigned long countVersions;		// Version replies this session
   static unsigned long countConnects;		// Connections this session
   static unsigned long countDisconnects;	// Disconnections this session
   static unsigned long countTx;		// Bytes sent
   static unsigned long countRx;		// Bytes received
   
   Daemon(void)					// Disabled constructor
     {};
   ~Daemon(void);

   static void checkpoint(void);		// Checkpoint the databases
   static bool connect(void);			// Connect, duh

   static bool handleInput(void);		// Handle input..
   static bool writeData(String &);		// Write out data
   
   static bool queueReady(void)			// Check if the queue has data
     {
	return !outputQueue.empty();
     };
   
   static bool queueFlush(void)			// Flush the queue by a line
     {
	if (writeData(outputQueue.front())) {
	   outputQueue.pop();
	   return true;
	}
	return false;
     };
   
   static void queueKill(void)			// Wipe the queue
     {
	while (!outputQueue.empty()) {
	   outputQueue.pop();
	}
     };
   
 public:
   static void initDaemon(void);		// Initialise the daemon
   
   static void queueAdd(String &line)		// Add a line to the queue
     {
	outputQueue.push(line);
     };
   static void queueAdd(String line)
     {
	outputQueue.push(line);
     };

   static void gotEOB(void)			// Received an end of burst
     {
	burstOk = true;
     };
   
   static bool inBurst(void)			// Are we in burst mode?
     {
	return !burstOk;
     };

   static void userOn(void)			// A user signing on
     {
	countUsers++;
	countConnects++;
     };
   
   static void userOff(void)			// A user signing off
     {
	countUsers--;
	countDisconnects++;
     };
   
   static void gotVersion(String &version)	// Got a version string
     {
	countVersions++;
     };
   
   static void gotPong(void)			// Received a server pong
     {
	sentPing = false;
     };
   
   static void run(void);			// Main loop
};

#endif
