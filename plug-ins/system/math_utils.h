#ifndef MATH_UTILS_H
#define MATH_UTILS_H

float linear_interpolation (float x, float x1, float x2, float y1, float y2);

/** Generate random alnum string of given length. Uses rand(); cosmetic tokens only. */
std::string create_nonce(int len);

/** CSPRNG-seeded code from an unambiguous alphabet (no 0 O 1 I L), for identity-gating codes. */
std::string create_secure_nonce(int len);

/** CSPRNG-seeded decimal digits, for emailed one-time codes. */
std::string create_secure_digits(int len);


/** Calculate average value of throwing a die. */
int dice_ave(int d1, int d2);

/** Returns -1 for negative, 1 for positive and 0 for zero. */
int signum(float x);

/** Return given percentage of the original. */
int percentage(int original, int percent);

#endif
