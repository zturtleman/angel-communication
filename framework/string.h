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

#ifndef ANGEL_STRING_INCLUDED
#define ANGEL_STRING_INCLUDED

#include <vector>

namespace AngelCommunication
{

/*
    String Class
    Simple string class to make handling char* simpler.
    Automatic change lengths using new and delete.
*/
class String
{
    private:
        char *data; // pointer to the text.
        unsigned int len; // length of the text, doesn't count the null.

    public:
        String(const String &text);
        String(const String *text);
        String(const char *newData);
        String(void);
        ~String(void);

        const char *c_str(void) const;
        unsigned int getLen(void) const;
        void setLen(unsigned int newlen);
        void setData(const String &text);
        void setData(const String *text);
        void setData(const char *newData);

        int compareTo(const String &str, int len = -1) const;
        int compareTo(const String *str, int len = -1) const;
        int compareTo(const char *str, int len = -1) const;

        int icompareTo(const String &str, int len = -1) const;
        int icompareTo(const String *str, int len = -1) const;
        int icompareTo(const char *str, int len = -1) const;

        bool validString(const unsigned long min) const;
		static bool validString(const char *string, const unsigned long min);

        void ltrim(void);
        void rtrim(void);
        void trim(void);

        String toUpper(size_t start = 0, size_t end = 0) const;
        String toLower(size_t start = 0, size_t end = 0) const;

        bool isEmpty(void) const;

        // File stuff.
        bool validPath(void) const;
        String removeFilename(void);
        String removeDir(void);
        bool replaceEXT(const char *_ext);
        String getEXT(void) const;
        bool fileExists(void) const;
        static bool fileExists(const char *filename);
        String replaceENV(void);

        bool isNum(void) const;

        /*
            snprintf
            Replaces this String with the formated data passed.
        */
        int snprintf(size_t nsize, const char *format, ...);

        /*
            append_snprintf
            Addes the formated data to the end of this String.
        */
        int append_snprintf(size_t nsize, const char *format, ...);

        /*
            append
            Addes str (or character) to the end of this String.
        */
        void append(const String &str);
        void append(const String *str);
        void append(const char *str);
        void append(char character);

        /*
            findString
            Returns true if str is found in this String.
        */
        bool findString(String str) const;
        bool findString(const char *str) const;

        /*
            replaceString
            Replaces all occerences of findStr with newData.
        */
        String replaceString(String findStr, String newData) const;
        String replaceString(const char *findStr, const char *newData) const;

        /*
            subscript
            Returns a String containing the data from start to end.
            ...or end to start. Data can be copied backward.
        */
        String subscript(size_t start, size_t end) const;

        /*
            insert
            Moves [where] through the end of the string up one index.
            Then set [where] to character.
        */
        void insert(size_t where, char character);

        /*
            remove
            Removes 'len' number of characters stating at 'start'
        */
        void remove(size_t start, size_t len);

        /*
            operator[]
            returns reference to the character at index.
        */
        char &operator[](unsigned int index) const;
        char &charAtIndex(unsigned int index) const;

        /*
            operator=
            Replaces this String with str.
        */
        String operator=(const String &str);
        String operator=(const char *str);

        bool operator==(const String &str) const;
};

} // end namespace AngelCommunication

#endif // ANGEL_STRING_INCLUDED

