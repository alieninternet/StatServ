/* sender.cpp
 * Command compilation and sending routines
 * Copyright (c) 2002 Alien Internet Services
 */

# include "config.h"
# include "sender.h"
# include "daemon.h"

void Sender::sendBurst(void)
{
   Daemon::queueAdd(":" MY_SERVERNAME " N " MY_USERNICK " 1 0 "
		    MY_USERNAME " " MY_USERHOST " " MY_USERVWHOST " " 
		    MY_SERVERNAME " " MY_USERMODES " 0 0 :" MY_USERDESC);
# ifdef BURST_CLONE_TEST
   for (int i = 0; i != BURST_CLONE_TEST; i++) {
      Daemon::queueAdd(String(":" MY_SERVERNAME " N " 
			      BURST_CLONE_PREFIX) + String(i) +
		       " 1 0 " MY_USERNAME " " MY_USERHOST " " 
		       MY_USERVWHOST " " MY_SERVERNAME " "
		       BURST_CLONE_MODES " 0 0 :" MY_USERDESC);
#  ifdef BURST_CLONE_CHAN_PREFIX
      Daemon::queueAdd(String(":" BURST_CLONE_PREFIX) + String(i) + 
		       " J " BURST_CLONE_CHAN_PREFIX + String(i) + 
		       " 0");
#  endif
#  ifdef BURST_CLONE_FAT_CHAN
      Daemon::queueAdd(String(":" BURST_CLONE_PREFIX) + String(i) + 
		       " J " BURST_CLONE_FAT_CHAN + " 0");
#  endif
   }
# endif
   Daemon::queueAdd("END_OF_BURST");
};

void Sender::sendCTCPpingReply(String &who, String &data)
{
   Daemon::queueAdd(String(":" MY_USERNICK " NO ") + who +
		    " :\001PING " + data + "\001");
};
   
void Sender::sendCTCPversion(String &who)
{
   Daemon::queueAdd(String(":" MY_USERNICK " P ") + who +
		    " :\001VERSION\001");
};
   
void Sender::sendPING(String &data)
{
   Daemon::queueAdd(String(":" MY_SERVERNAME " PONG ") + data);
};

void Sender::sendPONG(String &data)
{
   Daemon::queueAdd(String(":" MY_SERVERNAME " PONG ") + data);
};
   
void Sender::sendWALLOPS(String &data)
{
   Daemon::queueAdd(String(":" MY_USERNICK " WALLOPS :") + data);
};
