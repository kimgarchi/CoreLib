#ifndef LOG_H
#define LOG_H

#include <string>
using std::string;


/* +------------------------------------------------------+
   | An Easy-to-use Log File                              |
   |                                                      |
   | Author : Remisa (itioma@naver.com)                   |
   | Homepage : blog.naver.com/itioma                     |
   | Last modification : 2007. 1. 25.                     |
   +------------------------------------------------------+ */


// write a string into the file
void Log(const string& str);
void Log(const char* str);
void Log(const int n);

// write a error msg into the file, call a message box, and shut down the program.
void FatalError(const string& str, const char* fileName=NULL, const int line=-1);
void FatalError(const char* str, const char* fileName=NULL, const int line=-1);

// integer to string helper
string I2S(int n);

#endif
