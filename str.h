/* str.h
 * String, StringTokens, and StringMask classes 
 * 
 * Copyright (c) 1996-2001 Alien Internet Services
 */

#ifndef __STR_H_
# define __STR_H_

# include <iostream.h>
#include <stdlib.h>

# ifdef STL_HAS_HASH
#  include <hash_map>
# endif


class String {
 public:
   typedef signed int length_t;			// Our length type

 private:
   struct strptr {				// To stop reference errors
      char *str;				// Pointer to the data
      int ref;					// Reference counter
      strptr(void):ref(1) {};			// Initialiser
   };
   strptr *ptr;					// Pointer to above
   length_t len;				// Length (quick reference)
   
 public:
   // Constructor
   String(void)
     : ptr(new strptr), len(0)
     {};
   
   String(char const *);			// Migration constructor
   String(String const &);			// Copy constructor
   String(unsigned char);			// Migration constructor
   String(int);					// Migration constructor
   String(unsigned int);			// Migration constructor
   String(long);				// Migration constructor
   String(unsigned long);			// Migration constructor
   String(double);				// Migration constructor
   String(char);				// MIgration constructor
   
   ~String(void);				// Destructor
   
   String & operator=(char const *);		// Set operator (migration)
   String & operator=(String const &);		// Set operator
   
   char & operator[](length_t);			// Get char operator
   char const & operator[](length_t) const;	// Get char operator
   
   // Boolean equals operator
   bool operator==(char const *s) const
     {
	return (strcmp(ptr->str, s) == 0);
     };
   
   // Boolean equals operator
   bool operator==(String const &s) const
     {
	return ((ptr->str == s.ptr->str) ||
		(strcmp(ptr->str, s.ptr->str) == 0));
     };
   
   // Boolean not-equals operator
   bool operator!=(char const *s) const
     {
	return (strcmp(ptr->str, s) != 0);
     };
   
   // Boolean not-equals operator
   bool operator!=(String const &s) const
     {
	return (strcmp(ptr->str, s.ptr->str) != 0);
     };
   
   // Boolean less-than operator
   bool operator<(String const &s) const
     {
	return (strcmp(ptr->str, s.ptr->str) < 0);
     };
   
   // Boolean greater-than operator
   bool operator>(String const &s) const
     {
	return (strcmp(ptr->str, s.ptr->str) > 0);
     };
   
   String operator+(char const *) const;	// Addition operator
   String operator+(String const &) const;	// Addition operator
   
   // Migration operator
   operator char const *(void) const
     {
	return ptr->str;
     };
   
   // Migration operator
   operator char *(void) const
     {
	return ptr->str;
     };
   
   // IO-stream output operator
   friend ostream & operator<<(ostream &s, String const &line)
     {
	return (s << line.ptr->str);
     };

   // IO-stream input operator
   friend istream & operator>>(istream &, String &);

   // Get the length of a string
   length_t length(void) const
     {
	return len;
     };
   
   length_t find(char) const;			// Find the location of a char
   
   String subString(length_t, length_t) const;	// Sub-string from x to y
   
   // Sub-string from x to end
   String subString(length_t start) const
     {
	return subString(start, len -1);
     };
   
   String toLower(void) const;			// Lower-case the string
   String toUpper(void) const;			// Upper-case the string

   String IRCtoLower(void) const;		// toLower() with IRC chars
   
   int toInt(void) const			// Convert to an integer
     {
	return atoi(ptr->str);
     };
   
   long toLong(void) const 			// Convert to a long int
     {
	return atol(ptr->str);
     };
   
   double toDouble(void) const			// Convert to a double
     {
	return atof(ptr->str);
     };
   
   String trim(void) const;			// Trim whitespace (pre/suf)fix
   String trimQuotes(void) const;		// Trim quote mark (pre/suf)fix
   
   String pad(length_t, char = ' ') const;	// Pad a string with chars
   String prepad(length_t, char = ' ') const;	// Prepad a string with chars

   static String printf(char *, ...);		// printf clone
};


// StringTokens for String class
class StringTokens {
 private:
   String str;					// The string
   String::length_t pos;			// Our position
   
 public:
   // Constructor
   StringTokens(String s, String::length_t p = 0)
     : str(s), pos(p)
     {};
   
   // Copy constructor
   StringTokens(StringTokens &st)
     : str(st.str), pos(st.pos)
     {};
   
   bool hasMoreTokens(void) const;		// Any more tokens left?
   bool hasMoreTokens(char) const;
   
   unsigned int countTokens(void);		// Count the number of tokens
   unsigned int countTokens(char);
   
   String nextToken(void);			// Get the next token
   String nextToken(char, bool = false);
   
   String nextColonToken(void);			// Get next IRC style token
   
   String rest(void);				// Get the rest of the line
};


// String masking routines for String class
class StringMask {
 private:
   String mask;					// The mask itself
   
   static bool match(char const *, 
		     char const *);		// Match checker
   
 public:
   StringMask(void)				// Constructor
     : mask("")
     {};
   
   StringMask(String s)				// Constructor
     : mask(s)
     {};
   
   StringMask(StringMask const &sm)		// Copy constructor
     : mask(sm.mask)
     {};

   bool operator==(StringMask const &sm) const	// Boolean equals operator
     {
	return mask == sm.mask;
     };
   
   bool operator!=(StringMask const &sm) const	// Boolean not-equals operator
     {
	return mask != sm.mask;
     };
   
   String getMask(void) const			// Return the mask
     {
	return mask;
     };
   
   bool matches(String const &s) const		// Check string match
     {
	return match((char const *)mask, (char const *)s);
     };

   bool matches(char const *s) const
     {
	return match((char const *)mask, s);
     };
};


# ifdef STL_HAS_HASH
// STL hash templates for String class
template<> struct hash<String>
{
   size_t operator()(String const &) const;	// STL Hash operator
};
# endif

#endif
