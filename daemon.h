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
# include <arpa/inet.h>
# include <queue>
# include <map>
# include <set>

# include "str.h"

class Daemon {
 private:
   typedef map <String, long> versions_map_t;
   typedef map <String, time_t> version_request_map_t;
   typedef set <String> ignore_set_t;
   
   static int sock;				// Our connection socket
   static int maxSock;				// Max socket for select()

   static char *inputBuffer;			// Socket input buffer
   static unsigned int inputBufferSize;		// Socket input buffer size
   static unsigned int inputBufferPosition;	// Socket input buffer position
   
   static struct sockaddr_in addr;		// Place to connect to

   static queue <String> outputQueue;		// Socket output data queue
   static versions_map_t versions;		// The version count map
   static version_request_map_t versionRequests;// The version request map
   static set <String> burstServers;		// List of servers bursting
   static ignore_set_t ignoreList;		// List of ignored nicknames
   
   static bool connected;			// Are we connected?
   static bool stopping;			// Are we to stop?
   static bool sentPing;			// Have we sent a ping?
   static bool burstOk;				// Has our link bursted ok?
   
   static time_t startTime;			// Time the daemon was started
   static time_t lastPing;			// Time we last pinged
   static time_t lastCheckpoint;		// Time we last checkpointed
   static time_t serverLastSpoke;		// Time the server last spoke
   static time_t disconnectTime;		// Time we disconnected
   static time_t currentTime;			// The time "now"

   static unsigned long countVersions;		// Version replies this session
   static unsigned long countVersionsTotal;	// Version replies in memory
   static unsigned long countVersionSpoofs;	// Unwanted version replies
   static unsigned long countUserConnects;	// Connections this session
   static unsigned long countTx;		// Bytes sent
   static unsigned long countRx;		// Bytes received
   
   Daemon(void) {};
   ~Daemon(void) {};

   static bool connect(void);			// Connect to the server
   static void disconnect(void);		// Disconnect
   
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
   static void init(void);			// Initialise the daemon
   static void deinit(void);			// De-init the daemon
   static void checkpoint(void);		// Checkpoint the databases
   static void shutdown(String const &);	// Shutdown

   static void queueAdd(String &line)		// Add a line to the queue
     {
	outputQueue.push(line);
     };
   static void queueAdd(String line)
     {
	outputQueue.push(line);
     };

   static void gotEOB(String const &server)	// End of burst from remote
     {
#ifdef DEBUG
	cout << "Received End Of Burst from " << server << endl;
#endif
	burstServers.erase(server.toLower());
	if (server == CONNECT_SERVERNAME) {
	   burstOk = true;
	}
     };
   
   static void gotSOB(String const &server)	// Start of burst from remote
     {
#ifdef DEBUG
	cout << "Received Start Of Burst from " << server << endl;
#endif
	burstServers.insert(server.toLower());
     };
   
   static bool inMainBurst(void)		// In the main connect burst?
     {
	return !burstOk;
     };
   
   static bool inBurst(String const &server)	// Is a remote in burst mode?
     {
	return (!burstOk ? true : 
		(*burstServers.find(server.toLower())).length());
     };

   static void userOn(void)			// A user signing on
     {
	countUserConnects++;
     };
   
   static void addIgnore(String const &who)	// Told to ignore someone
     {
	ignoreList.insert(who.IRCtoLower());
     };

   static void delIgnore(String const &who)	// Told to un-ignore someone
     {
	ignoreList.erase(who.IRCtoLower());
     };
   
   static bool isIgnoring(String const &who)	// Ignoring someone?
     {
	return (*ignoreList.find(who.IRCtoLower())).length();
     };
   
   static void gotVersion(String const &,
			  String const &);	// Got a version string

   static void addVersionRequest(String const &nick)// Add a version request
     {
	versionRequests[nick.IRCtoLower()] = currentTime;
     };
   
   static void gotPong(void)			// Received a server pong
     {
	sentPing = false;
     };

   static unsigned long getCountVersions(void)
     {
	return countVersions;
     };

   static unsigned long getCountVersionSpoofs(void)
     {
	return countVersionSpoofs;
     };

   static unsigned long getCountVersionsTotal(void)
     {
	return countVersionsTotal;
     };

   static unsigned long getCountUserConnects(void)
     {
	return countUserConnects;
     };
   
   static unsigned long getCountTx(void)
     {
	return countTx;
     };
   
   static unsigned long getCountRx(void)
     {
	return countRx;
     };

   static int getCountIgnores(void)
     {
	return ignoreList.size();
     };
   
   static int getUniqueVersions(void)
     {
	return versions.size();
     };

   static time_t getStartTime(void)
     {
	return startTime;
     };
   
   static time_t getUptime(void)
     {
	return currentTime - startTime;
     };
   
   static void run(void);			// Main loop
};

#endif
