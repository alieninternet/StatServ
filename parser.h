/* parser.cpp
 * Parsing routines
 * Copyright (c) 2002 Alien Internet Services
 */

#ifndef __PARSER_H_
# define __PARSER_H_

# include "config.h"

# include "daemon.h"

# define PARSER_FUNC(x)		x(String &origin, StringTokens &tokens)

class Parser {
 private:
   struct functionTableStruct {
      char const *command;
      void PARSER_FUNC((*function));
   };
   static struct functionTableStruct const functionTable[];
   
   Parser(void)
     {};
   ~Parser(void)
     {};
   
 public:
   static void parseLine(String &);
   
 private:
   static void PARSER_FUNC(parseEOB);
   static void PARSER_FUNC(parseMOTD);
   static void PARSER_FUNC(parseNICK);
   static void PARSER_FUNC(parseNOTICE);
   static void PARSER_FUNC(parsePING);
   static void PARSER_FUNC(parsePRIVMSG);
   static void PARSER_FUNC(parseQUIT);
   static void PARSER_FUNC(parseSERVER);
   static void PARSER_FUNC(parseVERSION);
};

#endif
