#pragma once

namespace bmfm
{

/* 
   Memory map the file content at the given path.
   Please note that the caller should free the returned
   buffer using 'delete[] buffer;' after done using.
*/
char* mmap(const char* path);

void log(const char* msg);
void logfmt(const char* fmt, ...);
void logerr(const char* msg);
void logerrfmt(const char* fmt, ...);

};
