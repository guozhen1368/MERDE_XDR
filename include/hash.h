/*
 * (C) Copyright 2008
 * Hu Chunlin <chunlin.hu@gmail.com>
 *
 * hash.h - A brief description goes here.
 *
 */

#ifndef _HEAD_HASH_732968DD_45737B9F_16867DAF_H
#define _HEAD_HASH_732968DD_45737B9F_16867DAF_H
#ifndef WIN32
#include <bits/wordsize.h>
#else
#pragma warning(disable : 4311)
#ifndef __WORDSIZE
//#define __WORDSIZE (sizeof(long) * 8)
#define __WORDSIZE 32
#endif

#endif
#if !defined(__WORDSIZE)
#error __WORDSIZE not defined!
#endif

#if __WORDSIZE == 32
/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define GOLDEN_RATIO_PRIME 0x9e370001UL
#elif __WORDSIZE == 64
/*  2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1 */
#define GOLDEN_RATIO_PRIME 0x9e37fffffffc0001UL
#else
#error Define GOLDEN_RATIO_PRIME for your __WORDSIZE.
#endif
#include <stdio.h>
#include <stdint.h>

static __inline unsigned long hash_long(unsigned long val, unsigned int bits)
{
	unsigned long hash = val;

#if __WORDSIZE == 64
	/*  Sigh, gcc can't optimise this alone like it does for 32 bits. */
	unsigned long n = hash;
	n <<= 18; hash -= n;
	n <<= 33; hash -= n;
	n <<= 3;  hash += n;
	n <<= 3;  hash -= n;
	n <<= 4;  hash += n;
	n <<= 2;  hash += n;
#else
	/* On some cpus multiply is faster, on others gcc will do shifts */
	hash *= GOLDEN_RATIO_PRIME;
#endif

	/* High bits are more random, so use them. */
	return hash >> (__WORDSIZE - bits);
}

static __inline unsigned long hash_ptr(void *ptr, unsigned int bits)
{
	return hash_long((unsigned long)ptr, bits);
}

/* ************************************************************************** */
static __inline unsigned int RSHash(char* str, unsigned int len)
{
   unsigned int b    = 378551;
   unsigned int a    = 63689;
   unsigned int hash = 0;
   unsigned int i    = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash = hash * a + (*str);
      a    = a * b;
   }

   return hash;
}
static __inline unsigned long RSHashUL(unsigned long val, unsigned int bits)
{
	unsigned int v = (unsigned int)val;
	unsigned int hash;

	hash = RSHash((char *)&v, sizeof(v));
	return (hash & ((1 << bits) - 1));
}
/* End Of RS Hash Function */

static __inline unsigned int JSHash(char* str, unsigned int len)
{
   unsigned int hash = 1315423911;
   unsigned int i    = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash ^= ((hash << 5) + (*str) + (hash >> 2));
   }

   return hash;
}
static __inline unsigned long JSHashUL(unsigned long val, unsigned int bits)
{
	unsigned int v = (unsigned int)val;
	unsigned int hash;

	hash = JSHash((char *)&v, sizeof(v));
	return (hash & ((1 << bits) - 1));
}
/* End Of JS Hash Function */


static __inline unsigned int PJWHash(char* str, unsigned int len)
{
   const unsigned int BitsInUnsignedInt = (unsigned int)(sizeof(unsigned int) * 8);
   const unsigned int ThreeQuarters     = (unsigned int)((BitsInUnsignedInt  * 3) / 4);
   const unsigned int OneEighth         = (unsigned int)(BitsInUnsignedInt / 8);
   const unsigned int HighBits          = (unsigned int)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
   unsigned int hash              = 0;
   unsigned int test              = 0;
   unsigned int i                 = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash = (hash << OneEighth) + (*str);

      if((test = hash & HighBits)  != 0)
      {
         hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
      }
   }

   return hash;
}
static __inline unsigned long PJWHashUL(unsigned long val, unsigned int bits)
{
	unsigned int v = (unsigned int)val;
	unsigned int hash;

	hash = PJWHash((char *)&v, sizeof(v));
	return (hash & ((1 << bits) - 1));
}
/* End Of  P. J. Weinberger Hash Function */


static __inline unsigned int ELFHash(char* str, unsigned int len)
{
   unsigned int hash = 0;
   unsigned int x    = 0;
   unsigned int i    = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash = (hash << 4) + (*str);
      if((x = hash & 0xF0000000L) != 0)
      {
         hash ^= (x >> 24);
      }
      hash &= ~x;
   }

   return hash;
}
static __inline unsigned long ELFHashUL(unsigned long val, unsigned int bits)
{
	unsigned int v = (unsigned int)val;
	unsigned int hash;

	hash = ELFHash((char *)&v, sizeof(v));
	return (hash & ((1 << bits) - 1));
}
/* End Of ELF Hash Function */


static __inline unsigned int BKDRHash(char* str, unsigned int len)
{
   unsigned int seed = 131; /* 31 131 1313 13131 131313 etc.. */
   unsigned int hash = 0;
   unsigned int i    = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash = (hash * seed) + (*str);
   }

   return hash;
}
static __inline unsigned long BKDRHashUL(unsigned long val, unsigned int bits)
{
	unsigned int v = (unsigned int)val;
	unsigned int hash;

	hash = BKDRHash((char *)&v, sizeof(v));
	return (hash & ((1 << bits) - 1));
}
/* End Of BKDR Hash Function */


static __inline unsigned int SDBMHash(char* str, unsigned int len)
{
   unsigned int hash = 0;
   unsigned int i    = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash = (*str) + (hash << 6) + (hash << 16) - hash;
   }

   return hash;
}
static __inline unsigned long SDBMHashUL(unsigned long val, unsigned int bits)
{
	unsigned int v = (unsigned int)val;
	unsigned int hash;

	hash = SDBMHash((char *)&v, sizeof(v));
	return (hash & ((1 << bits) - 1));
}
/* End Of SDBM Hash Function */


static __inline unsigned int DJBHash(char* str, unsigned int len)
{
   unsigned int hash = 5381;
   unsigned int i    = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash = ((hash << 5) + hash) + (*str);
   }

   return hash;
}
static __inline unsigned long DJBHashUL(unsigned long val, unsigned int bits)
{
	unsigned int v = (unsigned int)val;
	unsigned int hash;

	hash = DJBHash((char *)&v, sizeof(v));
	return (hash & ((1 << bits) - 1));
}
/* End Of DJB Hash Function */


static __inline unsigned int DEKHash(char* str, unsigned int len)
{
   unsigned int hash = len;
   unsigned int i    = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
   }
   return hash;
}
static __inline unsigned long DEKHashUL(unsigned long val, unsigned int bits)
{
	unsigned int v = (unsigned int)val;
	unsigned int hash;

	hash = DEKHash((char *)&v, sizeof(v));
	return (hash & ((1 << bits) - 1));
}
/* End Of DEK Hash Function */


static __inline unsigned int BPHash(char* str, unsigned int len)
{
   unsigned int hash = 0;
   unsigned int i    = 0;
   for(i = 0; i < len; str++, i++)
   {
      hash = hash << 7 ^ (*str);
   }

   return hash;
}
static __inline unsigned long BPHashUL(unsigned long val, unsigned int bits)
{
	unsigned int v = (unsigned int)val;
	unsigned int hash;

	hash = BPHash((char *)&v, sizeof(v));
	return (hash & ((1 << bits) - 1));
}
/* End Of BP Hash Function */


static __inline unsigned int FNVHash(char* str, unsigned int len)
{
   const unsigned int fnv_prime = 0x811C9DC5;
   unsigned int hash      = 0;
   unsigned int i         = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash *= fnv_prime;
      hash ^= (*str);
   }

   return hash;
}
static __inline unsigned long FNVHashUL(unsigned long val, unsigned int bits)
{
	unsigned int v = (unsigned int)val;
	unsigned int hash;

	hash = FNVHash((char *)&v, sizeof(v));
	return (hash & ((1 << bits) - 1));
}
/* End Of FNV Hash Function */


static __inline unsigned int APHash(char* str, unsigned int len)
{
   unsigned int hash = 0xAAAAAAAA;
   unsigned int i    = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash ^= ((i & 1) == 0) ? (  (hash <<  7) ^ (*str) * (hash >> 3)) :
                               (~((hash << 11) + ((*str) ^ (hash >> 5))));
   }

   return hash;
}
static __inline unsigned long APHashUL(unsigned long val, unsigned int bits)
{
	unsigned int v = (unsigned int)val;
	unsigned int hash;

	hash = APHash((char *)&v, sizeof(v));
	return (hash & ((1 << bits) - 1));
}
/* End Of AP Hash Function */
/* ************************************************************************** */

static __inline unsigned long LowerBits(unsigned long val, unsigned int bits)
{
	return (val & ((1 << bits) - 1));
}

static __inline unsigned long HigherBits(unsigned long val, unsigned int bits)
{
	return (val >> (__WORDSIZE - bits));
}

static __inline unsigned long SwapHashlong(unsigned long val, unsigned int bits)
{
	val = ((val & 0xff) << (__WORDSIZE - 8)) | (val >> 8);
	return hash_long(val, bits);
}

static __inline unsigned long RobertJenkins32(unsigned long val, unsigned int bits)
{
	val = (val + 0x7ed55d16) + (val << 12);
	val = (val ^ 0xc761c23c) ^ (val >> 19);
	val = (val + 0x165667b1) + (val << 5);
	val = (val + 0xd3a2646c) ^ (val << 9);
	val = (val + 0xfd7046c5) + (val << 3);
	val = (val ^ 0xb55a4f09) ^ (val >> 16);

	return (val & ((1 << bits) - 1));
}

static __inline unsigned long RobertJenkins32Higher(unsigned long val, unsigned int bits)
{
	val = (val + 0x7ed55d16) + (val << 12);
	val = (val ^ 0xc761c23c) ^ (val >> 19);
	val = (val + 0x165667b1) + (val << 5);
	val = (val + 0xd3a2646c) ^ (val << 9);
	val = (val + 0xfd7046c5) + (val << 3);
	val = (val ^ 0xb55a4f09) ^ (val >> 16);

	return ((val & 0xffffffff) >> (32 - bits));
}

static __inline unsigned long Mix(unsigned long val, unsigned int bits)
{
	val = (~val) + (val << 21);
	val = val ^ (val >> 24);
	val = (val + (val << 3)) + (val << 8);
	val = val ^ (val >> 14);
	val = (val + (val << 2)) + (val << 4);
	val = val ^ (val >> 28);
	val = val + (val << 31);

	return ((val & 0xffffffff) >> (32 - bits));
}

#endif /* #ifndef _HEAD_HASH_732968DD_45737B9F_16867DAF_H */
