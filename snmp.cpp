/* snmp.h
 * Interface to NET-SNMP to allow for SNMP connectivity to this server
 * Copyright (c) 2002 Alien Internet Services
 */

#include "config.h"

#include <string.h>

#include "snmp.h"
#include "daemon.h"

   
// Our entry level object identifier (see the MIB file)
static oid statservOID[] = { 1,3,6,1,4,1,4379,3,3 };


// These are the oid magic numbers for our variable table and callback function
enum var_oids {
   VAR_UPTIME = 1,
   VAR_COUNTTX = 2,
   VAR_COUNTRX = 3,
   VAR_COUNTUSERS = 4,
   VAR_COUNTUSERCONNECTS = 5,
   VAR_COUNTUSERDISCONNECTS = 6,
   VAR_COUNTSERVERS = 7,
   VAR_COUNTSERVERCONNECTS = 8,
   VAR_COUNTSERVERDISCONNECTS = 9,
   VAR_COUNTVERSIONREPLIES = 10,
   VAR_COUNTVERSIONREPLIESTOTAL = 11,
   VAR_COUNTVERSIONUNIQUEDB = 12
};


/* variableCallback - Return the appropriate value as requested to do so
 * Original 20/02/2002 simonb
 */
unsigned char *variableCallback(struct variable *vp, oid *name, size_t *length,
				int exact, size_t *var_len, 
				WriteMethod **write_method)
{
#ifdef DEBUG
   cout << "SNMP callback for magic #" << (int)vp->magic << endl;
#endif
   
   // The variable we will be returning, hopefully
   static long ret; // unsigned long?
   
   // Work out which variable we are supposed to return exactly...
   switch(vp->magic) {
    case VAR_UPTIME: 
      ret = Daemon::getUptime();
      return (unsigned char *)&ret;
    case VAR_COUNTTX:
      ret = Daemon::getCountTx();
      return (unsigned char *)&ret;
    case VAR_COUNTRX:
      ret = Daemon::getCountRx();
      return (unsigned char *)&ret;
    case VAR_COUNTUSERS: // !
      ret = Daemon::getCountUsers();
      return (unsigned char *)&ret;
    case VAR_COUNTUSERCONNECTS:
      ret = Daemon::getCountUserConnects();
      return (unsigned char *)&ret;
    case VAR_COUNTUSERDISCONNECTS:
      ret = Daemon::getCountUserDisconnects();
      return (unsigned char *)&ret;
    case VAR_COUNTSERVERS: // !
      ret = Daemon::getCountServers();
      return (unsigned char *)&ret;
    case VAR_COUNTSERVERCONNECTS:
      ret = Daemon::getCountServerConnects();
      return (unsigned char *)&ret;
    case VAR_COUNTSERVERDISCONNECTS:
      ret = Daemon::getCountServerDisconnects();
      return (unsigned char *)&ret;
    case VAR_COUNTVERSIONREPLIES:
      ret = Daemon::getCountVersions();
      return (unsigned char *)&ret;
    case VAR_COUNTVERSIONREPLIESTOTAL:
      ret = Daemon::getCountVersionsTotal();
      return (unsigned char *)&ret;
    case VAR_COUNTVERSIONUNIQUEDB:
      ret = Daemon::getUniqueVersions();
      return (unsigned char *)&ret;
    default:
      ERROR_MSG("");
   }
   
   // Nothing to return..
   return SNMP_ERR_NOERROR;
}


// Function table - this is to tell NET-SNMP how we return our variables
static struct variable2 variablesTable[] = {
   /* Format: 
    *   magic number,			variable type,	access,
    *     handler function,			oid suffix length,
    *     oidsuffix 
    */
     {
	VAR_UPTIME,				ASN_TIMETICKS,	RONLY,	
	  variableCallback,			1,
	  { 1 }
     },
     { 
	VAR_COUNTTX,				ASN_COUNTER,	RONLY,	
	  variableCallback, 			1,
	  { 2 }
     },
     {
	VAR_COUNTRX,				ASN_COUNTER,	RONLY,	
	  variableCallback,			1,
	  { 3 }
     },
     { 
	VAR_COUNTUSERS,				ASN_GAUGE,	RONLY,	
	  variableCallback, 			1,
	  { 4 }
     },
     {
	VAR_COUNTUSERCONNECTS,			ASN_COUNTER,	RONLY,
	  variableCallback, 			1,
	  { 5 }
     },
     { 
	VAR_COUNTUSERDISCONNECTS,		ASN_COUNTER,	RONLY,
	  variableCallback, 			1,
	  { 6 }
     },
     { 
	VAR_COUNTSERVERS,			ASN_GAUGE,	RONLY,
	  variableCallback, 			1,
	  { 7 }
     },
     { 
	VAR_COUNTSERVERCONNECTS,		ASN_COUNTER,	RONLY,
	  variableCallback, 			1,
	  { 8 }
     },
     { 
	VAR_COUNTSERVERDISCONNECTS,		ASN_COUNTER,	RONLY,
	  variableCallback, 			1,
	  { 9 }
     },
     { 
	VAR_COUNTVERSIONREPLIES,		ASN_COUNTER,	RONLY,
	  variableCallback, 			1,
	  { 10 }
     },
     { 
	VAR_COUNTVERSIONREPLIESTOTAL,		ASN_COUNTER,	RONLY,
	  variableCallback, 			1,
	  { 11 }
     },
     { 
	VAR_COUNTVERSIONUNIQUEDB,		ASN_COUNTER,	RONLY,
	  variableCallback, 			1,
	  { 12 }
     }
};


/* init - Initialise the SNMP module
 * Original 20/02/2002 simonb
 */
void SNMP::init(void) {
#ifdef DEBUG
   cout << "Initialising SNMP agent..." << endl;

# ifndef DEBUG_PROTOCOL
   // Log errors to stderr so we can see what is happening
   snmp_enable_stderrlog();
# endif
#endif

#ifdef SNMP_AS_AGENTX
   // Set ourselves up as an AgentX sub-agent
   ds_set_boolean(DS_APPLICATION_ID, DS_AGENT_ROLE, 1);
#endif
   
   // Initialise the agent..
   init_agent(SNMP_AGENT_NAME);
   
   // Tell the agent to use this bit of the MIB tree
   REGISTER_MIB(SNMP_AGENT_NAME, variablesTable, variable2,
		statservOID);
   
   // Initialise SNMP (it will read statserv.conf files)
   init_snmp(SNMP_AGENT_NAME);
   
#ifndef SNMP_AS_AGENTX
   // Fire up the master agent core
   init_master_agent(SNMP_MASTER_AGENT_PORT, NULL, NULL);
#endif
}


/* deinit - Shutdown the SNMP agent
 * Original 20/02/2002 simonb
 */
void SNMP::deinit(void) {
   snmp_shutdown(SNMP_AGENT_NAME);
}

