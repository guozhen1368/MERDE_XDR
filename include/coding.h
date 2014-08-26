/*
 * (C) Copyright 2008
 * Hu Chunlin <chunlin.hu@gmail.com>
 *
 * coding.h - A brief description goes here.
 *
 */

#ifndef _HEAD_CODING_1E340A5C_63852368_790F459E_H
#define _HEAD_CODING_1E340A5C_63852368_790F459E_H

#include <string.h>

#define CODING_8(ptr, val) { \
	unsigned char *coding_ptr = (unsigned char *)(ptr); \
	*coding_ptr = (val) & 0xff; \
	ptr++; \
}

#define DECODE_8(ptr, val, len) { \
	unsigned char *decoding_ptr = (unsigned char *)(ptr); \
	if ((len) < 1) \
		return -1; \
	(val) = *decoding_ptr; \
	ptr++; \
	(len) = (len) - 1; \
}

#define CODING_16(ptr, val) { \
	unsigned char *coding_ptr = (unsigned char *)(ptr); \
	*coding_ptr++ = ((val) >> 8) & 0xff; \
	*coding_ptr = (val) & 0xff; \
	ptr += 2; \
}

#define DECODE_16(ptr, val, len) { \
	unsigned char *decoding_ptr = (unsigned char *)(ptr); \
	if ((len) < 2) \
		return -1; \
	(val) = (*decoding_ptr << 8) | *(decoding_ptr + 1); \
	ptr += 2; \
	(len) = (len) - 2; \
}

#define CODING_24(ptr, val) { \
	unsigned char *coding_ptr = (unsigned char *)(ptr); \
	*coding_ptr++ = ((val) >> 16) & 0xff; \
	*coding_ptr++ = ((val) >> 8) & 0xff; \
	*coding_ptr = (val) & 0xff; \
	ptr += 3; \
}

#define DECODE_24(ptr, val, len) { \
	unsigned char *decoding_ptr = (unsigned char *)(ptr); \
	if ((len) < 3) \
		return -1; \
	(val) = (*decoding_ptr << 16) | \
	        (*(decoding_ptr + 1) << 8) | *(decoding_ptr + 2); \
	ptr += 3; \
	(len) = (len) - 3; \
}

#define CODING_32(ptr, val) { \
	unsigned char *coding_ptr = (unsigned char *)(ptr); \
	*coding_ptr++ = ((val) >> 24) & 0xff; \
	*coding_ptr++ = ((val) >> 16) & 0xff; \
	*coding_ptr++ = ((val) >> 8) & 0xff; \
	*coding_ptr = (val) & 0xff; \
	ptr += 4; \
}

#define DECODE_32(ptr, val, len) { \
	unsigned char *decoding_ptr = (unsigned char *)(ptr); \
	if ((len) < 4) \
		return -1; \
	(val) = (*decoding_ptr << 24) | \
	        (*(decoding_ptr + 1) << 16) | \
	        (*(decoding_ptr + 2) << 8) | *(decoding_ptr + 3); \
	ptr += 4; \
	(len) = (len) - 4; \
}

#define CODING_64(ptr, val) { \
	unsigned char *coding_ptr = (unsigned char *)(ptr); \
	*coding_ptr++ = ((val) >> 56) & 0xff; \
	*coding_ptr++ = ((val) >> 48) & 0xff; \
	*coding_ptr++ = ((val) >> 40) & 0xff; \
	*coding_ptr++ = ((val) >> 32) & 0xff; \
	*coding_ptr++ = ((val) >> 24) & 0xff; \
	*coding_ptr++ = ((val) >> 16) & 0xff; \
	*coding_ptr++ = ((val) >> 8) & 0xff; \
	*coding_ptr = (val) & 0xff; \
	ptr += 8; \
}

#define DECODE_64(ptr, val, len) { \
	unsigned char *decoding_ptr = (unsigned char *)(ptr); \
	if ((len) < 8) \
		return -1; \
	(val) = ((unsigned long long)*decoding_ptr << 56) | \
	        ((unsigned long long)*(decoding_ptr + 1) << 48) | \
	        ((unsigned long long)*(decoding_ptr + 2) << 40) | \
			((unsigned long long)*(decoding_ptr + 3) << 32) | \
			((unsigned long long)*(decoding_ptr + 4) << 24) | \
			((unsigned long long)*(decoding_ptr + 5) << 16) | \
			((unsigned long long)*(decoding_ptr + 6) << 8) | \
			(unsigned long long)*(decoding_ptr + 7); \
	ptr += 8; \
	(len) = (len) - 8; \
}

#define CODING_OCTSTRING(ptr, val, len) { \
	unsigned char *coding_ptr = (unsigned char *)(ptr); \
	if ((val == NULL) || (len == 0)) { \
		*coding_ptr = 0; \
		ptr += 1; \
	} \
	else { \
		if (len < 0x80) { \
			*coding_ptr++ = (unsigned char)len; \
			ptr += 1;\
		} \
		else if (len < 0x7f00) { \
			*coding_ptr++ = ((unsigned char)((len >> 8) & 0x7f)) | 0x80; \
			*coding_ptr++ = (unsigned char)(len & 0xff); \
			ptr += 2;\
		} \
		else { \
			*coding_ptr++ = 0xff; \
			*coding_ptr++ = (unsigned char)((len >> 24) & 0xff); \
			*coding_ptr++ = (unsigned char)((len >> 16) & 0xff); \
			*coding_ptr++ = (unsigned char)((len >> 8) & 0xff); \
			*coding_ptr++ = (unsigned char)((len) & 0xff); \
			ptr += 5;\
		} \
		memcpy(coding_ptr, (val), len); \
		ptr += len;\
	} \
}

#define CODING_STRING(ptr, val) { \
	unsigned int len = (unsigned int)strlen(val); \
	CODING_OCTSTRING(ptr, val, len); \
}

#define _DECODE_STRING(ptr, val, size, len, is_octstring) { \
	unsigned char *decoding_ptr = (unsigned char *)(ptr); \
	if (((len) < 1) || ((size) < 1)) \
		return -1; \
	\
	if (*decoding_ptr == 0) { \
		(val)[0] = 0; \
		ptr += 1; \
		(len) = (len) - 1; \
	} \
	else { \
		unsigned int slen; \
		\
		if (*decoding_ptr < 0x80) { \
			slen = (unsigned int)(*decoding_ptr); \
			decoding_ptr++; \
			(len) = (len) - 1; \
			ptr += 1; \
		} \
		else if (*decoding_ptr != 0xff) { \
			if ((len) < 2) \
				return -1; \
			slen = ((*decoding_ptr & 0x7f) << 8) | *(decoding_ptr + 1); \
			decoding_ptr += 2; \
			(len) = (len) - 2; \
			ptr += 2; \
		} \
		else { \
			if ((len) < 5) \
				return -1; \
			slen = (*(decoding_ptr + 1) << 24) | (*(decoding_ptr + 2) << 16) | \
				(*(decoding_ptr + 3) << 8) | *(decoding_ptr + 4); \
			decoding_ptr += 5; \
			(len) = (len) - 5; \
			ptr += 5; \
		} \
		if ((unsigned int)(len) < slen) \
			return -1; \
		(len) = (len) - slen; \
		ptr += slen; \
		\
		if (slen >= (size))  { \
			slen = (size); \
			if (!(is_octstring)) \
				--slen; \
		} \
		memcpy((val), decoding_ptr, slen); \
		if (!(is_octstring)) \
			(val)[slen] = 0; \
	} \
}

#define DECODE_STRING(ptr, val, sz, len)    _DECODE_STRING(ptr, val, sz, len, 0)
#define DECODE_OCTSTRING(ptr, val, sz, len) _DECODE_STRING(ptr, val, sz, len, 1)

#endif /* #ifndef _HEAD_CODING_1E340A5C_63852368_790F459E_H */
