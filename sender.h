/* sender.h
 * Command compilation and sending routines
 * Copyright (c) 2002 Alien Internet Services
 */

#ifndef __SENDER_H_
# define __SENDER_H_

# include <slist>

# include "config.h"
# include "daemon.h"
# include "str.h"

class Sender {
 private:
   typedef slist <String> textbuff_t;

   static textbuff_t motdData;
   static textbuff_t helpData;
   
   Sender(void) {};
   ~Sender(void) {};
   
 public:
   static void init(void);
   
   static void sendBurst(void);

   static void sendCTCPpingReply(String const &who, String &data)
     {
	Daemon::queueAdd(String(":" MY_USERNICK " NO ") + who +
			 " :\001PING " + data + "\001");
     };
   
   static void sendCTCPversion(String const &who)
     {
	Daemon::queueAdd(String(":" MY_USERNICK " P ") + who +
			 " :\001VERSION\001");
     };
   
   static void sendCTCPversionReply(String const &who)
     {
	Daemon::queueAdd(String(":" MY_USERNICK " P ") + who +
			 " :\001VERSION " VERSION "\001");
     };

   static void sendMOTDreply(String const &);
   
   static void sendNOTICE(String const &who, String &data)
     {
	Daemon::queueAdd(String(":" MY_USERNICK " NO ") + who + " :" + data);
     };
   
   static void sendPING()
     {
	Daemon::queueAdd(":" MY_SERVERNAME " PING :Hello?");
     };
   
   static void sendPONG(String &data)
     {
	Daemon::queueAdd(String(":" MY_SERVERNAME " PONG ") + data);
     };

   static void sendSQUIT(String const &reason)
     {
	Daemon::queueAdd(String(":" MY_SERVERNAME " SQUIT " MY_SERVERNAME 
				"0 :") + reason);
     };
   
   static void sendVERSIONreply(String const &who)
     {
	Daemon::queueAdd(String(":" MY_SERVERNAME " 351 ") + who +
			 " :" VERSION);
     };

   static void sendWHOISreply(String const &);
   static void sendHelpReply(String const &);
   static void sendStatsReply(String const &);
   
# ifdef ALLOW_RAW_COMMAND
   static void Sender::sendRaw(String const &data)
     {
	Daemon::queueAdd(data);
     };
# endif
};

#endif
