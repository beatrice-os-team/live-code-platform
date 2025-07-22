/*
  Losu Copyright Notice
  --------------------
    Losu is an open source programming language project under the MIT license
  that can be used for both academic and commercial purposes. There are no
  fees, no royalties, and no GNU-like restrictions. Losu qualifies
  as open source software. However, Losu is not public property, and founder
  'chen-chaochen' retains its copyright.

    Losu has been registered with the National Copyright Administration of the
  People's Republic of China, and adopts the MIT license as the copyright
  licensing contract under which the right holder conditionally licenses its
  reproduction, distribution, and modification rights to an unspecified public.

    If you use Losu, please follow the public MIT agreement or choose to enter
  into a dedicated license agreement with us.

  The MIT LICENSE is as follows
  --------------------
  Copyright  2020  chen-chaochen

    Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the “Software”), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef FILE_SRC_SYNTAX_LEXER_C
#define FILE_SRC_SYNTAX_LEXER_C

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "losu_bytecode.h"
#include "losu_mem.h"
#include "losu_object.h"
#include "token.h"

const losu_syntax_keyword_t losu_syntax_keyword[] = {
    /** logic */
    {"and", 0, TOKEN_AND},
    {"or", 0, TOKEN_OR},
    {"not", 0, TOKEN_NOT},
    // {"True", 0, TOKEN_TRUE},
    {"true", 0, TOKEN_TRUE},
    // {"False", 0, TOKEN_FALSE},
    {"false", 0, TOKEN_FALSE},
    // {"null", 0, TOKEN_FALSE},
    /** pass */
    {"pass", -1, TOKEN_PASS},
    /** if */
    {"if", 1, TOKEN_IF},
    {"else", 1, TOKEN_ELSE},
    {"elif", 1, TOKEN_ELSEIF},
    /** def */
    {"def", 1, TOKEN_DEF},
    {"lambda", 0, TOKEN_LAMBDA},
    {"...", 0, TOKEN_ARG},
    /** let global  */
    {"let", 0, TOKEN_LET},
    {"global", 0, TOKEN_GLOBAL},
    {"class", 1, TOKEN_CLASS},
    // {"module", 1, TOKEN_MODULE},
    /** return */
    {"return", 0, TOKEN_RETURN},
    {"break", 0, TOKEN_BREAK},
    {"continue", 0, TOKEN_CONTINUE},
    {"yield", 0, TOKEN_YIELD},
    /** while until for */
    {"while", 1, TOKEN_WHILE},
    {"until", 1, TOKEN_UNTIL},
    {"for", 1, TOKEN_FOR},
    /** import */
    {"import", 0, TOKEN_IMPORT},
    {"async", 0, TOKEN_ASYNC},
    /* match case */
    {"match", 1, TOKEN_MATCH},
    {"case", 1, TOKEN_CASE},
    /* try except */
    // {"assert", 1, TOKEN_ASSERT},
    {"except", 1, TOKEN_EXCEPT},
    {"raise", 0, TOKEN_RAISE},
    /** alu */
    {"==", 0, TOKEN_EQ},
    {">=", 0, TOKEN_GE},
    {"<=", 0, TOKEN_LE},
    {"!=", 0, TOKEN_NE},
    {"::", 0, TOKEN_ASSIGN},
    {"**", 0, TOKEN_POW},
    {"|>", 0, TOKEN_PIPE},
    /* other */
    {"[NAME]", 0, TOKEN_NAME},
    {"[NUMBER]", 0, TOKEN_NUMBER},
    {"[STRING]", 0, TOKEN_STRING},
    {"[EOZ]", 0, TOKEN_EOZ},
};

/**
 * @brief init lexer
 * @param lex losu lexer handle
 */
void losu_syntax_lex_init(losu_syntax_lex_t lex) {
  losu_vm_t vm = lex->vm;
  for (size_t i = 0;
       i < sizeof(losu_syntax_keyword) / sizeof(losu_syntax_keyword[0]); i++) {
    losu_object_string_t s = losu_objstring_new(vm, losu_syntax_keyword[i].str);
    s->marked = losu_syntax_keyword[i].token;
  }
}

/**
 * @brief lexer next
 * @param lex lexer handle
 * @param tk   token value
 * @return  token type
 */
int16_t losu_syntax_lex_next(losu_syntax_lex_t lex, losu_syntax_tkvalue_t tk) {
  if (lex->idt.tmp > 0) {
    lex->idt.tmp--;
    lex->idt.nowidt--;
    return TOKEN_PASS;
  }
  while (1) {
    switch (lex->current) {
      /* ignore */
      // case ';':
      case '\r':
      case '\t':
      case ' ':
      case '\0': {
        next(lex);
        break;
      }

      /* comment */
      case '#': {
        while (lex->current != '\n') {
          next(lex);
          if (lex->current == EOF)
            return TOKEN_EOZ;
        }
        break;
      }

      case ':': {
        next(lex);
        if (lex->current == ':') {
          next(lex);
          return TOKEN_ASSIGN;
        }
        if (lex->current == '>') {
          next(lex);
          return TOKEN_PIPE;
        }
        if (lex->current == '<') {
          while (1) {
            next(lex);
            if (lex->current == '\'' || lex->current == '"')
              break;
            if (lex->current == '\n' || lex->current == '\r' ||
                lex->current == EOF)
              losu_syntax_error(lex, "invalid syntax of ':<'");
          }
          losu_syntax_tkvalue tmp;
          readstr(lex, lex->current, &tmp);
          while (lex->current != '>') {
            next(lex);
            if (lex->current == '\n' || lex->current == '\r' ||
                lex->current == EOF)
              losu_syntax_error(lex, "invalid syntax of ':<'");
          }
          next(lex);
          break;
        }
        return ':';
      }
      /* next line & EOZ */
      case '\n': {
        next(lex);
        incline(lex);
        /* check idt */
        lex->idt.read = 0;
        losu_ctype_bool i = 0;
        while (1) {
          switch (lex->current) {
            case '\n': {
              lex->idt.read = 0;
              incline(lex);
              break;
            }
            case ' ': {
              lex->idt.read++;
              break;
            }
            case '\t': {
              lex->idt.read += lex->idt.size;
              break;
            }
            default: {
              i = 1;
              if (lex->idt.read < lex->idt.nowidt * lex->idt.size) {
                if (lex->idt.read % lex->idt.size != 0)
                  losu_syntax_error(lex, "invalid identation level '%d' ",
                                    lex->idt.read);
                lex->idt.tmp = lex->idt.nowidt - lex->idt.read / lex->idt.size;
                if (lex->idt.tmp > 0) {
                  lex->idt.tmp--;
                  lex->idt.nowidt--;
                  return TOKEN_PASS;
                }
              }
              break;
            }
          }
          if (i)
            break;
          next(lex);
        }
        break;
      }
      case EOF: {
        if (lex->idt.nowidt) {
          lex->idt.tmp = lex->idt.nowidt;
          lex->idt.tmp--;
          lex->idt.nowidt--;
          return TOKEN_PASS;
        }
        return TOKEN_EOZ;
      }

      /* embed */
      case '`': {
        readstr(lex, lex->current, tk);
        // next(lex);
        const char* pwd = lex->vm->path ? lex->vm->path : "./";
        size_t filename_len = tk->s->len + strlen(pwd) + 1;
        char* filename = losu_mem_malloc(lex->vm, filename_len);
        memset(filename, 0, filename_len);
        snprintf(filename, filename_len, "%s%s", pwd, tk->s->str);
        FILE* fp = fopen(filename, "rb");
        if (!fp) {
          losu_syntax_error(lex, "cannot embed file %s", filename);
          exit(1);
        } else {
          fseek(fp, 0, SEEK_END);
          uint64_t flen = ftell(fp);
          if (flen >= _losu_vmlim_maxstrlen)
            losu_syntax_error(lex, "file %s is too large", filename);
          char* tmp = losu_mem_malloc(lex->vm, flen + 1);
          fseek(fp, 0, SEEK_SET);
          fread(tmp, 1, flen, fp);
          tk->s = losu_objstring_newlen(lex->vm, tmp, flen);
          fclose(fp);
          losu_mem_free(lex->vm, tmp);
          losu_mem_free(lex->vm, filename);
        }
        return TOKEN_STRING;
      }

      // case '@': {
      //   next(lex);
      //   losu_object_string_t ts = losu_objstring_new(lex->vm,
      //   readname(lex)); if (strcmp(ts->str, "embed") == 0) { if
      //   (lex->current == '(') {
      //     next(lex);
      //     if (lex->current == '\'' || lex->current == '\"') {
      //       readstr(lex, lex->current, tk);
      //       if (lex->current == ')') {
      //         next(lex);
      //         const char* pwd = lex->vm->path ? lex->vm->path : "./";
      //         size_t filename_len = tk->s->len + strlen(pwd) + 1;
      //         char* filename = losu_mem_malloc(lex->vm, filename_len);
      //         memset(filename, 0, filename_len);
      //         snprintf(filename, filename_len, "%s%s", pwd, tk->s->str);
      //         FILE* fp = fopen(filename, "rb");
      //         if (!fp) {
      //           losu_syntax_error(lex, "cannot embed file %s", filename);
      //           exit(1);
      //         } else {
      //           fseek(fp, 0, SEEK_END);
      //           long flen = ftell(fp);
      //           char* tmp = losu_mem_malloc(lex->vm, flen + 1);
      //           fseek(fp, 0, SEEK_SET);
      //           fread(tmp, 1, flen, fp);
      //           tk->s = losu_objstring_newlen(lex->vm, tmp, flen);
      //           fclose(fp);
      //           losu_mem_free(lex->vm, tmp);
      //         }
      //         return TOKEN_STRING;
      //       }
      //     }
      //   }
      //   losu_syntax_error(lex, "invalid '@embed(filename)'");
      //   }
      //   break;
      // }
      /* string */
      case '\'':
      case '\"': {
        readstr(lex, lex->current, tk);
        return TOKEN_STRING;
      }

      case '|': {
        next(lex);
        if (lex->current == '>') {
          next(lex);
          return TOKEN_PIPE;
        }
        return '|';
      }
      case '<': {
        next(lex);
        if (lex->current == '=') {
          next(lex);
          return TOKEN_LE;
        }
        return '<';
      }
      case '>': {
        next(lex);
        if (lex->current == '=') {
          next(lex);
          return TOKEN_GE;
        }
        return '>';
      }

      case '=': {
        next(lex);
        if (lex->current == '=') {
          next(lex);
          return TOKEN_EQ;
        }

        return '=';
      }
      case '!': {
        next(lex);
        if (lex->current == '=') {
          next(lex);
          return TOKEN_NE;
        }
        return '!';
      }
      case '*': {
        next(lex);
        if (lex->current == '*') {
          next(lex);
          return TOKEN_POW;
        }
        return '*';
      }

      case '.': {
        next(lex);
        if (lex->current == '.') {
          next(lex);
          if (lex->current == '.') {
            next(lex);
            return TOKEN_ARG;
          }
          losu_syntax_error(lex, "invalid token '..'");
        }
        if (isdigit(lex->current)) {
          readnum(lex, tk, 1);
          return TOKEN_NUMBER;
        }
        return '.';
      }

      /* number */
      case '0': {
        /* 0x (hex) 0b(binary) 0o (octal) 0.(float) uint32_t*/
        next(lex);
        if (lex->current == 'x') {
          next(lex);
          uint32_t intnum = 0;
          for (uint8_t i = 0; i < 8; i++) {
            if (!isxdigit(lex->current))
              break;
            intnum = (intnum << 4) | hex2int(lex->current);
            next(lex);
          }
          tk->num = intnum;
        } else if (lex->current == 'b') {
          next(lex);
          uint32_t intnum = 0;
          for (uint8_t i = 0; i < 32; i++) {
            if (lex->current != '0' && lex->current != '1')
              break;
            intnum = (intnum << 1) | (uint32_t)(lex->current == '1');
            next(lex);
          }
          tk->num = intnum;
        } else if (lex->current == 'a') {
          next(lex);
          uint8_t c = 0;
          if (!isalpha(lex->current))
            losu_syntax_error(lex, "invalid format '0a'");
          c = lex->current;
          tk->num = c;
          next(lex);
        } else if (lex->current == 'o') {
          /* octal */
          next(lex);
          uint32_t intnum = 0;
          for (uint8_t i = 0; i < 10; i++) {
            if (!(lex->current >= '0' && lex->current < '8'))
              break;
            intnum = (intnum << 3) | (uint32_t)(lex->current - '0');
            next(lex);
          }
          tk->num = intnum;
        } else if (lex->current == '.') {
          next(lex);
          readnum(lex, tk, 1);
        } else {
          tk->num = 0;
        }
        return TOKEN_NUMBER;
      }
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        readnum(lex, tk, 0);
        return TOKEN_NUMBER;
      }

      default: {
        /* ASCII */
        if (!isalpha(lex->current) && lex->current != '_' &&
            lex->current < 0x80) {
          int16_t c = lex->current;
          next(lex);
          return c;
        }
        /* keyword or name */
        losu_object_string_t ts = losu_objstring_new(lex->vm, readname(lex));
        if (ts->marked > 0xff) {
          /* keyword */
          for (int32_t i = 0;
               i < sizeof(losu_syntax_keyword) / sizeof(losu_syntax_keyword_t);
               i++) {
            if (losu_syntax_keyword[i].token == ts->marked) {
              lex->idt.nowidt += losu_syntax_keyword[i].idt;
              break;
            }
          }
          return ts->marked;
        }
        tk->s = ts;
        return TOKEN_NAME;
      }
    }
  }
  return TOKEN_EOZ;
}

static void checkbuffer(losu_vm_t vm, size_t n, size_t len) {
  size_t nlen = n + len;
  if (nlen <= vm->nbufftmp)
    return;
  losu_mem_reallocvector(vm, vm->bufftmp, nlen, unsigned char);
  vm->nblocks += (nlen - vm->nbufftmp) * sizeof(unsigned char);
  vm->nbufftmp = nlen;
}

static uint8_t __utf8len(uint8_t c) {
  if (c < 0x80)
    return 1;
  else if (c < 0xE0)
    return 2;
  else if (c < 0xF0)
    return 3;
  else if (c < 0xF8)
    return 4;
  else if (c < 0xFC)
    return 5;
  else if (c < 0xFE)
    return 6;
  else
    return 0;
}

static int __unicode2utf8(uint32_t unicode, char* buff) {
  if (unicode < 0x80) {
    *buff++ = unicode;
    return 1;
  } else if (unicode < 0x800) {
    *buff++ = 0xC0 | (unicode >> 6);
    *buff++ = 0x80 | (unicode & 0x3F);
    return 2;
  } else if (unicode < 0x10000) {
    *buff++ = 0xE0 | (unicode >> 12);
    *buff++ = 0x80 | ((unicode >> 6) & 0x3F);
    *buff++ = 0x80 | (unicode & 0x3F);
    return 3;
  } else {
    *buff++ = 0xF0 | (unicode >> 18);
    *buff++ = 0x80 | ((unicode >> 12) & 0x3F);
    *buff++ = 0x80 | ((unicode >> 6) & 0x3F);
    *buff++ = 0x80 | (unicode & 0x3F);
    return 4;
  }
}

static const char* readname(losu_syntax_lex_t lex) {
  losu_vm_t vm = lex->vm;
  size_t l = 0;
  checkbuffer(vm, l, 32);
  do {
    uint8_t len = __utf8len((uint8_t)lex->current);
    checkbuffer(vm, l, len);
    for (uint8_t i = 0; i < len; i++) {
      save_next(vm, lex, l);
    }
  } while (isalnum((unsigned char)lex->current) || lex->current == '_' ||
           __utf8len(lex->current) > 1);
  save(vm, '\0', l);
  return (const char*)vm->bufftmp;
}

static void readnum(losu_syntax_lex_t lex,
                    losu_syntax_tkvalue_t tkv,
                    losu_ctype_bool dot) {
  losu_vm_t vm = lex->vm;
  size_t l = 0;
  checkbuffer(vm, l, 32);
  if (dot)
    save(vm, '.', l);
  while (isdigit(lex->current)) {
    checkbuffer(vm, l, 32);
    save_next(vm, lex, l);
  }
  if (lex->current == '.')
    save_next(vm, lex, l);
  while (isdigit(lex->current)) {
    checkbuffer(vm, l, 32);
    save_next(vm, lex, l);
  }
  if (lex->current == 'e' || lex->current == 'E') {
    save_next(vm, lex, l);
    if (lex->current == '+' || lex->current == '-')
      save_next(vm, lex, l);
    while (isdigit(lex->current)) {
      checkbuffer(vm, l, 32);
      save_next(vm, lex, l);
    }
  }
  save(vm, '\0', l);
  if (!losu_object_str2num(vm, (const char*)vm->bufftmp, &tkv->num))
    losu_syntax_error(lex, "invalid syntax '%s' ", (const char*)vm->bufftmp);
}

static void readstr(losu_syntax_lex_t lex,
                    uint8_t del,
                    losu_syntax_tkvalue_t tkv) {
  losu_vm_t vm = lex->vm;
  size_t l = 0;
  checkbuffer(vm, l, 32);
  save_next(vm, lex, l);
  while (lex->current != del) {
    if (lex->current == '\n' || lex->current == EOF)
      losu_syntax_error(lex, "invalid syntax '%.20s\n...' ",
                        (const char*)vm->bufftmp);
    checkbuffer(vm, l, 32);
    if (lex->current == '\\') {
      next(lex);
      switch (lex->current) {
        case '\0': {
          save(vm, '\0', l);
          next(lex);
          break;
        }
        case '"': {
          save(vm, '\"', l);
          next(lex);
          break;
        }
        case '\\': {
          save(vm, '\\', l);
          next(lex);
          break;
        }
        case '/': {
          save(vm, '/', l);
          next(lex);
          break;
        }
        case 'b': {
          save(vm, '\b', l);
          next(lex);
          break;
        }
        case 'f': {
          save(vm, '\f', l);
          next(lex);
          break;
        }
        case 'n': {
          save(vm, '\n', l);
          next(lex);
          break;
        }
        case 'r': {
          save(vm, '\r', l);
          next(lex);
          break;
        }
        case 't': {
          save(vm, '\t', l);
          next(lex);
          break;
        }
        case 'u': {
          char hex[5] = {0};
          for (uint8_t i = 0; i < 4; i++) {
            next(lex);
            hex[i] = lex->current;
            if (!isxdigit(lex->current)) {
              losu_syntax_error(lex, "invalid unicode '\\u%s' ", hex);
              break;
            }
          }
          next(lex);

          uint32_t unicode = 0;
          sscanf(hex, "%x", &unicode);
          if (unicode > 0x10FFFF)
            losu_syntax_error(lex, "invalid unicode '\\u%s' ", hex);
          char buff[5] = {0};
          int _len = __unicode2utf8(unicode, buff);
          checkbuffer(vm, l, 32);
          for (uint8_t i = 0; i < _len; i++) {
            // if (buff[i] != '\0')
            save(vm, buff[i], l);
          }

          break;
        }
        default: {
          losu_syntax_error(lex, "invalid syntax '\\%c' ", lex->current);
          break;
        }
      }
      continue;
    }
    save_next(vm, lex, l);
  }
  save_next(vm, lex, l);
  save(vm, '\0', l);
  if (l - 3 > _losu_vmlim_maxstrlen - 1)
    losu_syntax_error(lex, "string is too long");
  tkv->s = losu_objstring_newlen(vm, (const char*)vm->bufftmp + 1, l - 3);
}

static uint32_t hex2int(char c) {
  // hex char to int
  if ('0' <= c && c <= '9')
    return c - '0';
  if ('a' <= c && c <= 'f')
    return c - 'a' + 10;
  if ('A' <= c && c <= 'F')
    return c - 'A' + 10;
  return 0;
}

#endif
