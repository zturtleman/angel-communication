/*
Angel Communication
Copyright (C) 2013 Zack Middleton <zturtleman@gmail.com>

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

// Revision History:
// Rev#   Data           Rev By   Description
//  1.0   Feb  11, 2008   ZTM      Initial Code
//  1.1   May  17, 2008   ZTM      The String class has grown and isn't as
//                                   simple as it started.
//  1.2   June 27, 2008   ZTM      Added to Shell Engine.
//        Oct 5, 2013     ZTM      Import into Angel Communication
//

#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <iostream>

#include "string.h"

#ifndef _WIN32
#define strnicmp strncasecmp
#define stricmp strcasecmp
#endif

namespace AngelCommunication
{

/*
    String::String
*/
String::String(const String &text)
{
    data = NULL;
    len = 0;
    setData(text.data);
}

String::String(const String *text)
{
    data = NULL;
    len = 0;
    if (text != NULL)
        setData(text->data);
}

/*
    String::String
*/
String::String(const char *newData)
{
    data = NULL;
    len = 0;
    setData(newData);
}

/*
    String::String
*/
String::String(void)
{
    data = NULL;
    len = 0;
}

/*
    String::~String
*/
String::~String(void)
{
    if (data != NULL)
    {
        delete[] data;
        data = NULL;
    }
}

/*
    String::c_str
    Returns a C string, pointer to a character array.
    This never returns NULL. If the String has no data return a one character
        array that contains a '\0' character.
*/
const char *String::c_str(void) const
{
    static char emptyCString[1] = { '\0' };

    if (data == NULL)
    {
        return (const char *)emptyCString;
    }

    return (const char *)data;
}

/*
    String::getLen
    Returns the Length of the String without the '\0'
*/
unsigned int String::getLen(void) const
{
    return len;
}

/*
    String::setLen
    newlen is the length to make th string without the '\0'.
*/
void String::setLen(unsigned int newlen)
{
    char *temp = NULL;

    if (len == newlen)
    {
        return;
    }

    if (newlen == 0)
    {
        delete[] data;
        data = NULL;
        len = 0;
        return;
    }

    temp = new char [newlen+1];

    // If the str gets longer set extra space to '\0's
    memset(temp, '\0', newlen+1);

    // Save data.
    if (data != NULL)
    {
        memcpy(temp, data, std::min(newlen, len));

        delete[] data;
        data = NULL;
    }

    data = temp;
    len = newlen;
}

/*
    String::setData
*/
void String::setData(const String &text)
{
	setData(text.data);
	return;
}

/*
    String::setData
*/
void String::setData(const String *text)
{
    if (text == NULL)
    {
        setData((const char *)NULL);
        return;
    }
	setData(text->data);
	return;
}

/*
    String::setData
*/
void String::setData(const char *newData)
{
    unsigned int newlen;

    if (newData == NULL)
    {
        return;
    }

    newlen = strlen(newData);

    if (newlen == 0)
    {
        if (data != NULL)
        {
            delete[] data;
            data = NULL;
        }
        len = 0;
        return;
    }

    if (data == NULL || len != newlen)
    {
        if (data != NULL)
        {
            delete[] data;
            data = NULL;
        }
        data = new char[newlen+1];
    }

    strcpy(data, newData);
    len = newlen;
    return;
}

/*
    String::compareTo
*/
int String::compareTo(const String &str, int len) const
{
    if (len >= 0)
    {
        return strncmp(c_str(), str.c_str(), len);
    }
    return strcmp(c_str(), str.c_str());
}

/*
    String::compareTo
*/
int String::compareTo(const String *str, int len) const
{
    if (str == NULL)
    {
        return 0;
    }
    if (len >= 0)
    {
        return strncmp(c_str(), str->c_str(), len);
    }
    return strcmp(c_str(), str->c_str());
}

/*
    String::compareTo
*/
int String::compareTo(const char *str, int len) const
{
    if (str == NULL)
    {
        return 0;
    }
    if (len >= 0)
    {
        return strncmp(c_str(), str, len);
    }
    return strcmp(c_str(), str);
}

/*
    String::icompareTo
*/
int String::icompareTo(const String &str, int len) const
{
    if (len >= 0)
    {
        return strnicmp(c_str(), str.c_str(), len);
    }
    return stricmp(c_str(), str.c_str());
}

/*
    String::icompareTo
*/
int String::icompareTo(const String *str, int len) const
{
    if (str == NULL)
    {
        return 0;
    }
    if (len >= 0)
    {
        return strnicmp(c_str(), str->c_str(), len);
    }
    return stricmp(c_str(), str->c_str());
}

/*
    String::icompareTo
*/
int String::icompareTo(const char *str, int len) const
{
    if (str == NULL)
    {
        return 0;
    }
    if (len >= 0)
    {
        return strnicmp(c_str(), str, len);
    }
    return stricmp(c_str(), str);
}

/*
    String::validString
    Returns true if string's length is more or equal to min, and
    doesn't start with a odd char.
*/
bool String::validString(const unsigned long min) const
{
    // Check if the string is long enough.
    if (getLen() < min)
    {
        return false;
    }

    if (isEmpty() == false)
    {
        // Check if the first subscript is odd.
        if (data[0] == '\0')
        {
            return false;
        }
        if (data[0] == '\n')
        {
            return false;
        }
        if (data[0] == '\r')
        {
            return false;
        }
        if (data[0] == '\t')
        {
            return false;
        }
        if (data[0] == -52) // Tab
        {
            return false;
        }
    }

    return true;
}

/*
    String::validString
    Returns true if string's length is more or equal to min, and
    doesn't start with a odd char.
*/
bool String::validString(const char *string, const unsigned long min)
{
    if (string == NULL)
    {
        return false;
    }

    // Check if the string is long enough.
    if (strlen(string) < min)
    {
        return false;
    }

    // Check if the first subscript is odd.
    if (string[0] == '\0')
    {
        return false;
    }
    if (string[0] == '\n')
    {
        return false;
    }
    if (string[0] == '\r')
    {
        return false;
    }
    if (string[0] == '\t')
    {
        return false;
    }
    if (string[0] == -52) // Tab
    {
        return false;
    }

    return true;
}

/*
    String::ltrim
    Remove unneeded spaces off the left side
*/
void String::ltrim(void)
{
    size_t i = 0, len = 0;
    char *temp = NULL;

    if (getLen()  < 2)
    {
        return;
    }

    temp = new char [getLen()+1];

    strcpy(temp, c_str());
    len = getLen();

    for (i = 0; i < len; i++)
    {
        if (temp[i] != ' ')
        {
            break;
        }
    }

    setData(&temp[i]);
    delete[] temp;
    return;
}

/*
    String::rtrim
    Remove unneeded spaces off the right side, by setting them to '\0'.
*/
void String::rtrim(void)
{
    char *str = NULL;

    if (getLen() < 2)
    {
        return;
    }

    str = new char [getLen()+1];
    strcpy(str, c_str());

    for (size_t i = getLen(); i > 0; i--)
    {
        if (str[i] != ' '
            && str[i] != '\0')
        {
            break;
        }

        str[i] = '\0';
    }

    setData(str);

    delete[] str;

    return;
}

/*
    String::trim
    Removes the spaces at the begining and end of string.
*/
void String::trim(void)
{
    rtrim();
    ltrim();
    return;
}

/*
    String::toUpper
    Returns this String's data, the characters between start and end will
        be converted to uppercase.
    If end is 0 the end is set to the length of the String.
*/
String String::toUpper(size_t start, size_t end) const
{
    String s;
    char *str = NULL;
    size_t len = getLen();

    if (len == 0)
    {
        return String();
    }

    str = new char [len+1];
    strcpy(str, data);

    if (end == 0 || end > len)
    {
        end = len;
    }

    for (size_t i = start; i < end; ++i)
    {
        // The only different between toUpper and toLower
        // is right here.
        if (str[i] >= 'a' && str[i] <= 'z')
        {
            str[i] -= ('a' - 'A');
        }
    }

    s.setData(str);
    delete[] str;
    return s;
}

/*
    String::toLower
    Returns this String's data, the characters between start and end will
        be converted to lowercase.
    If end is 0 the end is set to the length of the String.
*/
String String::toLower(size_t start, size_t end) const
{
    String s;
    char *str = NULL;
    size_t len = getLen();

    if (len == 0)
    {
        return String();
    }

    str = new char [len+1];
    strcpy(str, data);

    for (size_t i = start; i < end && i < len; ++i)
    {
        // The only different between toUpper and toLower
        // is right here.
        if (str[i] >= 'A' && str[i] <= 'Z')
        {
            str[i] += ('a' - 'A');
        }
    }

    s.setData(str);
    delete[] str;
    return s;
}

/*
    String::isEmpty
    Returns true if the string is empty.
*/
bool String::isEmpty() const
{
    return (getLen() == 0);
}

/*
    String::validPath
    returns true if the path doesn't contain invalid characters.
*/
bool String::validPath(void) const
{
    size_t length = getLen();
    if (isEmpty() == true)
    {
        return true;
    }

    for (size_t i = 0; i < length; i++)
    {
        // Note: Filenames can't have \, /, or : but path can.
        if (data[i] == '*' ||
            data[i] == '?' ||
            data[i] == '<' ||
            data[i] == '>' ||
            data[i] == '|')
        {
            return false;
        }
        else if (data[i] == '"')
        {
            // Only allow the first and last chastacters
            // to be '"'.
            if (i != 0 && i != length - 1)
            {
                return false;
            }
        }
    }
    return true;
}

/*
    String::removeFilename
*/
String String::removeFilename(void)
{
    String str(data);

    if (str.c_str() == NULL || str.getLen() == 0)
    {
        return String();
    }

    // Check if the str has no file name.
    if (str[str.getLen()-1] == '\\' || str[str.getLen()-1] == '/')
    {
        return str;
    }

    // Start at the end of the string, check each char in str for '\\'
    // when it is found set the lenght to i
    // to remove the filename but keep the '\\'.
    for (int i = str.getLen()-1; i > 0; i--)
    {
        if (str[i] == '\\' || str[i] == '/')
        {
            str.setLen(i+1); // Keep the last '\\'
            break; // Stop or else the will only have the drive letter.
        }
    }

    return str;
}

/*
    String::removeDir
*/
String String::removeDir(void)
{
    String str(data);
    char *temp = NULL;

    if (str.c_str() == NULL || str.getLen() == 0)
    {
        return String();
    }

    // Check if the str has no file name.
    if (str[str.getLen()-1] == '\\' || str[str.getLen()-1] == '/')
    {
        return String();
    }

    // Start at the end of the string, check each char in str for '\\'
    // when it is found set the lenght to i
    // to remove the filename but keep the '\\'.
    for (int i = str.getLen()-1; i > 0; i--)
    {
        if (str[i] == '\\' || str[i] == '/')
        {
            // Alloc temp
            temp = new char [str.getLen()-i];

            // Copy the needed data.
            strncpy(temp, &data[i+1], str.getLen()-i);

            // Copy temp to str.
            str.setData(temp);

            // Free temp
            delete[] temp;
            temp = NULL;

            // Stop or else all that will be left is the first dir.
            break;
        }
    }

    return str;
}

/*
    String::replaceEXT
    Replaces/Adds the EXT in the String to _ext.

    if _ext is "", _ext[0] == '\0', removes the EXT.
*/
bool String::replaceEXT(const char *_ext)
{
    char *dest = NULL;
    char *ext = NULL;
    size_t destLen = 0;
    size_t extLen = 0;
    size_t size = 0;

    if (_ext == NULL)
    {
        return false;
    }

    if (isEmpty() == true)
    {
        return false;
    }

    // Always have ".EXT", don't allow "EXT"
    if (_ext[0] != '.' && strlen(_ext) > 0)
    {
        ext = new char [strlen(_ext)+1+1];
        strcpy(ext, ".");
        strcat(ext, _ext);
    }
    else if (_ext[0] != '\0')
    {
        ext = new char [strlen(_ext)+1];
        strcpy(ext, _ext);
    }

    // Length of ext.
    if (ext != NULL)
    {
        extLen = strlen(ext);
    }

    // size of dest.
    size = getLen()+extLen+1;

    dest = new char [size];

    strcpy(dest, c_str());
    destLen = strlen(dest);

    // must at lest be "A.EXT"
    if (getLen() > 4)
    {
        for (size_t i = destLen; i > 0; i--)
        {
            if (dest[i] == '.')
            {
                dest[i] = '\0';
                if (extLen > 0)
                {
                    strncat(dest, ext, size);
                    setData(dest);
                }

                // Don't forget to free memory!
                delete[] dest;
                if (ext != NULL)
                {
                    delete[] ext;
                }
                return true;
            }
        }
    }

    // If no EXT was found stop looking.
    if (extLen == 0)
    {
        // Don't forget to free memory!
        delete[] dest;
        if (ext != NULL)
        {
            delete[] ext;
        }

        return true;
    }

    // If no '.' was found just add
    // it to the end of the string.
    strncpy(dest, c_str(), size);
    strcat(dest, ext);

    // Set the data.
    setData(dest);

    delete[] dest;
    if (ext)
    {
        delete[] ext;
    }

    return true;
}

/*
    String::getEXT
    Returns the part after the last '.'
*/
String String::getEXT() const
{
    String str;

    if (isEmpty() == true
        || this->data[getLen()] == '.')
    {
        return String();
    }

    for (long i = getLen()-1; i >= 0; --i)
    {
        if (this->data[i] == '.')
        {
            str = subscript(i+1, getLen());
            break;
        }
    }

    return str;
}

/*
    String::fileExists
    Returns true if "data of this String" as a file exists.
*/
bool String::fileExists(void) const
{
    if (c_str() == NULL)
    {
        return false;
    }
    return (access(c_str(), 0) == 0);
}

/*
    String::fileExists
    Returns true if filename exists.
*/
bool String::fileExists(const char *filename)
{
    if (filename == NULL)
    {
        return false;
    }
    return (access(filename, 0) == 0);
}

/*
    String::replaceENV
    Returns a String with all of "$(envname)"s replaced with there envvar.
*/
String String::replaceENV()
{
    String output;
    size_t outputLen = getLen();

    if (findString("$(") == false)
    {
        return String(*this);
    }

    for (size_t i = 0; i < outputLen; /* */)
    {
        if (strncmp(&this->data[i], "$(", 2) == 0)
        {
            size_t start = i;
            size_t size = 0;
            String envName;
            char *env = NULL;

            i += 2; // Skip "$(" when reading env name.

            for (size = 0; this->data[i] != ')'; ++size, ++i)
            {
                envName.append(this->data[i]);
            }

            // Next data read at start + size + 3.
            i = start + size + 3; // Add 3 to skip "$(" and ")"

            //cout << "envName: \"" << envName.c_str() << "\"" << endl;

            env = getenv(envName.c_str());
            if (env == NULL)
            {
                // Can't replace...
                output.append("$(");
                output.append(envName);
                output.append(")");
            }
            else
            {
                //cout << envName.c_str() << ": " << env << endl;
                output.append(env);
                outputLen += strlen(env);
            }
        }
        else
        {
            // Add the character.
            output.append(this->data[i]);
            ++i;
        }
    }

    return output;
}

/*
    String::isNum
    Returns true if string is a number.
*/
bool String::isNum(void) const
{
    if (c_str() == NULL)
    {
        return false;
    }

    for (size_t i = 0; i < getLen(); i++)
    {
        // Check if the digit is valid
        // for a number.
        if (!(data[i] >= '0' && data[i] <= '9')
            && data[i] != '.')
        {
            // Found invalid data.
            return false;
        }
    }

    // The string is valid for a number.
    return true;
}

/*
    String::snprintf
    Works like snprintf, the data is set to this String.
    nsize is the size of the buffer.
*/
int String::snprintf(size_t nsize, const char *fmt, ...)
{
	va_list argptr;
	char *str = NULL;
    int rtn = 0;

    try
    {
        str = new char [nsize+1];
    }
    catch (std::bad_alloc) // Couldn't alloc memory.
    {
        return 0;
    }

	va_start(argptr, fmt);
	rtn = vsnprintf (str, nsize, fmt, argptr);
	va_end(argptr);

    setData(str); // Set the data to str.

    if (str != NULL)
        delete[] str;

	return rtn;
}

/*
    String::append_snprintf
    Works like snprintf only the data is append to this String.
    nsize is the size of the buffer.
*/
int String::append_snprintf(size_t nsize, const char *fmt, ...)
{
	va_list argptr;
	char *str = NULL;
    int rtn = 0;

    try
    {
        str = new char [nsize+1];
    }
    catch (std::bad_alloc) // Couldn't alloc memory.
    {
        return 0;
    }

	va_start(argptr, fmt);
	rtn = vsnprintf (str, nsize, fmt, argptr);
	va_end(argptr);

    append(str); // Append it to the current data.

    if (str != NULL)
        delete[] str;

	return rtn;
}

/*
    String::append
    Appends str to this String.
*/
void String::append(const String &str)
{
    if (str.isEmpty() == true)
    {
        return;
    }
    if (isEmpty() == true)
    {
        setData(str);
        return;
    }
    setLen(getLen()+str.getLen()+1);
    strcat(data, str.c_str());
}

/*
    String::append
    Appends str to this String.
*/
void String::append(const String *str)
{
    if (str == NULL)
    {
        return;
    }
    if (str->isEmpty() == true)
    {
        return;
    }
    if (isEmpty() == true)
    {
        setData(str);
        return;
    }
    setLen(getLen()+str->getLen()+1);
    strcat(data, str->c_str());
}

/*
    String::append
    Appends str to this String.
*/
void String::append(const char *str)
{
    if (str == NULL)
    {
        return;
    }
    if (isEmpty() == true)
    {
        setData(str);
        return;
    }
    setLen(getLen()+strlen(str)+1);
    strcat(data, str);
}

/*
    String::append
*/
void String::append(char character)
{
    setLen(getLen()+1); // Make the String one character longer.
    this->data[getLen()-1] = character; // -1 because idex is zero based.
}

/*
    String::findString
    Returns true if str is found in this String.
*/
bool String::findString(String str) const
{
    if (isEmpty() == true
        || str.isEmpty() == true)
    {
        return false;
    }

    return (strstr(data, str.data) != NULL);
}

/*
    String::findString
    Returns true if str is found in this String.
*/
bool String::findString(const char *str) const
{
    if (isEmpty() == true
        || str == NULL || strlen(str) == 0)
    {
        return false;
    }

    return (strstr(data, str) != NULL);
}

/*
    String::replaceString
    Returns a String
*/
String String::replaceString(String findStr, String newData) const
{
    String output;
    size_t findLen = 0;
    size_t inputLen = getLen();

    if (findStr.isEmpty() == true)
    {
        return String(*this);
    }

    findLen = findStr.getLen();

    for (size_t i = 0; i < (inputLen - findLen)+1; /* */)
    {
        if (strncmp(&this->data[i], findStr.c_str(), findLen) == 0)
        {
            if (newData.isEmpty() == false && newData.getLen() > 0)
            {
                output.append(newData);
                /*if (output[getLen()] == '\0')
                {
                    output.setLen(output.getLen()-1); // Remove '\0'
                }*/
            }
            inputLen += findLen;
            i += findLen;
        }
        else
        {
            // Add the character.
            output.append(this->data[i]);
            i += 1;
        }
    }

    // Append '\0'
    if (output[getLen()] != '\0')
    {
        output.append('\0');
    }

    return output;
}

/*
    String::replaceString
    Returns a String.
*/
String String::replaceString(const char *findStr, const char *newData) const
{
    String output;
    String find, set;

    if (findStr == NULL)
    {
        return String(*this);
    }

    find.setData(findStr);
    set.setData(newData);

    output.setData(replaceString(find,set));
    return output;
}

/*
    String::subscript
    Returns a String containing the data from start to end.
    ...or end to start. Data can be copied backward.
*/
String String::subscript(size_t start, size_t end) const
{
    bool backward = false;
    size_t len = 0;
    char *temp = NULL;
    String str;

    if (end > getLen())
    {
        end = getLen();
    }
    if (start > getLen())
    {
        start = getLen();
    }

    // Copy the data backward...
    if (start > end)
    {
        backward = true;
        len = start-end;
    }
    // Copy the data forward.
    else
    {
        backward = false;
        len = end-start;
    }

    if (len < 1)
    {
    	len = 1;
	}

    temp = new char [len+2];
    memset(temp, 0, len+2);

    if (backward == true)
    {
        // Copy the data backward...
        for (size_t i = end, j = 0; i >= start; --i, ++j)
        {
            temp[j] = data[i];
        }
    }
    else
    {
        // Copy the data forward.
        for (size_t i = start; i <= end; ++i)
        {
            temp[i-start] = data[i];
        }
    }

    str.setData(temp);
    delete[] temp;

    return str;
}

/*
    String::insert
    Moves [where] through the end of the string up one index.
    Then set [where] to character.
*/
void String::insert(size_t where, char character)
{
#if 1
    String /*s1, */s2;

    //s1 = subscript(0, where-1);
    s2 = subscript(where, getLen());

    //setLen(0);
    setLen(where);

    //append(s1);
    append(character);
    append(s2);

#else // Doesn't seem to work.
    setLen(getLen()+1);

    strcpy(&this->data[where+1], &this->data[where]);
    this->data[where] = character;
#endif
}

/*
    String::remove
    Removes 'len' number of characters stating at 'start'
*/
void String::remove(size_t start, size_t len)
{
    if (start >= getLen())
        return;
    if (len == 0)
        return;
    if (len > getLen()-start)
        len = getLen()-start;

    strcpy(&this->data[start], &this->data[start+len]);
    setLen(getLen() - len);
}

/*
    String::operator[]
    Returns the character at index.
    if index is invalid returns 0.
*/
char &String::operator[](unsigned int index) const
{
    return charAtIndex(index);
}

char &String::charAtIndex(unsigned int index) const
{
    if (index >= getLen())
    {
        static char dummy;
		dummy = 0;
        return dummy;
    }

    return data[index];
}

String String::operator=(const String &str)
{
    setData(str);
    return *this;
}

String String::operator=(const char *str)
{
    setData(str);
    return *this;
}

bool String::operator==(const String &str) const
{
    return (icompareTo(str) == 0);
}

} // end namespace AngelCommunication

