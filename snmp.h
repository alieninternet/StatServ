/* snmp.h
 * Interface to NET-SNMP to allow for SNMP connectivity to this server
 * Copyright (c) 2002 Alien Internet Services
 */

#ifndef __SNMP_H_
# define __SNMP_H_

#include <ucd-snmp/ucd-snmp-config.h>
#include <netinet/in.h>
#include <ucd-snmp/ucd-snmp-includes.h>
#include <ucd-snmp/ucd-snmp-agent-includes.h>

config_require(util_funcs)

class SNMP {
 private:
   SNMP(void) {};
   ~SNMP(void) {};
   
 public:
   static void init(void);			// Initialise the SNMP agent
   static void deinit(void);			// De-init the agent
   
   static void slice(void)			// Check SNMP slice
     {
	// Check SNMP for activity and process requests (0 = Non-blocking)
	agent_check_and_process(0);
     }
};
   
#endif
