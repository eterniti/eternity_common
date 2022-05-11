#ifndef H__RIJNDAEL
#define H__RIJNDAEL

#include <stdint.h>

typedef uint32_t u32;
typedef uint8_t u8;


#ifdef __cplusplus
extern "C"
{
#endif

int rijndaelSetupEncrypt(u32 *rk, const u8 *key,
  int keybits);
int rijndaelSetupDecrypt(u32 *rk, const u8 *key,
  int keybits);
void rijndaelEncrypt(const u32 *rk, int nrounds,
  const u8 plaintext[16], u8 ciphertext[16]);
void rijndaelDecrypt(const u32 *rk, int nrounds,
  const u8 ciphertext[16], u8 plaintext[16]);
  
#ifdef __cplusplus
}
#endif

#define KEYLENGTH(keybits) ((keybits)/8)
#define RKLENGTH(keybits)  ((keybits)/8+28)
#define NROUNDS(keybits)   ((keybits)/32+6)

#endif
