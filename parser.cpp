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
     { "MOTD",			parseMOTD },
     { "N",			parseNICK },
     { "NO",			parseNOTICE },
     { "PING",			parsePING },
     { "PONG",			parsePONG },
     { "P",			parsePRIVMSG },
     { "S",			parseSERVER },
     { "VERSION",		parseVERSION },
     { "WHOIS",			parseWHOIS },
     { 0 }
};

/* parseLine - Parse a line of protocol data, and send it somewhere if needs be
 * Original 18/02/2002 simonb
 */
void Parser::parseLine(String &line)
{
#ifdef DEBUG_PROTOCOL
   cerr << line << endl;
#endif
   
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
   Daemon::gotEOB(origin);
}


/* parseMOTD - Parse an MOTD request
 * Original 19/02/2002 simonb
 */
void PARSER_FUNC(Parser::parseMOTD)
{
   Sender::sendMOTDreply(origin);
};


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
   
   // If we are still in bursting mode, do not flood the network..
   if (!Daemon::inBurst(origin)) {
      // Ok, grab the nickname
      String nickname = tokens.nextToken();
      
      // Check if we should ignore this nickname
      if (Daemon::isIgnoring(nickname)) {
	 return;
      }
	  
      // Send out a request for that nickname
      Daemon::addVersionRequest(nickname);
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
	 Daemon::gotVersion(origin, CTCPtokens.rest());
	 return;
#ifdef DEBUG
      } else {
	 cout << "CTCP(NO) " << origin << '>' << message << endl;
#endif
      }
   }

   // We ignore other notices, they serve no other purpose here.
}


/* parsePING - Handle the remote server pinging us
 * Original 18/02/2002 simonb
 */
void PARSER_FUNC(Parser::parsePING)
{
   // Just reply.. no fuss! P13 doesn't really let other servers ping..
   String data = tokens.rest();
   Sender::sendPONG(data);
}
 

/* parsePONG - Handle the remote server replying to our ping
 * Original 19/02/2002 simonb
 */
void PARSER_FUNC(Parser::parsePONG)
{
   // Pretty complex hey? :)
   Daemon::gotPong();
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
      } else if (CTCPcommand == "VERSION") {
	 Sender::sendCTCPversionReply(origin);
	 return;
#ifdef DEBUG
      } else {
	 cout << "CTCP(P) " << origin << '>' << message << endl;
#endif
      }
      return;
   }
   
   // An ordinary message.. break it up and grab the first word
   StringTokens commandLine(message);
   String command = commandLine.nextToken().toUpper();

   String reply = "";

   // We only know one command, maybe this is it?
   if (command == "HELP") {
      Sender::sendHelpReply(origin);
      return;
#ifdef IGNORE_ALLOWED
   } else if (command == "IGNORE") {
      if (!Daemon::isIgnoring(origin)) {
	 reply = MSG_IGNORE;
	 Daemon::addIgnore(origin);
      } else {
	 reply = MSG_IGNORE_ON;
      }
      Sender::sendNOTICE(origin, reply);
      return;
   } else if (command == "UNIGNORE") {
      if (Daemon::isIgnoring(origin)) {
	 reply = MSG_UNIGNORE;
	 Daemon::delIgnore(origin);
      } else {
	 reply = MSG_UNIGNORE_ON;
      }
      Sender::sendNOTICE(origin, reply);
      return;
#endif
#ifdef STATS_COMMAND
   } else if (command == "STATS") {
# ifdef STATS_PASSPHRASE
      // Check if the pass-phrase is valid
      if (commandLine.rest() == STATS_PASSPHRASE) {
# endif
	 Sender::sendStatsReply(origin);
	 return;
# ifdef STATS_PASSPHRASE
      }
# endif
#endif
#ifdef DEBUG
# ifdef ALLOW_RAW_COMMAND
   } else if (command == "RAW") {
      String data = commandLine.rest();
      cout << "RAW " << origin << '>' << data << endl;
      Sender::sendRaw(data);
      return;
# endif
#endif
   }
   
#ifndef ANNOYING_SILENT_TREATMENT
   reply = MSG_UNKNOWN;
   Sender::sendNOTICE(origin, reply);
#endif
}

/* parseSERVER - Parse a server link 
 * Original 18/02/2002 simonb
 */
void PARSER_FUNC(Parser::parseSERVER)
{
   if (!Daemon::inMainBurst() && !Daemon::inBurst(origin)) {
      // Add this server to the start-of-burst list
      String serverName = tokens.nextToken();
      Daemon::gotSOB(serverName);
   }
};


/* parseVERSION - Parse a server link 
 * Original 19/02/2002 simonb
 */
void PARSER_FUNC(Parser::parseVERSION)
{
   Sender::sendVERSIONreply(origin);
};


/* parseWHOIS - Do a remote WHOIS for those people with not enough to do...
 * Original 19/02/2002 simonb
 */
void PARSER_FUNC(Parser::parseWHOIS)
{
   // Grab the username and check if that message is really destined for us
   if (tokens.nextToken().toLower() != String(MY_USERNICK).toLower()) {
      return;
   }

   Sender::sendWHOISreply(origin);
};
