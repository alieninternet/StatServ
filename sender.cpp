/* sender.cpp
 * Command compilation and sending routines
 * Copyright (c) 2002 Alien Internet Services
 */

# include "config.h"

# include <fstream.h>
# include <list>

# include "sender.h"


// Variables, wonderous variables...
namespace Sender {
   Sender::textbuff_t motdData;
   Sender::textbuff_t helpData;
};


/* init - Load various files we need for certain output (eg. help/motd)
 * Original 19/02/2002 simonb
 */
void Sender::init(void)
{
   ifstream file;
   String line = "";
   
   /* This is our list. Single linked lists are 'filled' backwards, so we
    * need to make a backwards list then make it fill again into the real
    * list .. backwards again :)
    */
   Sender::textbuff_t inputList;
   
   // Clear the lists
   motdData.clear();
   helpData.clear();
   
   // Open our MOTD file
   file.open(FILE_MOTD);
   
   // Check
   if (!file) {
      cout << "Could not open " FILE_MOTD " for reading" << endl;
   } else {
      /* Run through the file and read the lines.. We do it this way
       * to avoid that ugly last line :)
       */
      for (;;) {
	 file >> line;
	 
	 if (!file.eof()) {
	    inputList.push_front(line);
	 } else {
	    break;
	 }
      }
      
      // Copy the input list to the real motdData list backwards to fix it
      for (Sender::textbuff_t::iterator it = inputList.begin();
	   it != inputList.end(); it++) {
	 motdData.push_front(*it);
      }
   }
   file.close();
   
   // Reset the inputList
   inputList.clear();
   
   // Open our HELP file
   file.open(FILE_HELP);
   
   // Check
   if (!file) {
      cout << "Could not open " FILE_HELP " for reading" << endl;
   } else {
      // Run through the file and read the lines
      /* Run through the file and read the lines.. We do it this way
       * to avoid that ugly last line :)
       */
      for (;;) {
	 file >> line;
	 
	 if (!file.eof()) {
	    /* If the line is 'blank', we have to add a space to make it send
	     * properly
	     */
	    if (!line.length()) {
	       line = " ";
	    }
	    
	    inputList.push_front(line);
	 } else {
	    break;
	 }
      }

      // Copy the input list to the real helpData list backwards to fix it
      for (Sender::textbuff_t::iterator it = inputList.begin();
	   it != inputList.end(); it++) {
	 helpData.push_front(*it);
      }
   }
   file.close();
}


/* sendBurst - Send out a connection burst
 * Original 18/02/2002 simonb
 * Note: This function is ugly - damned #ifdefs :(
 */
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
# ifdef NICKOP_NAME
   Daemon::queueAdd(":" MY_USERNAME " P " NICKOP_NAME " :identify " 
		    NICKOP_PASSWD);
# endif
}


/* sendMOTDreply - Send out a reply to an MOTD request
 * Original 19/02/2002 simonb
 */
void Sender::sendMOTDreply(String const &who)
{
   // Send the header
   Daemon::queueAdd(String(":" MY_SERVERNAME " 375 ") + who +
		    " :- " MY_SERVERNAME " Server Message of the Day.");

   // Loop and send the MOTD data
   for (Sender::textbuff_t::iterator it = motdData.begin(); 
	it != motdData.end(); it++) {
      Daemon::queueAdd(String(":" MY_SERVERNAME " 372 ") + who + " :- " +
		       *it);
   }

   // Send the footer
   Daemon::queueAdd(String(":" MY_SERVERNAME " 376 ") + who +
		    " :End of MOTD");
}


/* sendHelpReply - Send out a reply to HELP
 * Original 19/02/2002 simonb
 */
void Sender::sendHelpReply(String const &who)
{
   // Loop and send the help data
   for (Sender::textbuff_t::iterator it = helpData.begin(); 
	it != helpData.end(); it++) {
      sendNOTICE(who, *it);
   }
}


/* sendStatsReply - Send out a reply to a stats command request
 * Original 19/02/2002 simonb
 */
void Sender::sendStatsReply(String const &who)
{
   time_t uptime = Daemon::getUptime();
   Daemon::queueAdd(String(":" MY_USERNICK " NO ") + who + 
		    " :                   Server up-time - " +
		    (((long)(uptime / 86400) == 0) ? String("") :
		     (String((long)(uptime / 86400)) + " day" +
		      (((long)(uptime / 86400) >= 2) ? String("s") : String("")) + 
		      ", ")) +
		    ((((long)(uptime % 86400) / 3600) == 0) ? String("") :
		     (String((long)(uptime % 86400) / 3600) + " hour" + 
		      ((((long)(uptime % 86400) / 3600) >= 2) ? String("s") : String("")) + 
		      ", ")) +
		    (((long)((uptime % 3600) / 60) == 0) ? String("") :
		     (String((long)((uptime % 3600) / 60)) + " min" + 
		      (((long)((uptime % 3600) / 60) >= 2) ? String("s") : String("")) + 
		      ", ")) +
		    (String((long)(uptime % 60)) + " sec" + 
		     (((long)(uptime % 60) == 1) ? String("") : String("s"))));
   Daemon::queueAdd(String(":" MY_USERNICK " NO ") + who + 
		    " :       Data transfer this session - Rx: " +
		    String(Daemon::getCountRx() / 1024) + "k, Tx: " +
		    String(Daemon::getCountTx() / 1024) + "k");
   Daemon::queueAdd(String(":" MY_USERNICK " NO ") + who + 
		    " :                    Users online - " +
		    String(Daemon::getCountUsers()));
   Daemon::queueAdd(String(":" MY_USERNICK " NO ") + who + 
		    " :                  Servers online - " +
		    String(Daemon::getCountServers()));
   Daemon::queueAdd(String(":" MY_USERNICK " NO ") + who + 
		    " :   User connections this session - " +
		    String(Daemon::getCountConnects()));
   Daemon::queueAdd(String(":" MY_USERNICK " NO ") + who + 
		    " :User disconnections this session - " +
		    String(Daemon::getCountDisconnects()));
   Daemon::queueAdd(String(":" MY_USERNICK " NO ") + who + 
		    " :               Ignored nicknames - " +
		    String(Daemon::getCountIgnores()));
   Daemon::queueAdd(String(":" MY_USERNICK " NO ") + who +
		    " :            CTCP VERSION Replies - " +
		    String(Daemon::getCountVersions()) + " (" +
		    String(Daemon::getUniqueVersions()) + " unique)");
}
