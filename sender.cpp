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
};


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
};


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
};

