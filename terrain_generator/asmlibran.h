/*                         ASMLIBRAN.H                         Agner Fog 2007

Header file for the random number generators in the asmlib function library.

This library is available in several versions for different platforms:

alibomf32.lib  32-bit Windows, OMF file format for Borland compiler
alibcof32.lib  32-bit Windows, COFF file format for Microsoft compiler
alibcof64.lib  64-bit Windows, COFF/PE32+ file format
alibelf32.a    32-bit Linux and BSD, ELF file format
alibelf64.a    64-bit Linux and BSD, ELF file format
alibmac32.a    32-bit Mac OS X (Intel based). Mach-O file format

Description of random number generators:
The Mersenne Twister is usually the preferred random number generator for 
demanding scientific applications.

The Mother-of-all generator is also an excellent random number generator.
It is simpler and uses less cache memory than the Mersenne Twister.

For general applications, it doesn't matter which of these generators 
you use.


Description of random number generator functions:
=================================================

Single-threaded versions:
-------------------------
void MersenneRandomInit(int seed);
void MotherRandomInit(int seed);
This function must be called before any random number is generated.
Different values for seed will generate different random number sequences.

void MersenneRandomInitByArray(uint32 seeds[], int length);
Alternative to MersenneRandomInit when the seed consists of more than one integer.

int MersenneIRandom (int min, int max);
int MotherIRandom   (int min, int max);
Generates a random integer in the interval from min to max, inclusive.

int MersenneIRandomX(int min, int max);
Same as MersenneIRandom. All possible output values have exactly the same
probability, whereas MersenneIRandom (and MotherIRandom) may have a  
slight bias when the interval length is very high and not a power of 2.

double MersenneRandom();
double MotherRandom();
Generates a random number x in the interval 0 <= x < 1 with uniform
distribution. The resolution is 2^(-32).

uint32 MersenneBRandom();
uint32 MotherBRandom();
Generates a random 32-bit number. All 32 bits are random.


DLL versions:
-------------
These functions use the __stdcall calling convention rather than __cdecl.
They are intended for use with alibd32.dll for programming languages that
do not support static linking. The function names are the same as above
with a D appended to the name.


Thread-safe versions:
---------------------
These functions are wrapped in the class CRandomMersenneA or CRandomMotherA.
Use these for multi-threaded applications or when an object-oriented design
is desired. There is no performance penalty for using these classes.
Each thread should have its own instance of the random number generator
class to prevent interaction between the threads. Make sure each instance
has a different seed.


Position-independent code:
--------------------------
Shared objects (*.so) in Linux, BSD and Mac OS-X require position-independent
code. The random number generators that are wrapped in C++ classes
CRandomMersenneA and CRandomMotherA are compatible with position-independent
code. The other versions are not position-independent in the 32-bit version.
These functions cannot be used inside shared objects in 32-bit Linux, BSD
and Mac OS-X. The 64-bit versions are position-independent.


(c) 1997-2007 GPL by A. Fog. See license.htm
***********************************************************************/

#ifndef ASMLIBRAN_H
#define ASMLIBRAN_H

// Header file for other functions in the library
#include "asmlib.h"


// Turn off name mangling
#ifdef __cplusplus
   extern "C" {
#endif

/***********************************************************************
Function prototypes for random number generators
***********************************************************************/

/***********************************************************************
Define functions for Mersenne Twister
Thread-safe, single-threaded and Windows DLL versions
***********************************************************************/

// Single-threaded static link versions for Mersenne Twister
void   MersenneRandomInit(int seed);                            // Re-seed
void   MersenneRandomInitByArray(uint32 seeds[], int length);   // Seed by more than 32 bits
int    MersenneIRandom (int min, int max);                      // Output random integer
int    MersenneIRandomX(int min, int max);                      // Output random integer, exact
double MersenneRandom();                                        // Output random float
uint32 MersenneBRandom();                                       // Output random bits

// Single-threaded dynamic link versions for Mersenne Twister
void   DLL_STDCALL MersenneRandomInitD(int seed);               // Re-seed
void   DLL_STDCALL MersenneRandomInitByArrayD(uint32 seeds[], int length); // Seed by more than 32 bits
int    DLL_STDCALL MersenneIRandomD (int min, int max);         // Output random integer
int    DLL_STDCALL MersenneIRandomXD(int min, int max);         // Output random integer, exact
double DLL_STDCALL MersenneRandomD();                           // Output random float
uint32 DLL_STDCALL MersenneBRandomD();                          // Output random bits

// Thread-safe library functions for Mersenne Twister
// The thread-safe versions have as the first parameter a pointer to a
// private memory buffer. These functions are intended to be called from
// the class CRandomMersenneA defined below. 
// If calling from C rather than C++ then supply a private memory buffer
// as Pthis. The necessary size of the buffer is given in the class 
// definition below. The buffer must be aligned by 16 because of the use 
// of XMM registers.
void   MersRandomInit(void * Pthis, int seed);                  // Re-seed
void   MersRandomInitByArray(void * Pthis, uint32 seeds[], int length); // Seed by more than 32 bits
int    MersIRandom (void * Pthis, int min, int max);            // Output random integer
int    MersIRandomX(void * Pthis, int min, int max);            // Output random integer, exact
double MersRandom  (void * Pthis);                              // Output random float
uint32 MersBRandom (void * Pthis);                              // Output random bits


/***********************************************************************
Define functions for Mother-of-all generator
Thread-safe, single-threaded and Windows DLL versions
***********************************************************************/
// Single-threaded static link versions for Mother-of-all
void   MotherRandomInit(int seed);                              // Re-seed
int    MotherIRandom (int min, int max);                        // Output random integer
double MotherRandom();                                          // Output random float
uint32 MotherBRandom();                                         // Output random bits

// Single-threaded dynamic link versions for Mother-of-all
void   DLL_STDCALL MotherRandomInitD(int seed);                 // Re-seed
int    DLL_STDCALL MotherIRandomD (int min, int max);           // Output random integer
double DLL_STDCALL MotherRandomD();                             // Output random float
uint32 DLL_STDCALL MotherBRandomD();                            // Output random bits

// Thread-safe library functions for Mother-of-all
// The thread-safe versions have as the first parameter a pointer to a 
// private memory buffer. These functions are intended to be called from
// the class CRandomMotherA defined below. 
// If calling from C rather than C++ then supply a private memory buffer
// as Pthis. The necessary size of the buffer is given in the class 
// definition below. The buffer must be aligned by 16 because of the use 
// of XMM registers.
void   MotRandomInit(void * Pthis, int seed);                   // Initialization
int    MotIRandom(void * Pthis, int min, int max);              // Get integer random number in desired interval
double MotRandom (void * Pthis);                                // Get floating point random number
uint32 MotBRandom(void * Pthis);                                // Output random bits

#ifdef __cplusplus
}  // end of extern "C"
#endif

/***********************************************************************
Define classes for thread-safe versions of random number generators
***********************************************************************/
#if defined(__cplusplus) && defined(ALIGN_SUPPORTED)

// Class for Mersenne Twister
ALIGN_PRE      // align by 16
class CRandomMersenneA {                                   // Encapsulate random number generator
public:
   CRandomMersenneA(uint32 seed) {                         // Constructor
      RandomInit(seed);}
   void RandomInit(uint32 seed) {                          // Re-seed
      MersRandomInit(this, seed);}
   void RandomInitByArray(uint32 seeds[], int length) {    // Seed by more than 32 bits
      MersRandomInitByArray(this, seeds, length);}
   int IRandom (int min, int max) {                        // Output random integer
      return MersIRandom(this, min, max);}
   int IRandomX(int min, int max) {                        // Output random integer, exact
      return MersIRandomX(this, min, max);}
   double Random() {                                       // Output random float
      return MersRandom(this);}
   uint32 BRandom() {                                      // Output random bits
      return MersBRandom(this);}
private:
   char internals[2672];                                   // Internal variables
} ALIGN_POST ; // align by 16


// Class for Mother-of-all
ALIGN_PRE      // align by 16
class CRandomMotherA {                                     // Encapsulate random number generator
public:
   void RandomInit(uint32 seed) {                          // Initialization
      MotRandomInit(this, seed);}
   int IRandom(int min, int max) {                         // Get integer random number in desired interval
      return MotIRandom(this, min, max);}
   double Random() {                                       // Get floating point random number
      return MotRandom(this);}
   uint32 BRandom() {                                      // Output random bits
      return MotBRandom(this);}
   CRandomMotherA(int seed) {                              // Constructor
      RandomInit(seed);}
protected:
   char internals[64];                                     // Internal variables
} ALIGN_POST ; // align by 16


#endif // __cplusplus && ALIGN_SUPPORTED

#endif // ASMLIBRAN
