/* str.cpp
 * String, StringTokens, and StringMask classes
 * 
 * Copyright (c) 1996-2001 Alien Internet Services
 * This version is somewhat outdated.
 */

#include "config.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "str.h"


// This is used in iostream reading
#define BUFFSIZE	4096	// 4k


/* String - [Various Forms] Constructors/copiers/converters
 * Original 07/96 simonb
 */
String::String(char const *s)
: ptr(new strptr),
  len(strlen(s))
{
   ptr->str = new char[len + 1];
   strcpy(ptr->str, s);
}

String::String(String const &s)
: ptr(s.ptr),
  len(s.len)
{
   ptr->ref++;
}

String::String(unsigned char i)
{
   char temp[16];
   sprintf(temp, "%d", i);
   
   ptr = new strptr;
   len = strlen(temp);
   ptr->str = new char[len + 1];
   strcpy(ptr->str, temp);
}

String::String(long i)
{
   char temp[32];
   sprintf(temp, "%ld", i);
   
   ptr = new strptr;
   len = strlen(temp);
   ptr->str = new char[len + 1];
   strcpy(ptr->str, temp);
}

String::String(unsigned long i)
{
   char temp[32];
   sprintf(temp, "%ld", i);
   
   ptr = new strptr;
   len = strlen(temp);
   ptr->str = new char[len + 1];
   strcpy(ptr->str, temp);
}

String::String(int i)
{
   char temp[32];
   sprintf(temp, "%d", i);
   
   ptr = new strptr;
   len = strlen(temp);
   ptr->str = new char[len + 1];
   strcpy(ptr->str, temp);
}

String::String(unsigned int i)
{
   char temp[32];
   sprintf(temp, "%d", i);
   
   ptr = new strptr;
   len = strlen(temp);
   ptr->str = new char[len + 1];
   strcpy(ptr->str, temp);
}

String::String(double i)
{
   char temp[128];
   sprintf(temp, "%f", i);
   
   ptr = new strptr;
   len = strlen(temp);
   ptr->str = new char[len + 1];
   strcpy(ptr->str, temp);
}

String::String(char c)
{
   ptr = new strptr;
   ptr->str = new char[2];
   ptr->str[0] = c;
   ptr->str[1] = '\0';
   len = 1;
}


/* ~String - Destructor
 * Original 07/96 simonb
 */
String::~String()
{
   // Sanity check
   if (--ptr->ref == 0) {
      delete[] ptr->str;
      delete ptr;
   }
}


/* String Operators
 * Original 07/96 simonb
 * 09/01/01 simonb - Fixed potential problems with [] operator
 */
String &String::operator=(char const *s)
{
   // Clear up old pointer, if any
   if (ptr->ref > 1) {
      ptr->ref--;
      ptr = new strptr;
   } else {
      delete[] ptr->str;
   }
   
   len = strlen(s);
   ptr->str = new char[len + 1];
   strcpy(ptr->str, s);
   
   return *this;
}

String &String::operator=(String const &s)
{
   // Protection against string = string possibility
   s.ptr->ref++;
   
   // Clear up old pointer, if any
   if (--ptr->ref == 0) {
      delete[] ptr->str;
      delete ptr;
   }

   ptr = s.ptr;
   len = s.len;

   return *this;
}

char &String::operator[](length_t i)
{
   // Range check
   if ((i < 0) || (i > len)) {
      return ptr->str[len + 1];
   }

   return ptr->str[i];
}

char const &String::operator[](length_t i) const
{
   // Range check
   if ((i < 0) || (i > len)) {
      return ptr->str[len + 1];
   }
   
   return ptr->str[i];
}

String String::operator+(char const *s) const
{
   char *temp = new char[len + strlen(s) + 1];
   
   strcpy(temp, ptr->str);
   strcat(temp, s);
   
   String result(temp);
   delete temp;
   
   return result;
}

String String::operator+(String const &s) const
{
   char *temp = new char[len + s.len + 1];
   
   strcpy(temp, ptr->str);
   strcat(temp, s.ptr->str);
   
   String result(temp);
   delete temp;
   
   return result;
}

istream &operator>>(istream &s, String &line)
{
   // Check to make sure we are not going to mess this up
   if (line.ptr->ref > 1) {
      line.ptr->ref--;
      line.ptr = new String::strptr;
   } else {
      delete[] line.ptr->str;
   }
   
   char buf[BUFFSIZE];
   char c;
   
   s.get(buf, BUFFSIZE, '\n');
   s.get(c);
   
   line.len = strlen(buf);
   line.ptr->str = new char[line.len + 1];
   strcpy(line.ptr->str, buf);
   
   return s;
}


/* find - Find a character in a string
 * Original 07/96 simonb
 */
String::length_t String::find(char c) const
{
   char *s = strchr(ptr->str, c);

   // Check that worked
   if (s) {
      return -1;
   }
   
   return (length_t)(s - ptr->str);
}


/* subString - Return a cut section of a string
 * Original 07/96 simonb
 */
String String::subString(length_t start, length_t end) const
{
   // Sanity check
   if (end < start) {
      return "";
   }
   
   char *temp = new char[end-start+2];
   
   strncpy(temp, (ptr->str + start), (end - start + 1));
   
   temp[end - start + 1] = '\0';
   String res(temp);
   delete temp;
   
   return res;
}


/* toLower - Convert an entire string to lower case
 * Original 07/96 simonb
 */
String String::toLower() const
{
   char *temp = new char[len + 1];
   
   for (register unsigned int i = (len + 1); i--;) {
      if (isupper(ptr->str[i])) {
	 temp[i] = tolower(ptr->str[i]);
      } else {
	 temp[i] = ptr->str[i];
      }
   }
   
   String res(temp);
   delete temp;
   
   return res;
}


/* toUpper - Convert an entire string to upper case
 * Original 07/96 simonb
 */
String String::toUpper() const
{
   char *temp = new char[len + 1];
   
   for (register unsigned int i = (len + 1); i--;) {
      if (islower(ptr->str[i])) {
	 temp[i] = toupper(ptr->str[i]);
      } else {
	 temp[i] = ptr->str[i];
      }
   }
   
   String res(temp);
   delete temp;

   return res;
}


/* IRCtoLower - Convert to lowercase while considering irc chars (eg {} and [])
 * Original 12/08/01 simonb
 */
String String::IRCtoLower() const
{
   char *temp = new char[len + 1];
   
   for (register unsigned int i = (len + 1); i--;) {
      switch (ptr->str[i]) {
       case '[':
	 temp[i] = '{';
	 continue;
       case ']':
	 temp[i] = '}';
	 continue;
       case '\\':
	 temp[i] = '|';
	 continue;
       case '~':
	 temp[i] = '^';
	 continue;
       default:
	 if (isupper(ptr->str[i])) {
	    temp[i] = tolower(ptr->str[i]);
	 } else {
	    temp[i] = ptr->str[i];
	 }
      }
   }
   
   String res(temp);
   delete temp;
   
   return res;
}


/* trim - Trim away spaces, tabs, and CR/LF's from the start/end of a string
 * Original 06/03/99 simonb
 */
String String::trim(void) const
{
   length_t i = 0;
   length_t j = len - 1;
   
   while ((i < j) && 
	  ((ptr->str[i] == ' ') || 
	   (ptr->str[i] == '\t') || 
	   (ptr->str[i] == '\r') ||
	   (ptr->str[i] == '\n'))) {
      i++;
   }
   
   while (j > 0 && 
	  ((ptr->str[j] == ' ') || 
	   (ptr->str[j] == '\t') ||
	   (ptr->str[j] == '\r') ||
	   (ptr->str[j] == '\n'))) {
      j--;
   }
   
   return subString(i, j);
}


/* trimQuotes - Same as regular trim(), but for removing ' and "'s
 * Original 11/08/01 simonb
 */
String String::trimQuotes(void) const
{
   length_t i = 0;
   length_t j = len - 1;
   
   while ((i < j) && 
	  ((ptr->str[i] == ' ') ||
	   (ptr->str[i] == '\'') || 
	   (ptr->str[i] == '"'))) {
      i++;
   }
   
   while (j > 0 && 
	  ((ptr->str[j] == ' ') ||
	   (ptr->str[j] == '\'') ||
	   (ptr->str[j] == '"'))) {
      j--;
   }
   
   return subString(i, j);
}


/* pad - Make string exactly n by cropping or adding spaces to the end
 * Original 06/03/99 simonb
 */
String String::pad(length_t n, char c) const
{
   length_t l = len;

   if (n <= l) {
      return subString(0, n-1);
   }
   
   char *temp = new char[n+1];
   strcpy(temp, ptr->str);
   
   for (register length_t i = l; i < n; i++) {
      temp[i] = ' ';
   }
   
   temp[n] = '\0';

   String res(temp);
   delete temp;

   return res;
}


/* prepad - Make string exactly n by cropping or adding spaces to the start
 * Original 06/03/99 simonb
 * Note: this really could be more efficient :-/
 */
String String::prepad(length_t n, char c) const
{
   char *temp = new char[n+1];
   strcpy(temp, ptr->str);
   String out(temp);
   delete temp;

   while (out.len < n) {
      out = String((char)c) + out;
   }

   return out;
}


/* printf - Essentially printf with String class return
 * Original 09/06/00 simonb
 * Note: Doesn't work that well, eg. with String class vars.
 *       AFAIK there isn't a way to define 'class String' as being passable
 *       though '...' -- the compiler is just stupid since we DO have a 
 *       (char const *)() overload operator defined :(
 */
String String::printf(char *format, ...)
{
   static char buff[10000];
   va_list ap;
   
   va_start(ap, format);
   vsprintf(buff, format, ap);
   va_end(ap);
   
   buff[9999] = '\0'; // safety!
   return String(buff);
}


/* hasMoreTokens - Check if the string has more tokens
 * Original 08/02/99 simonb
 */
bool StringTokens::hasMoreTokens(void) const
{
   if (pos == str.length()) {
      return false;
   }
   
   for (register String::length_t i = pos; i < str.length(); i++) {
      if ((str[i] != ' ') && 
	  (str[i] != '\t')) {
	 return true;
      }
   }
   
   return false;
}

bool StringTokens::hasMoreTokens(char c) const
{
   if (pos == str.length()) {
      return false;
   }
   
   for (register String::length_t i = pos; i < str.length(); i++) {
      if (str[i] != c) {
	 return true;
      }
   }
   
   return false;
}


/* countTokens - [Various Forms] Count the number of tokens
 * Original 08/02/99 simonb
 */
unsigned int StringTokens::countTokens(void)
{
   unsigned int i = 0;
   StringTokens s(str);
   
   while (s.hasMoreTokens()) {
      s.nextToken();
      i++;
   }
   
   return i;
}

unsigned int StringTokens::countTokens(char c)
{
   unsigned int i = 0;
   StringTokens s(str);
   
   while (s.hasMoreTokens(c)) {
      s.nextToken(c);
      i++;
   }
   
   return i;
}


/* nextToken - [Various Forms] Grab the next token
 * Original 08/02/99 simonb
 * Note: printf bug?
 */
String StringTokens::nextToken(void)
{
   String::length_t i = pos;
   
   while ((i < str.length()) &&
	  ((str[i] == ' ') || (str[i] == '\t'))) {
      i++;
   }
   
   for (String::length_t j = i; j < str.length(); j++) {
      if ((str[j] == ' ') || (str[j] == '\t')) {
	 pos = j + 1;
	 return str.subString(i, j - 1);
      }
   }
   
   pos = str.length();
   return str.subString(i, str.length() - 1);
}

String StringTokens::nextToken(char c, bool empty)
{
   String::length_t i = pos;
   
   while ((i < str.length()) && (str[i] == c)) {
      i++;
   }
   
   for (String::length_t j = i; j < str.length(); j++) {
      if (str[j] == c) {
	 pos = j + 1;
	 return str.subString(i, j - 1);
      }
   }
   
   if (empty) {
      return "";
   }
   
   pos = str.length();
   return str.subString(i, str.length() - 1);
}


/* nextColonToken - Get the next token, or the rest, depending on a :
 * Original 11/08/01 simonb
 */
String StringTokens::nextColonToken(void)
{
   if (pos == str.length()) {
      return "";
   }
   
   if (str[pos] == ':') {
      pos++;
      return rest();
   }
   
   return nextToken();
}


/* rest - Grab the rest of the string ignoring the remaining tokens
 * Original 08/02/99 simonb
 */
String StringTokens::rest(void)
{
   if (pos == str.length()) {
      return "";
   }
   
   while ((pos < str.length()) &&
	  (str[pos] == ' ' || str[pos] == '\t')) {
      pos++;
   }
   
   return str.subString(pos, str.length() - 1);
}


/* match - Do the actual match check
 * Original 15/02/96 simonb
 * 18/07/98 simonb - Ported from pascal to C++
 * 20/07/98 simonb - Changed String to char const
 * 07/09/01 simonb - Partial rewrite for more speed (no damned recursion!!)
 */
bool StringMask::match(char const *m, char const *n)
{
   bool escaped = false;
   
   // sanity check for strings, the neat way returning true/false
   if (!*m) {
      // Of course if they are both null then this is a match
      if (!*n) {
	 return true;
      } else {
	 return false;
      }
   } else {
      if (!*n) {
	 return false;
      }
   }
   
   // While the string is valid, run through it
   while (*m) {
      // Check if this is being escaped
      if (escaped) {
	 // Next character for both strings
	 m++;
	 n++;
	 
	 escaped = false;
      } else {
	 // Check what to do with this character
	 switch (*m) {
	  case '*': // Anything goes metacharacter
	    // Loop until the next character is NOT the astericks metacharacter
	    while (*m && (*m == '*')) {
	       m++;
	    }
	    
	    // Loop until the given string ends or we find a matching char
	    while (*n && (*n != *m)) {
	       n++;
	    }
	    
	    /* If the next char is the null, bail out now and say true since the
	     * rest of the line will of course match
	     */
	    if (!*m && !*n) {
	       return true;
	    }
	    
	    continue;
	  case '?': // Skip one char metacharacter
	    // If the string is broken already, this cannot count as a char.
	    if (!*n) {
	       return false;
	    }
	    
	    m++;
	    n++;
	    
	    // If both the next chars are nulls, this obviously worked OK
	    if (!*m && !*n) {
	       return true;
	    }

	    continue;
	  case '\\': // Escape character
	    escaped = true;
	  default:
	    // Check if these two characters match in both strings
	    if (*m != *n) {
	       return false;
	    }
	    
	    // Next character for both strings
	    m++;
	    n++;
	    
	    // If both of these are at the end of line, naturally they are OK!
	    if (!*m && !*n) {
	       return true;
	    }
	 }
      }
   }
   
   return false;
}


#ifdef STL_HAS_HASH
/* hash<String> - generic hash template for our String class
 * Original 24/08/01 simonb
 */
size_t hash<String>::operator()(String const &str) const
{
   char const *s = (char const *)str;
   size_t size = 0; // Should we offset this?
   
   /* Run through the string; Note the last number should be a prime for
    * minimising collisions. Everywhere I read people seem to like magic
    * number 17 as a generic prime, so I have followed the fashion :)
    */
   for (; *s; s++) {
      size = (size * 17) ^ *s;
   }
   
   return size;
}
#endif


