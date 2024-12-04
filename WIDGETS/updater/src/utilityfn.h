#ifndef __UTILITYFN_H__
#define __UTILITYFN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

int Utility_hexToInt(const char * s, int count);
char * Utility_hexToInt64(const char * s, int64_t * dst);
void Utility_intToHex(char * dst, const void * src_ptr, int count);
void Utility_bytesToHex(char * dst, const unsigned char * src, size_t count);
void Utility_reverse(char * str, int sz);
int Utility_intToA(char * ptr, int value, int radix);
int Utility_intToAPadded(char * ptr, int value, int radix, int padding);
int Utility_aToInt(const char * ptr);
#ifdef UTILITY_INCLUDE_DOUBLE
double Utility_aToDouble(const char * ptr);
int Utility_doubleToA(char * s, double n);
int Utility_doubleToAEx(char * s, double n, double precision);
size_t Utility_doubleToASci(char * s, double n);
#endif /* UTILITY_INCLUDE_DOUBLE */
void Utility_hexToBytes(char * dst, size_t * dst_sz, char * src, char ** src_pos);
extern const char * const Utility_EMPTYSTRING;
char * Utility_reverseTokenise(char * string, char * tokens, int skip);
char * Utility_tokeniseString(char * string, char * delimiters, char ** context);
int Utility_removeCharacters(char * string, const char * tomatch);
int Utility_endsWith(const char * string, const char * tomatch);
int Utility_isInteger(const char * str);
int Utility_isNumber(const char * str);
int Utility_dumphex(char * destination, const char * source, int len);
void Utility_genrandom(char * s, const int len);
char * Utility_easyDump(void * source, size_t source_len);

#ifdef __cplusplus
};
#endif

#endif // __UTILITYFN_H__
