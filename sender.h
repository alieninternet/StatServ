/* sender.h
 * Command compilation and sending routines
 * Copyright (c) 2002 Alien Internet Services
 */

#ifndef __SENDER_H_
# define __SENDER_H_

# include "str.h"

class Sender {
 private:
   Sender(void)
     {};
   ~Sender(void)
     {};
   
 public:
   static void sendBurst(void);
   static void sendCTCPpingReply(String &, String &);
   static void sendCTCPversion(String &);
   static void sendPING(String &);
   static void sendPONG(String &);
   static void sendWALLOPS(String &);
};

#endif
