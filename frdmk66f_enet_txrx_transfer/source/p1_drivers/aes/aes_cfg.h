/*
 * aes_cfg.h
 *
 *  Created on: 8 jun. 2025
 *      Author: armzr
 */

#ifndef AES_CFG_H_
#define AES_CFG_H_

// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES encryption in CBC-mode of operation.
// CTR enables encryption in counter-mode.
// ECB enables the basic ECB 16-byte block algorithm. All can be enabled simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at compile time.
#ifndef CBC
  #define CBC 1
#endif

#ifndef ECB
  #define ECB 0
#endif

#ifndef CTR
  #define CTR 0
#endif


#define AES128 1
//#define AES192 1
//#define AES256 1

#endif /* AES_CFG_H_ */
