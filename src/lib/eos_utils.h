#ifndef __EOS_UTILS_H__
#define __EOS_UTILS_H__

#include <stdbool.h>
#include <stdint.h>

unsigned char check_canonical(uint8_t *rs);

int ecdsa_der_to_sig(const uint8_t *der, uint8_t *sig);

void rng_rfc6979(unsigned char *rnd,
                 unsigned char *h1,
                 unsigned char *x, unsigned int x_len,
                 const unsigned char *q, unsigned int q_len,
                 unsigned char *V, unsigned char *K);

#endif
