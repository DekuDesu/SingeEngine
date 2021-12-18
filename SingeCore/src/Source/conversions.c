#include "singine/conversions.h"
#include "singine/guards.h"
#include <ctype.h>

void ToLower(char* buffer, size_t bufferLength, size_t offset)
{
	GuardNotNull(buffer);
	GuardNotZero(bufferLength);
	GuardLessThan(offset, bufferLength);

	for (size_t i = offset; i < bufferLength; i++)
	{
		buffer[i] = tolower(buffer[i]);
	}
}

void ToUpper(char* buffer, size_t bufferLength, size_t offset)
{
	GuardNotNull(buffer);
	GuardNotZero(bufferLength);
	GuardLessThan(offset, bufferLength);

	for (size_t i = offset; i < bufferLength; i++)
	{
		buffer[i] = toupper(buffer[i]);
	}
}