/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

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

/* Functions to discard MP3 tags -
 * written by O.Sezer <sezero@users.sourceforge.net>, put into public domain.
 */

/* Functions to parse some of MP3 tags -
 * written by V.Novichkov <admin@wohlnet.ru>, put into public domain.
 */

#include "SDL2/SDL_stdinc.h"
#include "SDL2/SDL_error.h"
#include "SDL2/SDL_rwops.h"

#include "mp3utils.h"

#include "SDL2/SDL_log.h"

/*********************** SDL_RW WITH BOOKKEEPING ************************/

int MP3_RWinit(struct mp3file_t *fil, SDL_RWops *src) {
    /* Don't use SDL_RWsize() here -- see SDL bug #5509 */
    fil->src = src;
    fil->start = SDL_RWtell(src);
    fil->length = SDL_RWseek(src, 0, RW_SEEK_END) - fil->start;
    fil->pos = 0;
    if (fil->start < 0 || fil->length < 0) {
        return SDL_Error(SDL_EFSEEK);
    }
    SDL_RWseek(src, fil->start, RW_SEEK_SET);
    return 0;
}

size_t MP3_RWread(struct mp3file_t *fil, void *ptr, size_t size, size_t maxnum) {
    size_t remaining = (size_t)(fil->length - fil->pos);
    size_t ret;
    maxnum *= size;
    if (maxnum > remaining) maxnum = remaining;
    ret = SDL_RWread(fil->src, ptr, 1, maxnum);
    fil->pos += (Sint64)ret;
    return ret;
}

Sint64 MP3_RWseek(struct mp3file_t *fil, Sint64 offset, int whence) {
    Sint64 ret;
    switch (whence) {
    case RW_SEEK_CUR:
        offset += fil->pos;
        break;
    case RW_SEEK_END:
        offset += fil->length;
        break;
    }
    if (offset < 0) return -1;
    if (offset > fil->length)
        offset = fil->length;
    ret = SDL_RWseek(fil->src, fil->start + offset, RW_SEEK_SET);
    if (ret < 0) return ret;
    fil->pos = offset;
    return offset;
}

Sint64 MP3_RWtell(struct mp3file_t *fil)
{
    return fil->pos;
}

static SDL_INLINE Sint32 read_sint24be(const Uint8 *data)
{
    Uint32 result = (Uint32)data[2];
    result |= (Uint32)data[1] << 8;
    result |= (Uint32)data[0] << 16;
    return (Sint32)result;
}

static SDL_INLINE Sint32 read_sint32be(const Uint8 *data)
{
    Uint32 result = (Uint32)data[3];
    result |= (Uint32)data[2] << 8;
    result |= (Uint32)data[1] << 16;
    result |= (Uint32)data[0] << 24;
    return (Sint32)result;
}

/* Parse ISO-8859-1 string and convert it into UTF-8 */
static char *parse_id3v1_ansi_string(const Uint8 *buffer, size_t src_len)
{
    char *src_buffer = (char*)SDL_malloc(src_len + 1);
    char *ret;
    if (!src_buffer) {
        return NULL; /* Out of memory */
    }
    SDL_memset(src_buffer, 0, src_len + 1);
    SDL_memcpy(src_buffer, buffer, src_len);
    ret = SDL_iconv_string("UTF-8", "ISO-8859-1", src_buffer, src_len + 1);
    SDL_free(src_buffer);
    return ret;
}

/********************************************************
 *                       ID3v2                          *
 ********************************************************/

#define ID3v2_BUFFER_SIZE               1024

#define ID3v2_HEADER_SIZE               10

#define ID3v2_FIELD_VERSION_MAJOR       3
#define ID3v2_FIELD_VERSION_MINOR       4
#define ID3v2_FIELD_HEAD_FLAGS          5
#define ID3v2_FIELD_TAG_LENGTH          6
#define ID3v2_FIELD_EXTRA_HEADER_LENGTH 10

#define ID3v2_FLAG_HAS_FOOTER           0x10
#define ID3v2_FLAG_HAS_EXTRA_HEAD       0x40

#define ID3v2_3_FRAME_HEADER_SIZE       10
#define ID3v2_2_FRAME_HEADER_SIZE       6
#define ID3v2_FIELD_FRAME_SIZE          4
#define ID3v2_FIELD_FRAME_SIZEv2        3
#define ID3v2_FIELD_FLAGS               8

static SDL_bool is_id3v2(const Uint8 *data, size_t length)
{
    /* ID3v2 header is 10 bytes:  http://id3.org/id3v2.4.0-structure */
    /* bytes 0-2: "ID3" identifier */
    if (length < ID3v2_HEADER_SIZE || SDL_memcmp(data, "ID3",3) != 0) {
        return SDL_FALSE;
    }
    /* bytes 3-4: version num (major,revision), each byte always less than 0xff. */
    if (data[3] == 0xff || data[4] == 0xff) {
        return SDL_FALSE;
    }
    /* bytes 6-9 are the ID3v2 tag size: a 32 bit 'synchsafe' integer, i.e. the
     * highest bit 7 in each byte zeroed.  i.e.: 7 bit information in each byte ->
     * effectively a 28 bit value.  */
    if (data[6] >= 0x80 || data[7] >= 0x80 || data[8] >= 0x80 || data[9] >= 0x80) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

static SDL_INLINE Sint32 id3v2_synchsafe_decode(const Uint8 *data)
{
    return ((data[0] << 21) + (data[1] << 14) + (data[2] << 7) + data[3]);
}

static long get_id3v2_len(const Uint8 *data, long length)
{
    /* size is a 'synchsafe' integer (see above) */
    long size = id3v2_synchsafe_decode(data + 6);
    size += ID3v2_HEADER_SIZE; /* header size */
    /* ID3v2 header[5] is flags (bits 4-7 only, 0-3 are zero).
     * bit 4 set: footer is present (a copy of the header but
     * with "3DI" as ident.)  */
    if (data[5] & 0x10) {
        size += ID3v2_HEADER_SIZE; /* footer size */
    }
    /* optional padding (always zeroes) */
    while (size < length && data[size] == 0) {
        ++size;
    }
    return size;
}

/* Decode a string in the frame according to an encoding marker */
static char *id3v2_decode_string(const Uint8 *string, size_t size)
{
    char *str_buffer = NULL;
    char *src_buffer = NULL;
    size_t copy_size = size;

    if (size == 0) {
        SDL_Log("id3v2_decode_string: Bad string size: a string should have at least 1 byte");
        return NULL;
    }

    if (size < 2) {
        return NULL;
    }

    if (string[0] == '\x01') { /* UTF-16 string with a BOM */
        if (size <= 5) {
            if (size < 5) {
                SDL_Log("id3v2_decode_string: Bad BOM-UTF16 string size: %u < 5", (unsigned int)size);
            }
            return NULL;
        }

        copy_size = size - 3 + 2; /* exclude 3 bytes of encoding hint, append 2 bytes for a NULL termination */
        src_buffer = (char*)SDL_malloc(copy_size);
        if (!src_buffer) {
            return NULL; /* Out of memory */
        }
        SDL_memset(src_buffer, 0, copy_size);
        SDL_memcpy(src_buffer, (string + 3), copy_size - 2);

        if (SDL_memcmp(string, "\x01\xFE\xFF", 3) == 0) { /* UTF-16BE*/
            str_buffer = SDL_iconv_string("UTF-8", "UCS-2BE", src_buffer, copy_size);
        } else if (SDL_memcmp(string, "\x01\xFF\xFE", 3) == 0) {  /* UTF-16LE*/
            str_buffer = SDL_iconv_string("UTF-8", "UCS-2LE", src_buffer, copy_size);
        }
        SDL_free(src_buffer);

    } else if (string[0] == '\x02') { /* UTF-16BEstring without a BOM */
        if (size <= 3) {
            if (size < 3) {
                SDL_Log("id3v2_decode_string: Bad UTF16BE string size: %u < 3", (unsigned int)size);
            }
            return NULL; /* Blank string*/
        }

        copy_size = size - 1 + 2; /* exclude 1 byte of encoding hint, append 2 bytes for a NULL termination */
        src_buffer = (char*)SDL_malloc(copy_size);
        if (!src_buffer) {
            return NULL; /* Out of memory */
        }
        SDL_memset(src_buffer, 0, copy_size);
        SDL_memcpy(src_buffer, (string + 1), copy_size - 2);

        str_buffer = SDL_iconv_string("UTF-8", "UCS-2BE", src_buffer, copy_size);
        SDL_free(src_buffer);

    } else if (string[0] == '\x03') { /* UTF-8 string */
        if (size <= 2) {
            return NULL; /* Blank string*/
        }
        str_buffer = (char*)SDL_malloc(size);
        if (!str_buffer) {
            return NULL; /* Out of memory */
        }
        SDL_strlcpy(str_buffer, (const char*)(string + 1), size);

    } else if (string[0] == '\x00') { /* Latin-1 string */
        if (size <= 2) {
            return NULL; /* Blank string*/
        }
        str_buffer = parse_id3v1_ansi_string((string + 1), size - 1);
    }

    return str_buffer;
}

/* Write a tag string into internal meta-tags storage */
static void write_id3v2_string(Mix_MusicMetaTags *out_tags, Mix_MusicMetaTag tag, const Uint8 *string, size_t size)
{
    char *str_buffer = id3v2_decode_string(string, size);

    if (str_buffer) {
        meta_tags_set(out_tags, tag, str_buffer);
        SDL_free(str_buffer);
    }

}

/* Identify a meta-key and decode the string (Note: input buffer should have at least 4 characters!) */
static void handle_id3v2_string(Mix_MusicMetaTags *out_tags, const char *key, const Uint8 *string, size_t size)
{
    if (SDL_memcmp(key, "TIT2", 4) == 0) {
        write_id3v2_string(out_tags, MIX_META_TITLE, string, size);
    } else if (SDL_memcmp(key, "TPE1", 4) == 0) {
        write_id3v2_string(out_tags, MIX_META_ARTIST, string, size);
    } else if (SDL_memcmp(key, "TALB", 4) == 0) {
        write_id3v2_string(out_tags, MIX_META_ALBUM, string, size);
    } else if (SDL_memcmp(key, "TCOP", 4) == 0) {
        write_id3v2_string(out_tags, MIX_META_COPYRIGHT, string, size);
    }
/* TODO: Extract "Copyright message" from TXXX value: a KEY=VALUE string divided by a zero byte:*/
/*
    else if (SDL_memcmp(key, "TXXX", 4) == 0) {
        write_id3v2_string(out_tags, MIX_META_COPYRIGHT, string, size);
    }
*/
}

/* Identify a meta-key and decode the string (Note: input buffer should have at least 4 characters!) */
static void handle_id3v2x2_string(Mix_MusicMetaTags *out_tags, const char *key, const Uint8 *string, size_t size)
{
    if (SDL_memcmp(key, "TT2", 3) == 0) {
        write_id3v2_string(out_tags, MIX_META_TITLE, string, size);
    } else if (SDL_memcmp(key, "TP1", 3) == 0) {
        write_id3v2_string(out_tags, MIX_META_ARTIST, string, size);
    } else if (SDL_memcmp(key, "TAL", 3) == 0) {
        write_id3v2_string(out_tags, MIX_META_ALBUM, string, size);
    } else if (SDL_memcmp(key, "TCR", 3) == 0) {
        write_id3v2_string(out_tags, MIX_META_COPYRIGHT, string, size);
    }
}

/* Parse a frame in ID3v2.2 format */
static size_t id3v22_parse_frame(Mix_MusicMetaTags *out_tags, struct mp3file_t *src, Uint8 *buffer)
{
    size_t size;
    char key[4];
    size_t read_size;
    Sint64 frame_begin = MP3_RWtell(src);

    read_size = MP3_RWread(src, buffer, 1, ID3v2_2_FRAME_HEADER_SIZE);

    if (read_size < ID3v2_2_FRAME_HEADER_SIZE) {
        SDL_Log("id3v22_parse_frame (1): Unexpected end of the file while frame header reading (had to read %u bytes, %u bytes wanted)",
                (unsigned int)read_size,
                (unsigned int)ID3v2_2_FRAME_HEADER_SIZE);
        MP3_RWseek(src, frame_begin, RW_SEEK_SET);
        return 0; /* Buffer size that left is too small */
    }

    if (SDL_memcmp(buffer, "\0\0\0", 3) == 0) {
        MP3_RWseek(src, frame_begin, RW_SEEK_SET);
        return 0;
    }

    SDL_memcpy(key, buffer, 3); /* Tag title (key) */

    size = (size_t)read_sint24be(buffer + ID3v2_FIELD_FRAME_SIZEv2);

    if (size < ID3v2_BUFFER_SIZE) {
        read_size = MP3_RWread(src, buffer, 1, size);
        if (read_size < size) {
            SDL_Log("id3v22_parse_frame (2): Unexpected end of the file while frame data reading (had to read %u bytes, %u bytes wanted)",
                    (unsigned int)read_size,
                    (unsigned int)size);
            MP3_RWseek(src, frame_begin, RW_SEEK_SET);
            return 0; /* Can't read frame data, possibly, a file size was reached */
        }
    } else {
        read_size = MP3_RWread(src, buffer, 1, ID3v2_BUFFER_SIZE);
        if (read_size < ID3v2_BUFFER_SIZE) {
            SDL_Log("id3v22_parse_frame (3): Unexpected end of the file while frame data reading (had to read %u bytes, %u bytes wanted)",
                    (unsigned int)read_size,
                    (unsigned int)ID3v2_BUFFER_SIZE);
            MP3_RWseek(src, frame_begin, RW_SEEK_SET);
            return 0; /* Can't read frame data, possibly, a file size was reached */
        }
        MP3_RWseek(src, frame_begin + (Sint64)size, RW_SEEK_SET);
    }

    handle_id3v2x2_string(out_tags, key, buffer, read_size);

    return (size_t)(size + ID3v2_2_FRAME_HEADER_SIZE); /* data size + size of the header */
}

/* Parse a frame in ID3v2.3 and ID3v2.4 formats */
static size_t id3v2x_parse_frame(Mix_MusicMetaTags *out_tags, struct mp3file_t *src, Uint8 *buffer, Uint8 version)
{
    Uint32 size;
    char key[4];
    Uint8 flags[2];
    size_t read_size;
    Sint64 frame_begin = MP3_RWtell(src);

    read_size = MP3_RWread(src, buffer, 1, ID3v2_3_FRAME_HEADER_SIZE);

    if (read_size < ID3v2_3_FRAME_HEADER_SIZE) {
        SDL_Log("id3v2x_parse_frame (1): Unexpected end of the file while frame header reading (had to read %u bytes, %u bytes wanted)",
                (unsigned int)read_size,
                (unsigned int)ID3v2_3_FRAME_HEADER_SIZE);
        MP3_RWseek(src, frame_begin, RW_SEEK_SET);
        return 0; /* Can't read frame header, possibly, a file size was reached */
    }

    if (SDL_memcmp(buffer, "\0\0\0\0", 4) == 0) {
        MP3_RWseek(src, frame_begin, RW_SEEK_SET);
        return 0;
    }

    SDL_memcpy(key, buffer, 4); /* Tag title (key) */

    if (version == 4) {
        size = (Uint32)id3v2_synchsafe_decode(buffer + ID3v2_FIELD_FRAME_SIZE);
    } else {
        size = (Uint32)read_sint32be(buffer + ID3v2_FIELD_FRAME_SIZE);
    }

    SDL_memcpy(flags, buffer + ID3v2_FIELD_FLAGS, 2);

    if (size < ID3v2_BUFFER_SIZE) {
        read_size = MP3_RWread(src, buffer, 1, size);
        if (read_size < size) {
            SDL_Log("id3v2x_parse_frame (2): Unexpected end of the file while frame data reading (had to read %u bytes, %u bytes wanted)",
                    (unsigned int)read_size,
                    (unsigned int)size);
            MP3_RWseek(src, frame_begin, RW_SEEK_SET);
            return 0; /* Can't read frame data, possibly, a file size was reached */
        }
    } else {
        read_size = MP3_RWread(src, buffer, 1, ID3v2_BUFFER_SIZE);
        if (read_size < ID3v2_BUFFER_SIZE) {
            SDL_Log("id3v2x_parse_frame (3): Unexpected end of the file while frame data reading (had to read %u bytes, %u bytes wanted)",
                    (unsigned int)read_size,
                    (unsigned int)ID3v2_BUFFER_SIZE);
            MP3_RWseek(src, frame_begin, RW_SEEK_SET);
            return 0; /* Can't read frame data, possibly, a file size was reached */
        }
        MP3_RWseek(src, frame_begin + (Sint64)size, RW_SEEK_SET);
    }

    handle_id3v2_string(out_tags, key, buffer, size);

    return (size_t)(size + ID3v2_3_FRAME_HEADER_SIZE); /* data size + size of the header */
}


/* Parse content of ID3v2 */
static SDL_bool parse_id3v2(Mix_MusicMetaTags *out_tags, struct mp3file_t *src)
{
    Uint8 version_major, flags;
    long total_length, tag_len, tag_extended_len = 0;
    Uint8 buffer[ID3v2_BUFFER_SIZE];
    size_t read_size;
    size_t frame_length;
    Sint64 file_size;

    total_length = 0;

    file_size = src->length;
    MP3_RWseek(src, 0, RW_SEEK_SET);
    read_size = MP3_RWread(src, buffer, 1, ID3v2_HEADER_SIZE); /* Retrieve the header */
    if (read_size < ID3v2_HEADER_SIZE) {
        SDL_Log("parse_id3v2: fail to read a header (%u < 10)", (unsigned int)read_size);
        return SDL_FALSE; /* Unsupported version of the tag */
    }

    total_length += ID3v2_HEADER_SIZE;

    version_major = buffer[ID3v2_FIELD_VERSION_MAJOR]; /* Major version */
    /* version_minor = buffer[ID3v2_VERSION_MINOR]; * Minor version, UNUSED */
    flags = buffer[ID3v2_FIELD_HEAD_FLAGS]; /* Flags */
    tag_len = id3v2_synchsafe_decode(buffer + ID3v2_FIELD_TAG_LENGTH); /* Length of a tag */

    if (version_major != 2 && version_major != 3 && version_major != 4) {
        SDL_Log("parse_id3v2: Unsupported version %d", version_major);
        return SDL_FALSE; /* Unsupported version of the tag */
    }

    if ((version_major > 2) && ((flags & ID3v2_FLAG_HAS_EXTRA_HEAD) == ID3v2_FLAG_HAS_EXTRA_HEAD)) {
        MP3_RWread(src, buffer + ID3v2_FIELD_EXTRA_HEADER_LENGTH, 1, 4);
        MP3_RWseek(src, -4, RW_SEEK_CUR);
        tag_extended_len = id3v2_synchsafe_decode(buffer + ID3v2_FIELD_EXTRA_HEADER_LENGTH); /* Length of an extended header */
    }

    if (tag_extended_len) {
        tag_len -= tag_extended_len; /* Subtract the size of extended header */
        MP3_RWseek(src, tag_extended_len, RW_SEEK_CUR); /* Skip extended header and it's size value */
    }

    total_length += tag_len;

    if (flags & ID3v2_FLAG_HAS_FOOTER) {
        total_length += ID3v2_HEADER_SIZE; /* footer size */
    }

    if ((MP3_RWtell(src) + tag_len) > file_size) {
        SDL_Log("parse_id3v2: Tag size bigger than actual file size");
        return SDL_FALSE; /* Tag size is bigger than actual buffer data */
    }

    while ((MP3_RWtell(src) >= 0) && (MP3_RWtell(src) < (total_length))) {
        if (version_major == 2) {
            frame_length = id3v22_parse_frame(out_tags, src, buffer);
        } else {
            frame_length = id3v2x_parse_frame(out_tags, src, buffer, version_major);
        }
        if (!frame_length) {
            break;
        }
    }

    return SDL_TRUE;
}

int read_id3v2_from_mem(Mix_MusicMetaTags *out_tags, Uint8 *data, size_t length)
{
    SDL_RWops *src = SDL_RWFromConstMem(data, (int)length);
    SDL_bool is_valid;
    struct mp3file_t fil;

    if (src) {
        fil.src = src;
        fil.start = 0;
        fil.length = (Sint64)length;
        fil.pos = 0;

        if (!is_id3v2(data, length)) {
            SDL_RWclose(src);
            return -1;
        }

        if (get_id3v2_len(data, (long)length) > (long)length) {
            SDL_RWclose(src);
            return -1;
        }

        is_valid = parse_id3v2(out_tags, &fil);
        SDL_RWclose(src);

        return is_valid ? 0 : -1;
    }
    return -1;
}
