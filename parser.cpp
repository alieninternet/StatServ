/* parser.cpp
 * Parsing routines
 * Copyright (c) 2002 Alien Internet Services
 */

#include "config.h"

#ifdef DEBUG
# include <iostream>
#endif

#include "parser.h"
#include "sender.h"

// Function table (list of commands)
struct Parser::functionTableStruct const Parser::functionTable[] = {
     { "END_OF_BURST",		parseEOB },
     { "N",			parseNICK },
     { "NO",			parseNOTICE },
     { "PING",			parsePING },
     { "P",			parsePRIVMSG },
     { "Q",			parseQUIT },
     { 0 }
};

/* parseLine - Parse a line of protocol data, and send it somewhere if needs be
 * Original 18/02/2002 simonb
 */
void Parser::parseLine(String &line)
{
   // Break the line apart using the string tokeniser
   StringTokens st(line);
   String origin = "";
   
   // Does this line start with a ':' (server form)?
   if (line[0] == ':') {
      // Skip the first parameter, we do not care about it anyway
      origin = st.nextToken().subString(1);
   }
   
   // Rip out the command (presume it is in the right case)
   String command = st.nextToken();
   
   // Look this command up in the command table...
   for (int i = 0; functionTable[i].command != 0; i++) {
      // Does this match?
      if (command == functionTable[i].command) {
	 // Run the command and leave
	 functionTable[i].function(origin, st);
	 return;
      }
   }
}


/* parseEOB - Handle an END_OF_BURST command
 * Original 18/02/2002 simonb
 */
void PARSER_FUNC(Parser::parseEOB)
{
#ifdef DEBUG
   cout << "Received End Of Burst line from " << origin << endl;
#endif
   Daemon::gotEOB();
}


/* parseNICK - Handle an new user signing on (or a nick change eek!)
 * Original 18/02/2002 simonb
 */
void PARSER_FUNC(Parser::parseNICK)
{
   /* Check the number of tokens, this will ensure we only look at new users
    * and not nick changes..
    */
   if (tokens.countTokens() < 13) {
      return;
   }
   
   Daemon::userOn();
   
   // If we are still in bursting mode, do not flood the network..
   if (!Daemon::inBurst()) {
      // Ok, grab the nickname
      String nickname = tokens.nextToken();
      
      // Check if we should ignore this nickname here??

      // Send out a request for that nickname
      Sender::sendCTCPversion(nickname);
   }
}


/* parseNOTICE - Handle an NOTICE message
 * Original 18/02/2002 simonb
 */
void PARSER_FUNC(Parser::parseNOTICE)
{
   // Grab the username and check if that message is really destined for us
   if (tokens.nextToken().toLower() != String(MY_USERNICK).toLower()) {
      return;
   }
   
   // Grab the message
   String message = tokens.nextColonToken();
   
   // Check if the message is a CTCP
   if ((message[0] == '\001') && 
       (message[message.length() - 1] == '\001')) {
      // Break the message down
      StringTokens CTCPtokens(message.subString(1, message.length() - 2));
      String CTCPcommand = CTCPtokens.nextToken().toUpper();
      
      // Check if this a version reply (most likely)
      if (CTCPcommand == "VERSION") {
	 String data = CTCPtokens.rest();
#ifdef DEBUG
	 cout << "Version Reply: " << data << endl;
#endif
	 Daemon::gotVersion(data);
	 return;
#ifdef DEBUG
      } else {
	 cout << "CTCP(NO) " << origin << '>' << message << endl;
#endif
      }
   }

   // We ignore other notices, they serve no other purpose here.
}


/* parsePING - Handle an PING message
 * Original 18/02/2002 simonb
 */
void PARSER_FUNC(Parser::parsePING)
{
   // Just reply.. no fuss!
   String data = tokens.rest();
   Sender::sendPONG(data);
}
 

/* parsePRIVMSG - Handle an PRIVMSG message
 * Original 18/02/2002 simonb
 */
void PARSER_FUNC(Parser::parsePRIVMSG)
{
   // Grab the username and check if that message is really destined for us
   if (tokens.nextToken().toLower() != String(MY_USERNICK).toLower()) {
      return;
   }
   
   // Grab the message
   String message = tokens.nextColonToken();

   // Check if the message is a CTCP
   if ((message[0] == '\001') && 
       (message[message.length() - 1] == '\001')) {
      // Break the message down
      StringTokens CTCPtokens(message.subString(1, message.length() - 2));
      String CTCPcommand = CTCPtokens.nextToken().toUpper();
      
      // Check what kind of CTCP this is
      if (CTCPcommand == "PING") {
	 String data = CTCPtokens.rest();
	 Sender::sendCTCPpingReply(origin, data);
	 return;
#ifdef DEBUG
      } else {
	 cout << "CTCP(P) " << origin << '>' << message << endl;
#endif
      }
      return;
   }
   
   // An ordinary message..
#ifdef DEBUG
   cout << "P " << origin << '>' << message << endl;
#endif
}


/* parseQUIT - Parse a user signoff message
 * Original 18/02/2002 simonb
 */
void PARSER_FUNC(Parser::parseQUIT)
{
   Daemon::userOff();
};
