#ifdef USE_VLD
#  include <vld.h>
#endif

#include "utils.h"
#include <stdio.h>
#include <stdarg.h>

static char _log_buf[4096];

char* bmfm::mmap(const char* path)
{
	char* buf = nullptr;
	FILE* f = ::fopen(path, "rb");
	if (!f)
	{
		logerrfmt("mmap(): Failed on opening file: %s", path);
		return nullptr;
	}

	do
	{
		if (::fseek(f, 0, SEEK_END))
		{
			logerrfmt("mmap(): fseek() failed on file: %s", path);
			break;
		}

		long filelen = ::ftell(f);
		if (filelen < 0)
		{
			logerrfmt("mmap(): ftell() failed on file: %s", path);
			break;
		}

		::fseek(f, 0, SEEK_SET);
		buf = new char[filelen + 1];
		if (!buf)
		{
			logerrfmt("mmap(): Failed to allocate memory for size: %ld", filelen+1);
			break;
		}

		size_t cnt = ::fread(buf, 1, (size_t)filelen, f);
		if (cnt < (size_t)filelen)
		{
			logerrfmt("mmap(): I/O error on reading file: %s", path);
			delete[] buf;
			buf = nullptr;
			break;
		}
		buf[filelen] = '\0'; // ensure null-terminated.
	} while (0);

	::fclose(f);
	return buf;
}

void bmfm::log(const char* msg)
{
	if (msg)
		printf("%s\n", msg);
}

void bmfm::logfmt(const char* fmt, ...)
{
	va_list valist;
	va_start(valist, fmt);
	int len = vsnprintf(_log_buf, 4096, fmt, valist);
	if (len>0)
		printf("%s\n", _log_buf);
	va_end(valist);
}

void bmfm::logerr(const char* msg)
{
	if (msg)
		fprintf(stderr, "%s\n", msg);
}

void bmfm::logerrfmt(const char* fmt, ...)
{
	va_list valist;
	va_start(valist, fmt);
	int len = vsnprintf(_log_buf, 4096, fmt, valist);
	if (len>0)
		fprintf(stderr, "%s\n", _log_buf);
	va_end(valist);
}

