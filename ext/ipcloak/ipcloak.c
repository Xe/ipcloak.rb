/*
 * Charybdis: an advanced ircd
 * ip_cloaking.c: provide user hostname cloaking
 *
 * Written originally by nenolod, altered to use FNV by Elizabeth in 2008
 * Altered to be a ruby module by Xe in 2017
 */

#include <ctype.h>
#include <ruby.h>

#define HOSTLEN 50

static void do_host_cloak_ip(const char *inbuf, char *outbuf);
static void do_host_cloak_host(const char *inbuf, char *outbuf);

VALUE Ipcloak = Qnil;

void Init_ipcloak();
VALUE method_ipcloak_ip(VALUE self, VALUE ipaddr);
VALUE method_ipcloak_host(VALUE self, VALUE host);

void Init_ipcloak() {
  Ipcloak = rb_define_module("IPCloak");
  rb_define_singleton_method(Ipcloak, "ip", method_ipcloak_ip, 1);
  rb_define_singleton_method(Ipcloak, "host", method_ipcloak_host, 1);
}

/* Magic value for FNV hash functions */
#define FNV1_32_INIT 0x811c9dc5UL

u_int32_t
fnv_hash(const unsigned char *s, int bits)
{
  u_int32_t h = FNV1_32_INIT;

  while (*s) {
    h ^= *s++;
    h += (h<<1) + (h<<4) + (h<<7) + (h << 8) + (h << 24);
  }
  if (bits < 32)
    h = ((h >> bits) ^ h) & ((1<<bits)-1);
  return h;
}

static void
do_host_cloak_ip(const char *inbuf, char *outbuf)
{
    /* None of the characters in this table can be valid in an IP */
    char chartable[] = "ghijklmnopqrstuvwxyz";
    char *tptr;
    uint32_t accum = fnv_hash((const unsigned char*) inbuf, 32);
    int sepcount = 0;
    int totalcount = 0;
    int ipv6 = 0;

    strlcpy(outbuf, inbuf, HOSTLEN + 1);

    if (strchr(outbuf, ':')) {
        ipv6 = 1;

        /* Damn you IPv6...
         * We count the number of colons so we can calculate how much
         * of the host to cloak. This is because some hostmasks may not
         * have as many octets as we'd like.
         *
         * We have to do this ahead of time because doing this during
         * the actual cloaking would get ugly
         */
        for (tptr = outbuf; *tptr != '\0'; tptr++)
            if (*tptr == ':')
                totalcount++;
    } else if (!strchr(outbuf, '.'))
        return;

    for (tptr = outbuf; *tptr != '\0'; tptr++) {
        if (*tptr == ':' || *tptr == '.') {
            sepcount++;
            continue;
        }

        if (ipv6 && sepcount < totalcount / 2)
            continue;

        if (!ipv6 && sepcount < 2)
            continue;

        *tptr = chartable[(*tptr + accum) % 20];
        accum = (accum << 1) | (accum >> 31);
    }
}

static void
do_host_cloak_host(const char *inbuf, char *outbuf)
{
    char b26_alphabet[] = "abcdefghijklmnopqrstuvwxyz";
    char *tptr;
    uint32_t accum = fnv_hash((const unsigned char*) inbuf, 32);

    strlcpy(outbuf, inbuf, HOSTLEN + 1);

    /* pass 1: scramble first section of hostname using base26
     * alphabet toasted against the FNV hash of the string.
     *
     * numbers are not changed at this time, only letters.
     */
    for (tptr = outbuf; *tptr != '\0'; tptr++) {
        if (*tptr == '.')
            break;

        if (isdigit(*tptr) || *tptr == '-')
            continue;

        *tptr = b26_alphabet[(*tptr + accum) % 26];

        /* Rotate one bit to avoid all digits being turned odd or even */
        accum = (accum << 1) | (accum >> 31);
    }

    /* pass 2: scramble each number in the address */
    for (tptr = outbuf; *tptr != '\0'; tptr++) {
        if (isdigit(*tptr))
            *tptr = '0' + (*tptr + accum) % 10;

        accum = (accum << 1) | (accum >> 31);
    }
}

VALUE method_ipcloak_ip(VALUE self, VALUE ip) {
  char *data;
  char res[HOSTLEN];

  data = StringValueCStr(ip);
  if (data == NULL) {
    rb_raise(rb_eTypeError, "argument must be a string");
    return Qnil;
  }

  do_host_cloak_ip(data, res);

  return rb_str_new2(res);
}

VALUE method_ipcloak_host(VALUE self, VALUE host) {
  char *data;
  char res[HOSTLEN];

  data = StringValueCStr(host);
  if (data == NULL) {
    rb_raise(rb_eTypeError, "argument must be a string");
    return Qnil;
  }

  do_host_cloak_host(data, res);

  return rb_str_new2(res);
}
