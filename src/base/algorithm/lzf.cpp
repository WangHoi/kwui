//|
//| this code is derived from:
//|

/*
 * Copyright (c) 2000 Marc Alexander Lehmann <pcg@goof.com>
 *
 * Redistribution and use in source and binary forms, with or without modifica-
 * tion, are permitted provided that the following conditions are met:
 *
 *   1.  Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *   3.  The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPE-
 * CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTH-
 * ERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "lzf.h"
#include <vector>

namespace base {
/*
 * size of hashtable is (1 << HLOG) * sizeof (char *)
 * decompression is independent of the hash table size
 * the difference between 15 and 14 is very small
 * for small blocks (and 14 is also faster).
 * For a low-memory configuration, use HLOG == 13;
 * For best compression, use 15 or 16.
 */
#ifndef HLOG
#define HLOG 15
#endif

 /*
  * sacrifice some compression quality in favour of speed.
  * (roughly 1-2% worse compression for large blocks and
  * 9-10% for small, redundant, blocks and 20% better speed in both cases)
  * In short: enable this for binary data, disable this for text data.
  */
#ifndef ULTRA_FAST
#define ULTRA_FAST 0
#endif

  /*
   * unconditionally aligning does not cost very much, so do it if unsure
   */
#ifndef STRICT_ALIGN
#define STRICT_ALIGN !defined(__i386)
#endif

   /*
	* use string functions to copy memory.
	* this is usually a loss, even with glibc's optimized memcpy
	*/
#ifndef USE_MEMCPY
#define USE_MEMCPY 1
#endif

	/*
	 * you may choose to pre-set the hash table (might be faster on modern cpus
	 * and large (>>64k) blocks)
	 */
#ifndef INIT_HTAB
#define INIT_HTAB 1
#endif

	 /*****************************************************************************/
	 /* nothing should be changed below */

typedef unsigned char byte;

#if !STRICT_ALIGN
/* for unaligned accesses we need a 16 bit datatype. */
#include <limits.h>
#if USHRT_MAX == 65535
typedef unsigned short word;
#elif UINT_MAX == 65535
typedef unsigned int word;
#else
#warn need 16 bit datatype when STRICT_ALIGN == 0
#undef STRICT_ALIGN
#define STRICT_ALIGN 1
#endif
#endif

#if USE_MEMCPY || INIT_HTAB
#include <string.h>
#endif

#include <errno.h>

#define HSIZE (1 << (HLOG))

/*
 * don't play with this unless you benchmark!
 * decompression is not dependent on the hash function
 * the hashing function might seem strange, just believe me
 * it works ;)
 */
#define FRST(p) (((p[0]) << 8) + p[1])
#define NEXT(v, p) (((v) << 8) + p[2])
#define IDX(h) ((((h ^ (h << 4)) >> (3 * 8 - HLOG)) + h * 3) & (HSIZE - 1))
 /*
  * IDX works because it is very similar to a multiplicative hash, e.g.
  * (h * 57321 >> (3*8 - HLOG))
  * the next one is also quite good, albeit slow ;)
  * (int)(cos(h & 0xffffff) * 1e6)
  */

#if 0
  /* original lzv-like hash function */
#define FRST(p) (p[0] << 5) ^ p[1]
#define NEXT(v, p) (v << 5) ^ p[2]
#define IDX(h) (h) & (HSIZE - 1)
#endif

#define MAX_LIT (1 << 5)
#define MAX_OFF (1 << 13)
#define MAX_REF ((1 << 8) + (1 << 3))

/*
 * compressed format
 *
 * 000LLLLL <L+1>    ; literal
 * LLLOOOOO oooooooo ; backref L
 * 111OOOOO LLLLLLLL oooooooo ; backref L+7
 *
 */
	unsigned int lzf_compress(const void* const in_data, unsigned int in_len,
		void* out_data, unsigned int out_len) {
	std::vector<const byte*> htab(HSIZE, nullptr);
	const byte* ip = (byte*)in_data;
	byte* op = (byte*)out_data;
	const byte* in_end = ip + in_len;
	byte* out_end = op + out_len;
	const byte* ref;

	unsigned int hval = FRST(ip);
	unsigned int off;
	int          lit = 0;

	do {
		hval = NEXT(hval, ip);
		off = IDX(hval);
		ref = htab[off];
		htab[off] = ip;

		if (1
#if INIT_HTAB && !USE_MEMCPY
			&& ref < ip /* the next test will actually take care of this, but it is
						   faster */
#endif
			&& (off = unsigned(ip - ref - 1)) < MAX_OFF && ip + 4 < in_end &&
			ref > (byte*)in_data
#if STRICT_ALIGN
			&& ref[0] == ip[0] && ref[1] == ip[1] && ref[2] == ip[2]
#else
			&& *(word*)ref == *(word*)ip && ref[2] == ip[2]
#endif
			) {
			/* match found at *ref++ */
			unsigned int len = 2;
			unsigned int maxlen = unsigned(in_end - ip - len);
			maxlen = maxlen > MAX_REF ? MAX_REF : maxlen;

			do
				len++;
			while (len < maxlen && ref[len] == ip[len]);

			if (op + lit + 1 + 3 >= out_end)
				return 0;

			if (lit) {
				*op++ = byte(lit - 1);
				lit = -lit;
				do
					*op++ = ip[lit];
				while (++lit);
			}

			len -= 2;
			ip++;

			if (len < 7) {
				*op++ = byte((off >> 8) + (len << 5));
			} else {
				*op++ = byte((off >> 8) + (7 << 5));
				*op++ = byte(len - 7);
			}

			*op++ = byte(off);

#if ULTRA_FAST
			ip += len;
			hval = FRST(ip);
			hval = NEXT(hval, ip);
			htab[IDX(hval)] = ip;
			ip++;
#else
			do {
				hval = NEXT(hval, ip);
				htab[IDX(hval)] = ip;
				ip++;
			} while (len--);
#endif
		} else {
			/* one more literal byte we must copy */
			lit++;
			ip++;

			if (lit == MAX_LIT) {
				if (op + 1 + MAX_LIT >= out_end)
					return 0;

				*op++ = MAX_LIT - 1;
#if USE_MEMCPY
				memcpy(op, ip - MAX_LIT, MAX_LIT);
				op += MAX_LIT;
				lit = 0;
#else
				lit = -lit;
				do
					*op++ = ip[lit];
				while (++lit);
#endif
			}
		}
	} while (ip < in_end);

	if (lit) {
		if (op + lit + 1 >= out_end)
			return 0;

		*op++ = byte(lit - 1);
		lit = -lit;
		do {
			*op++ = ip[lit];
		} while (++lit);
	}

	return unsigned(op - (byte*)out_data);
}

unsigned int lzf_decompress(const void* const in_data, unsigned int in_len,
	void* out_data, unsigned int out_len) {
	byte const* ip = (byte*)in_data;
	byte* op = (byte*)out_data;
	byte const* const in_end = ip + in_len;
	byte* const       out_end = op + out_len;

	do {
		uintptr_t ctrl = *ip++;

		if (ctrl < (1 << 5)) /* literal run */
		{
			ctrl++;

			if (op + ctrl > out_end) {
				errno = E2BIG;
				return 0;
			}

#if USE_MEMCPY
			memcpy(op, ip, ctrl);
			op += ctrl;
			ip += ctrl;
#else
			do
				*op++ = *ip++;
			while (--ctrl);
#endif
		} else /* back reference */
		{
			uintptr_t len = ctrl >> 5;

			byte* ref = op - ((ctrl & 0x1f) << 8) - 1;

			if (len == 7)
				len += *ip++;

			ref -= *ip++;

			if (op + len + 2 > out_end) {
				errno = E2BIG;
				return 0;
			}

			if (ref < (byte*)out_data) {
				errno = EINVAL;
				return 0;
			}

			*op++ = *ref++;
			*op++ = *ref++;

			do
				*op++ = *ref++;
			while (--len);
		}
	} while (op < out_end && ip < in_end);

	return unsigned(op - (byte*)out_data);
}

}
