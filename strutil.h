#ifndef _strutil_h_
#define _strutil_h_

#include <string>
#include <iostream>
#include <sstream> 

template <typename T> std::string tostr(const T& t) { 
   std::ostringstream os; 
   os<<t; 
   return os.str(); 
} 


#endif
