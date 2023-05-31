#include "string-tools.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*************************************************************************************************/

int IsHex(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
}

int IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

int IsAlnum(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int IsSpace(char c)
{
    /*
    * Ascii/UTF8
    * '\t' = 9
    * '\n' = 10
    * '\v' = 11
    * '\f' = 12
    * '\r' = 13
    */

    return (c == ' ') || (c >= '\t' && c <= '\r');
}

int GetHex(char c)
{
    return (c >= '0' && c <= '9') ? c - '0' : c - 'A' + 0xA;
}

char GetHexChar(char c)
{
    return c < 0xA ? c + '0' : (c - 0xA) + 'A';
}

int IntegerToString(int value, char* dst)
{
    int num, index = 0;

    if (value == 0)
    {
        *dst = '0';
        return 1;
    }

    for (unsigned int div = 1000000000; div > 0; div /= 10)
    {
        if (((num = (value / div) % 10)) || (index > 0))
            dst[index++] = '0' + num;
    }

    return index;
}

int IntegerToStringHex(int value, char* dst)
{
    unsigned int n, index = 0;

    if (value == 0)
    {
        *dst = '0';
        return 1;
    }

    int i = 28;
    while (1)
    {
        if ((n = (value >> i) & 0xF) || index > 0)
            dst[index++] = n < 0xA ? '0' + n : 'A' + (n - 0xA);

        if (i == 0)
            break;

        i -= 4;
    }

    return index;
}

int StringToInteger(const char* string)
{
    unsigned int n = 0;

    for (unsigned int v, i = 0; (v = string[i] - '0') < 10; i++)
        n = n * 10 + v;

    return n;
}

int StringToIntegerHex(const char* string)
{
    unsigned int n = 0;

    for (unsigned int i = 0; IsHex(string[i]); i++)
        n = n * 0x10 + GetHex(string[i]);

    return n;
}

/*************************************************************************************************/

static int CompileDigit(const char* string, char* c)
{
    unsigned char value = 0, i = 0;

    while (i < 3 && isdigit(string[i]) && value <= 25)
    {
        unsigned char result = value * 10 + (string[i] - '0');

        if (result < value)
            break;

        value = result;
        i++;
    }

    if (i == 0)
        return 0;

    *c = value;

    return i;
}

static int CompileHex(const char* string, char* c)
{
    unsigned char value = 0, i = 0;

    if (*(string++) != 'x')
        return 0;

    for (; i < 2 && isxdigit(string[i]); i++)
        value = value * 0x10 + GetHex(string[i]);

    if (i == 0)
        return 0;

    *c = value;

    return i + 1;
}

static int CompileSpace(const char* string, char* c)
{
    /*
    * Ascii/UTF8
    * '\t' = 9
    * '\n' = 10
    * '\v' = 11
    * '\f' = 12
    * '\r' = 13
    */
    const char* val = "\t\n\v\f\r";
    const int index = (int)(*string) - (int)'t';

    if (index < 0 || index > 4)
        return 0;
    
    *c = val[index];
    return 1;
}

static int CompileWildcardPatternSection(const char** pattern, char* buffer)
{
    int i = 0, j = 0;
    const char* pat = *pattern;

    while (pat[i] != '*' && pat[i] != 0)
    {
        if (pat[i] == '\\')
        {
            i++;
            int length = 0;

            if ((length = CompileDigit(pat + i, buffer + j)) ||
                (length = CompileHex(pat + i, buffer + j))   ||
                (length = CompileSpace(pat + i, buffer + j)))
            {

            }
            else if (pat[i] == '\\')
            {
                buffer[j] = '\\';
                length = 1;
            }

            i += length;
            j++;
        }
        else
        {
            buffer[j] = pat[i];
            j++;
            i++;
        }
    }

    if (pat[i] == '*')
        i++;

    *pattern = pat + i;

    return j;
}

/*************************************************************************************************/

static void ComputeKMPTable(const char* pattern, int* table, const int length)
{
    table[0] = 0;

    for (int i = 1, j = 0; i < (length - 1); i++)
    {
        while (j > 0 && pattern[i] != pattern[j])
            j = table[j - 1];

        if (pattern[i] == pattern[j])
            j++;

        table[i] = j;
    }
}

/*
* Prevent looking for the same value twice. Example: if pattern p = "abab" fails at p[3], it would
* backtrack to p[1]. Both are 'b' so this will fail aswell. Instead it will directly go to 0.
*/
static void OptimizeKMPTable(const char* pattern, int* table, const int length)
{
    for (int i = 1, j; i < (length - 1); i++)
    {
        j = table[i];
        while (j > 0 && pattern[i + 1] == pattern[j])
            j = table[j - 1];

        table[i] = j;
    }
}

const char* StringMatch(const char* text, int text_length, const char* pattern, int* match_length)
{
    int* table;
    char* string;
    const char* match;
    int pattern_length;
    int i, x;

    pattern_length = (int)strlen(pattern);

    if (pattern_length == 0)
        return NULL;

    /* kmp algorithm offset table buffer */
    table = malloc(pattern_length * (sizeof(int) + sizeof(char)));

    if (table == NULL)
        return NULL;

    /* compiled pattern buffer */
    string = (char*)(table + pattern_length);
    match = NULL;

    i = 0; /* text index */
    x = 0; /* section index */

    do
    {
        int length, j;

        /* Compile pattern until wildcard or NULL and move pattern pointer to next section. */
        length = CompileWildcardPatternSection(&pattern, string);
        ComputeKMPTable(string, table, length);
        OptimizeKMPTable(string, table, length);
        
        j = 0;

        while (i < text_length && j < length)
        {
            if (string[j] == text[i])
            {
                i++;
                j++;
            }
            else if (j != 0)
            {
                j = table[j - 1];
            }
            else
            {
                i++;
            }
        }

        if (j == length)
        {
            if (x == 0)
                match = text + i - length;

            x++;
            continue;
        }

        free(table);
        return NULL;

    } while (*pattern != 0);

    free(table);

    if (match_length != NULL)
        *match_length = (int)((text + i) - match);

    return match;
}

void StringReplaceChar(char* string, char find, char replace)
{
    while (string = strchr(string, (int)find))
        *string++ = replace;
}
