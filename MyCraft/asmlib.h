/*                         ASMLIB.H                           Agner Fog 2007

Header file for the asmlib function library.

This library is available in several versions for different platforms:

alibomf32.lib  32-bit Windows, OMF file format for Borland compiler
alibcof32.lib  32-bit Windows, COFF file format for Microsoft compiler
alibd32.dll    32-bit Windows, DLL for other programming languages
alibcof64.lib  64-bit Windows, COFF/PE32+ file format
alibd64.dll    64-bit Windows, DLL for other programming languages
alibelf32.a    32-bit Linux and BSD, ELF file format
alibelf64.a    64-bit Linux and BSD, ELF file format
alibmac32.a    32-bit Mac OS X (Intel based). Mach-O file format

Description of functions:

int InstructionSet()
--------------------
This function detects which instructions are supported by the microprocessor
and the operating system. Return value:
0          = use 80386 instruction set only
1 or above = MMX instructions supported
2 or above = conditional move and FCOMI supported
3 or above = SSE (XMM) supported by processor and operating system
4 or above = SSE2 supported by processor and operating system
5 or above = SSE3 supported by processor and operating system
6 or above = Supplementary SSE3 supported by processor and operating system
7 or above = SSE4.1 supported by processor and operating system
8 or above = SSE4.2 supported by processor and operating system

char * ProcessorName()
----------------------
Returns a pointer to a static zero-terminated ASCII string with a 
description of the microprocessor.

uint64 ReadTSC()
----------------
This function returns the value of the internal clock counter in the 
microprocessor. Useful for measuring how many clock cycles a piece
of code. Also useful as a seed for a random number generator.

void Serialize()
----------------
Serialize before and after __readpmc() to avoid out-of-order execution
to mess up the performance counters.

int Round(double x)
int Round(float  x)
-------------------
Round to nearest integer. (When two integers are equally near, the 
even integer is returned. Does not check for overflow).

Random number generators
------------------------
The random number generator functions and classes are declared in asmlibran.h.

See asmlib.txt for further details

(c) 2003 - 2007. GNU General Public License (www.gnu.org/copyleft/gpl.html).

******************************************************************************/

#ifndef ASMLIB_H
#define ASMLIB_H


/***********************************************************************
Define compiler-specific directives
***********************************************************************/

// Define 32 bit signed and unsigned integers.
// Change these definitions, if necessary, to match a particular platform
#if defined(_WIN16) || defined(__MSDOS__) || defined(_MSDOS) 
   // 16 bit systems use long int for 32 bit integer
   typedef long int           int32;   // 32 bit signed integer
   typedef unsigned long int  uint32;  // 32 bit unsigned integer
#else
   // Most other systems use int for 32 bit integer
   typedef int                int32;   // 32 bit signed integer
   typedef unsigned int       uint32;  // 32 bit unsigned integer
#endif

// Define 64 bit signed and unsigned integers, if possible
#if (defined(__WINDOWS__) || defined(_WIN32)) && (defined(_MSC_VER) || defined(__INTEL_COMPILER))
   // Microsoft and other compilers under Windows use __int64
   typedef __int64            int64;   // 64 bit signed integer
   typedef unsigned __int64   uint64;  // 64 bit unsigned integer
   #define INT64_DEFINED               // Remember that int64 is defined
#elif defined(__unix__) && (defined(_M_IX86) || defined(_M_X64))
   // Gnu and other compilers under Linux etc. use long long
   typedef long long          int64;   // 64 bit signed integer
   typedef unsigned long long uint64;  // 64 bit unsigned integer
   #define INT64_DEFINED               // Remember that int64 is defined
#else
   // 64 bit integers not defined
   // You may include definitions for other platforms here
#endif

// Define directives for aligning by 16
#if (defined(__WINDOWS__) || defined(_WIN32))
   #if defined(_MSC_VER) || defined(__INTEL_COMPILER)
      // MS and Intel compilers for Windows have alignment directive before class declaration
      #define ALIGN_PRE   __declspec(align(16))
      #define ALIGN_POST 
      #define ALIGN_SUPPORTED
   #else // Other compiler may not support alignment
      #define ALIGN_PRE
      #define ALIGN_POST
   #endif
#elif defined(__unix__) || defined(__linux__) || defined(__GNUC__)
   // Unix compilers have alignment directive after class declaration
   #define ALIGN_PRE
   #define ALIGN_POST  __attribute((aligned(16)))
   #define ALIGN_SUPPORTED
#else
   // Other platforms not supported
   #error  Possibly wrong platform. Must be x86 or x86-64
#endif

// Define macro for extern "C" __stdcall call for dynamic linking:
#if defined(_WIN32) && !defined(_WIN64)
   // __stdcall used only in 32-bit Windows
   #define DLL_STDCALL  __stdcall
#else
   // 64-bit Windows has only one calling convention.
   // 32 and 64 bit Linux does not use __stdcall for dynamic linking
   #define DLL_STDCALL
#endif

// Turn off name mangling
#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
Function prototypes for simple functions
***********************************************************************/
int InstructionSet(void);              // Tell which instruction set is supported
extern int IInstrSet;                  // Set by first call to InstructionSet() 
char * ProcessorName(void);            // ASCIIZ text describing microprocessor
void Serialize();                      // Serialize before and after __readpmc()
int RoundD (double x);                 // Round to nearest or even
int RoundF (float  x);                 // Round to nearest or even
#ifdef INT64_DEFINED
   uint64 ReadTSC(void);               // Read microprocessor internal clock (64 bits)
#else
   uint32 ReadTSC(void);               // Read microprocessor internal clock (only 32 bits supported by compiler)
#endif

#ifdef __cplusplus
}  // end of extern "C"

static inline int Round (double x) {   // Overload name Round
   return RoundD(x);}
static inline int Round (float  x) {   // Overload name Round
   return RoundF(x);}

#endif // __cplusplus

#endif // ASMLIB_H
