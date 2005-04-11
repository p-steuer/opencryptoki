
/* (C) COPYRIGHT International Business Machines Corp. 2001, 2002, 2005*/

/***************************************************************************
                          Change Log
                          ==========
       4/25/03    Kapil Sood (kapil@corrent.com)
                  Added DH key pair generation and DH shared key derivation
                  functions.
****************************************************************************/

#include <pthread.h>
#include <stdlib.h>

#include "pkcs11types.h"
#include "stdll.h"
#include "defs.h"
#include "host_defs.h"
#include "h_extern.h"


CK_SLOT_INFO    slot_info;
CK_BBOOL        initialized = FALSE;

// native_mutex is used to protect C_Initialize.  It gets created when the DLL
// is attached, it gets destroyed when the DLL is detached
//
pthread_mutex_t  native_mutex ;
MUTEX   pkcs_mutex, obj_list_mutex, sess_list_mutex, login_mutex;

#if SYSVSEM
int   xprocsemid = -1;
#endif

void *xproclock;

DL_NODE  *sess_list      = NULL;
DL_NODE  *sess_obj_list  = NULL;
DL_NODE  *publ_token_obj_list = NULL;
DL_NODE  *priv_token_obj_list = NULL;
DL_NODE  *object_map     = NULL;


CK_STATE  global_login_state = 0;


LW_SHM_TYPE *global_shm;

CK_ULONG next_session_handle = 1;
CK_ULONG next_object_handle = 1;

TOKEN_DATA  *nv_token_data = NULL;

struct ST_FCN_LIST function_list ;
  extern CK_RV LW_Initialize();                                 /* extern CK_RV SC_Initialize             */
  extern CK_RV SC_GetFunctionList();                            /* extern CK_RV SC_GetFunctionList        */
  extern CK_RV SC_GetTokenInfo();                               /* extern CK_RV SC_GetTokenInfo           */
  extern CK_RV SC_GetMechanismList();                           /* extern CK_RV SC_GetMechanismList       */
  extern CK_RV SC_GetMechanismInfo();                           /* extern CK_RV SC_GetMechanismInfo       */
  extern CK_RV SC_InitToken();                                  /* extern CK_RV SC_InitToken              */
  extern CK_RV SC_InitPIN();                                    /* extern CK_RV SC_InitPIN                */
  extern CK_RV SC_SetPIN();                                     /* extern CK_RV SC_SetPIN                 */
  extern CK_RV SC_OpenSession();                                /* extern CK_RV SC_OpenSession            */
  extern CK_RV SC_CloseSession();                               /* extern CK_RV SC_CloseSession           */
  extern CK_RV SC_CloseAllSessions();                           /* extern CK_RV SC_CloseAllSessions       */
  extern CK_RV SC_GetSessionInfo();                             /* extern CK_RV SC_GetSessionInfo         */
  extern CK_RV SC_GetOperationState();                          /* extern CK_RV SC_GetOperationState      */
  extern CK_RV SC_SetOperationState();                          /* extern CK_RV SC_SetOperationState      */
  extern CK_RV SC_Login();                                      /* extern CK_RV SC_Login                  */
  extern CK_RV SC_Logout();                                     /* extern CK_RV SC_Logout                 */
  extern CK_RV SC_CreateObject();                               /* extern CK_RV SC_CreateObject           */
  extern CK_RV SC_CopyObject();                                 /* extern CK_RV SC_CopyObject             */
  extern CK_RV SC_DestroyObject();                              /* extern CK_RV SC_DestroyObject          */
  extern CK_RV SC_GetObjectSize();                              /* extern CK_RV SC_GetObjectSize          */
  extern CK_RV SC_GetAttributeValue();                          /* extern CK_RV SC_GetAttributeValue      */
  extern CK_RV SC_SetAttributeValue();                          /* extern CK_RV SC_SetAttributeValue      */
  extern CK_RV SC_FindObjectsInit();                            /* extern CK_RV SC_FindObjectsInit        */
  extern CK_RV SC_FindObjects();                                /* extern CK_RV SC_FindObjects            */
  extern CK_RV SC_FindObjectsFinal();                           /* extern CK_RV SC_FindObjectsFinal       */
  extern CK_RV SC_EncryptInit();                                /* extern CK_RV SC_EncryptInit            */
  extern CK_RV SC_Encrypt();                                    /* extern CK_RV SC_Encrypt                */
  extern CK_RV SC_EncryptUpdate();                              /* extern CK_RV SC_EncryptUpdate          */
  extern CK_RV SC_EncryptFinal();                               /* extern CK_RV SC_EncryptFinal           */
  extern CK_RV SC_DecryptInit();                                /* extern CK_RV SC_DecryptInit            */
  extern CK_RV SC_Decrypt();                                    /* extern CK_RV SC_Decrypt                */
  extern CK_RV SC_DecryptUpdate();                              /* extern CK_RV SC_DecryptUpdate          */
  extern CK_RV SC_DecryptFinal();                               /* extern CK_RV SC_DecryptFinal           */
  extern CK_RV SC_DigestInit();                                 /* extern CK_RV SC_DigestInit             */
  extern CK_RV SC_Digest();                                     /* extern CK_RV SC_Digest                 */
  extern CK_RV SC_DigestUpdate();                               /* extern CK_RV SC_DigestUpdate           */
  extern CK_RV SC_DigestKey();                                  /* extern CK_RV SC_DigestKey              */
  extern CK_RV SC_DigestFinal();                                /* extern CK_RV SC_DigestFinal            */
  extern CK_RV SC_SignInit();                                   /* extern CK_RV SC_SignInit               */
  extern CK_RV SC_Sign();                                       /* extern CK_RV SC_Sign                   */
  extern CK_RV SC_SignUpdate();                                 /* extern CK_RV SC_SignUpdate             */
  extern CK_RV SC_SignFinal();                                  /* extern CK_RV SC_SignFinal              */
  extern CK_RV SC_SignRecoverInit();                            /* extern CK_RV SC_SignRecoverInit        */
  extern CK_RV SC_SignRecover();                                /* extern CK_RV SC_SignRecover            */
  extern CK_RV SC_VerifyInit();                                 /* extern CK_RV SC_VerifyInit             */
  extern CK_RV SC_Verify();                                     /* extern CK_RV SC_Verify                 */
  extern CK_RV SC_VerifyUpdate();                               /* extern CK_RV SC_VerifyUpdate           */
  extern CK_RV SC_VerifyFinal();                                /* extern CK_RV SC_VerifyFinal            */
  extern CK_RV SC_VerifyRecoverInit();                          /* extern CK_RV SC_VerifyRecoverInit      */
  extern CK_RV SC_VerifyRecover();                              /* extern CK_RV SC_VerifyRecover          */
  extern CK_RV SC_DigestEncryptUpdate();                        /* extern CK_RV SC_DigestEncryptUpdate    */
  extern CK_RV SC_DecryptDigestUpdate();                        /* extern CK_RV SC_DecryptDigestUpdate    */
  extern CK_RV SC_SignEncryptUpdate();                          /* extern CK_RV SC_SignEncryptUpdate      */
  extern CK_RV SC_DecryptVerifyUpdate();                        /* extern CK_RV SC_DecryptVerifyUpdate    */
  extern CK_RV SC_GenerateKey();                                /* extern CK_RV SC_GenerateKey            */
  extern CK_RV SC_GenerateKeyPair();                            /* extern CK_RV SC_GenerateKeyPair        */
  extern CK_RV SC_WrapKey();                                    /* extern CK_RV SC_WrapKey                */
  extern CK_RV SC_UnwrapKey();                                  /* extern CK_RV SC_UnwrapKey              */
  extern CK_RV SC_DeriveKey();                                  /* extern CK_RV SC_DeriveKey              */
  extern CK_RV SC_SeedRandom();                                 /* extern CK_RV SC_SeedRandom             */
  extern CK_RV SC_GenerateRandom();                             /* extern CK_RV SC_GenerateRandom         */
  extern CK_RV SC_GetFunctionStatus();                          /* extern CK_RV SC_GetFunctionStatus      */
  extern CK_RV SC_CancelFunction();                             /* extern CK_RV SC_CancelFunction         */
  extern CK_RV SC_WaitForSlotEvent();                           /* extern CK_RV SC_WaitForSlotEvent       */


// ber_rsaEncryption is the AlgorithmIdentifier for RSA including the
// NULL parameter (0x05 0x00)
//
CK_BYTE  ber_AlgIdRSAEncryption[] = { 0x30, 0x0D, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00 };

// OBJECT IDENTIFIERs
//
CK_BYTE  ber_idDSA[]         = { 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x38, 0x04, 0x01 };
CK_BYTE  ber_rsaEncryption[] = { 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01 };
CK_BYTE  ber_md2WithRSAEncryption[] = { 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x02, 0x05, 0x00 };
CK_BYTE  ber_md4WithRSAEncryption[] = { 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x04, 0x05, 0x00 };
CK_BYTE  ber_md5WithRSAEncryption[] = { 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x05, 0x05, 0x00 };
CK_BYTE  ber_sha1WithRSAEncryption[] = { 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00 };


CK_ULONG ber_AlgIdRSAEncryptionLen = sizeof(ber_AlgIdRSAEncryption);
CK_ULONG ber_rsaEncryptionLen = sizeof(ber_rsaEncryption);
CK_ULONG ber_idDSALen = sizeof(ber_idDSA);
CK_ULONG ber_md2WithRSAEncryptionLen = sizeof(ber_md2WithRSAEncryption);
CK_ULONG ber_md4WithRSAEncryptionLen = sizeof(ber_md4WithRSAEncryption);
CK_ULONG ber_md5WithRSAEncryptionLen = sizeof(ber_md5WithRSAEncryption);
CK_ULONG ber_sha1WithRSAEncryptionLen= sizeof(ber_sha1WithRSAEncryption);



CK_ULONG des_weak_count = 4;
CK_ULONG des_semi_weak_count = 12;
CK_ULONG des_possibly_weak_count = 48;

CK_BYTE  des_weak_keys[4][8] = {
   {0x01, 0x01, 0x01, 0x01,  0x01, 0x01, 0x01, 0x01},
   {0x1F, 0x1F, 0x1F, 0x1F,  0x0E, 0x0E, 0x0E, 0x0E},
   {0xE0, 0xE0, 0xE0, 0xE0,  0xF1, 0xF1, 0xF1, 0xF1},
   {0xFE, 0xFE, 0xFE, 0xFE,  0xFE, 0xFE, 0xFE, 0xFE}
};

CK_BYTE  des_semi_weak_keys[12][8] = {
   {0x01, 0xFE, 0x01, 0xFE,  0x01, 0xFE, 0x01, 0xFE},
   {0xFE, 0x01, 0xFE, 0x01,  0xFE, 0x01, 0xFE, 0x01},
   {0x1F, 0xE0, 0x1F, 0xE0,  0x0E, 0xF1, 0x0E, 0xF1},
   {0xE0, 0x1F, 0xE0, 0x1F,  0xF1, 0x0E, 0xF1, 0x0E},
   {0x01, 0xE0, 0x01, 0xE0,  0x01, 0xF1, 0x01, 0xF1},
   {0xE0, 0x01, 0xE0, 0x01,  0xF1, 0x01, 0xF1, 0x01},
   {0x1F, 0xFE, 0x1F, 0xFE,  0x0E, 0xFE, 0x0E, 0xFE},
   {0xFE, 0x1F, 0xFE, 0x1F,  0xFE, 0x0E, 0xFE, 0x0E},
   {0x01, 0x1F, 0x01, 0x1F,  0x01, 0x0E, 0x01, 0x0E},
   {0x1F, 0x01, 0x1F, 0x01,  0x0E, 0x01, 0x0E, 0x01},
   {0xE0, 0xFE, 0xE0, 0xFE,  0xF1, 0xFE, 0xF1, 0xFE},
   {0xFE, 0xE0, 0xFE, 0xE0,  0xFE, 0xF1, 0xFE, 0xF1}
};

CK_BYTE  des_possibly_weak_keys[48][8] = {
   {0x1F, 0x1F, 0x01, 0x01,  0x0E, 0x0E, 0x01, 0x01},
   {0x01, 0x1F, 0x1F, 0x01,  0x01, 0x0E, 0x0E, 0x01},
   {0x1F, 0x01, 0x01, 0x1F,  0x0E, 0x01, 0x01, 0x0E},
   {0x01, 0x01, 0x1F, 0x1F,  0x01, 0x01, 0x0E, 0x0E},

   {0xE0, 0xE0, 0x01, 0x01,  0xF1, 0xF1, 0x01, 0x01},
   {0xFE, 0xFE, 0x01, 0x01,  0xFE, 0xFE, 0x01, 0x01},
   {0xFE, 0xE0, 0x1F, 0x01,  0xFE, 0xF1, 0x0E, 0x01},
   {0xE0, 0xFE, 0x1F, 0x01,  0xF1, 0xFE, 0x0E, 0x01},
   {0xFE, 0xE0, 0x01, 0x1F,  0xFE, 0xF1, 0x01, 0x0E},
   {0xE0, 0xFE, 0x01, 0x1F,  0xF1, 0xFE, 0x01, 0x0E},
   {0xE0, 0xE0, 0x1F, 0x1F,  0xF1, 0xF1, 0x0E, 0x0E},
   {0xFE, 0xFE, 0x1F, 0x1F,  0xFE, 0xFE, 0x0E, 0x0E},

   {0xFE, 0x1F, 0xE0, 0x01,  0xFE, 0x0E, 0xF1, 0x01},
   {0xE0, 0x1F, 0xFE, 0x01,  0xF1, 0x0E, 0xFE, 0x01},
   {0xFE, 0x01, 0xE0, 0x1F,  0xFE, 0x01, 0xF1, 0x0E},
   {0xE0, 0x01, 0xFE, 0x1F,  0xF1, 0x01, 0xFE, 0x0E},

   {0x01, 0xE0, 0xE0, 0x01,  0x01, 0xF1, 0xF1, 0x01},
   {0x1F, 0xFE, 0xE0, 0x01,  0x0E, 0xFE, 0xF0, 0x01},
   {0x1F, 0xE0, 0xFE, 0x01,  0x0E, 0xF1, 0xFE, 0x01},
   {0x01, 0xFE, 0xFE, 0x01,  0x01, 0xFE, 0xFE, 0x01},
   {0x1F, 0xE0, 0xE0, 0x1F,  0x0E, 0xF1, 0xF1, 0x0E},
   {0x01, 0xFE, 0xE0, 0x1F,  0x01, 0xFE, 0xF1, 0x0E},
   {0x01, 0xE0, 0xFE, 0x1F,  0x01, 0xF1, 0xFE, 0x0E},
   {0x1F, 0xFE, 0xFE, 0x1F,  0x0E, 0xFE, 0xFE, 0x0E},

   {0xE0, 0x01, 0x01, 0xE0,  0xF1, 0x01, 0x01, 0xF1},
   {0xFE, 0x1F, 0x01, 0xE0,  0xFE, 0x0E, 0x01, 0xF1},
   {0xFE, 0x01, 0x1F, 0xE0,  0xFE, 0x01, 0x0E, 0xF1},
   {0xE0, 0x1F, 0x1F, 0xE0,  0xF1, 0x0E, 0x0E, 0xF1},
   {0xFE, 0x01, 0x01, 0xFE,  0xFE, 0x01, 0x01, 0xFE},
   {0xE0, 0x1F, 0x01, 0xFE,  0xF1, 0x0E, 0x01, 0xFE},
   {0xE0, 0x01, 0x1F, 0xFE,  0xF1, 0x01, 0x0E, 0xFE},
   {0xFE, 0x1F, 0x1F, 0xFE,  0xFE, 0x0E, 0x0E, 0xFE},

   {0x1F, 0xFE, 0x01, 0xE0,  0x0E, 0xFE, 0x01, 0xF1},
   {0x01, 0xFE, 0x1F, 0xE0,  0x01, 0xFE, 0x0E, 0xF1},
   {0x1F, 0xE0, 0x01, 0xFE,  0x0E, 0xF1, 0x01, 0xFE},
   {0x01, 0xE0, 0x1F, 0xFE,  0x01, 0xF1, 0x0E, 0xFE},

   {0x01, 0x01, 0xE0, 0xE0,  0x01, 0x01, 0xF1, 0xF1},
   {0x1F, 0x1F, 0xE0, 0xE0,  0x0E, 0x0E, 0xF1, 0xF1},
   {0x1F, 0x01, 0xFE, 0xE0,  0x0E, 0x01, 0xFE, 0xF1},
   {0x01, 0x1F, 0xFE, 0xE0,  0x01, 0x0E, 0xFE, 0xF1},
   {0x1F, 0x01, 0xE0, 0xFE,  0x0E, 0x01, 0xF1, 0xFE},
   {0x01, 0x1F, 0xE0, 0xFE,  0x01, 0x0E, 0xF1, 0xFE},
   {0x01, 0x01, 0xFE, 0xFE,  0x01, 0x01, 0xFE, 0xFE},
   {0x1F, 0x1F, 0xFE, 0xFE,  0x0E, 0x0E, 0xFE, 0xFE},

   {0xFE, 0xFE, 0xE0, 0xE0,  0xFE, 0xFE, 0xF1, 0xF1},
   {0xE0, 0xFE, 0xFE, 0xE0,  0xF1, 0xFE, 0xFE, 0xF1},
   {0xFE, 0xE0, 0xE0, 0xFE,  0xFE, 0xF1, 0xF1, 0xFE},
   {0xE0, 0xE0, 0xFE, 0xFE,  0xF1, 0xF1, 0xFE, 0xFE}
};


MECH_LIST_ELEMENT mech_list[] = {
   { CKM_RSA_PKCS_KEY_PAIR_GEN,     512, 2048, CKF_HW | CKF_GENERATE_KEY_PAIR },
#if !(NODSA)
   { CKM_DSA_KEY_PAIR_GEN,          512, 1024, CKF_HW | CKF_GENERATE_KEY_PAIR },
#endif
   { CKM_DES_KEY_GEN,                 0,    0, CKF_HW | CKF_GENERATE },
   { CKM_DES3_KEY_GEN,                0,    0, CKF_HW | CKF_GENERATE },
#if !(NOCDMF)
   { CKM_CDMF_KEY_GEN,                0,    0, CKF_HW | CKF_GENERATE },
#endif
#if 0
   { CKM_RSA_PKCS,                  512, 2048, CKF_HW           |
                                               CKF_ENCRYPT      | CKF_DECRYPT |
                                               CKF_WRAP         | CKF_UNWRAP  |
                                               CKF_SIGN         | CKF_VERIFY  |
                                               CKF_SIGN_RECOVER | CKF_VERIFY_RECOVER },
#endif
   { CKM_RSA_PKCS_OAEP,             512, 2048, CKF_HW           |
                                               CKF_ENCRYPT      | CKF_DECRYPT |
                                               CKF_WRAP         | CKF_UNWRAP  |
                                               CKF_SIGN         | CKF_VERIFY  |
                                               CKF_SIGN_RECOVER | CKF_VERIFY_RECOVER },
#if !(NOX509)
   { CKM_RSA_X_509,                 512, 2048, CKF_HW           |
                                               CKF_ENCRYPT      | CKF_DECRYPT |
                                               CKF_WRAP         | CKF_UNWRAP  |
                                               CKF_SIGN         | CKF_VERIFY  |
                                               CKF_SIGN_RECOVER | CKF_VERIFY_RECOVER },
#endif
#if !(NOMD2)
   { CKM_MD2_RSA_PKCS,              512, 2048, CKF_HW      |
                                               CKF_SIGN    | CKF_VERIFY },

#endif
#if !(NOMD5)
   { CKM_MD5_RSA_PKCS,              512, 2048, CKF_HW      |
                                               CKF_SIGN    | CKF_VERIFY },
#endif
#if !(NOSHA1)
   { CKM_SHA1_RSA_PKCS,             512, 2048, CKF_HW      |
                                               CKF_SIGN    | CKF_VERIFY },
#endif


#if !(NODSA)
   { CKM_DSA,                       512, 1024, CKF_HW      |
                                               CKF_SIGN    | CKF_VERIFY },
#endif

/* Begin code contributed by Corrent corp. */
#if !(NODH)
   { CKM_DH_PKCS_DERIVE,            512, 2048, CKF_HW | CKF_DERIVE },
   { CKM_DH_PKCS_KEY_PAIR_GEN,      512, 2048, CKF_HW | CKF_GENERATE_KEY_PAIR },
#endif
/* End code contributed by Corrent corp. */

   { CKM_DES_ECB,                     0,    0, CKF_HW      |
                                               CKF_ENCRYPT | CKF_DECRYPT |
                                               CKF_WRAP    | CKF_UNWRAP },

   { CKM_DES_CBC,                     0,    0, CKF_HW      |
                                               CKF_ENCRYPT | CKF_DECRYPT |
                                               CKF_WRAP    | CKF_UNWRAP },

   { CKM_DES_CBC_PAD,                 0,    0, CKF_HW      |
                                               CKF_ENCRYPT | CKF_DECRYPT |
                                               CKF_WRAP    | CKF_UNWRAP },

#if !(NOCDMF)
   { CKM_CDMF_ECB,                    0,    0, CKF_HW      |
                                               CKF_ENCRYPT | CKF_DECRYPT |
                                               CKF_WRAP    | CKF_UNWRAP },

   { CKM_CDMF_CBC,                    0,    0, CKF_HW      |
                                               CKF_ENCRYPT | CKF_DECRYPT |
                                               CKF_WRAP    | CKF_UNWRAP },
#endif

   { CKM_DES3_ECB,                    0,    0, CKF_HW      |
                                               CKF_ENCRYPT | CKF_DECRYPT |
                                               CKF_WRAP    | CKF_UNWRAP },

   { CKM_DES3_CBC,                    0,    0, CKF_HW      |
                                               CKF_ENCRYPT | CKF_DECRYPT |
                                               CKF_WRAP    | CKF_UNWRAP },

   { CKM_DES3_CBC_PAD,                0,    0, CKF_HW      |
                                               CKF_ENCRYPT | CKF_DECRYPT |
                                               CKF_WRAP    | CKF_UNWRAP },

#if !(NOSHA1)
   { CKM_SHA_1,                       0,    0, CKF_HW | CKF_DIGEST },
   { CKM_SHA_1_HMAC,                  0,    0, CKF_HW | CKF_SIGN | CKF_VERIFY },
   { CKM_SHA_1_HMAC_GENERAL,          0,    0, CKF_HW | CKF_SIGN | CKF_VERIFY },
#endif

#if !(NOMD2)
   { CKM_MD2,                         0,    0, CKF_HW | CKF_DIGEST },
   { CKM_MD2_HMAC,                    0,    0, CKF_HW | CKF_SIGN | CKF_VERIFY },
   { CKM_MD2_HMAC_GENERAL,            0,    0, CKF_HW | CKF_SIGN | CKF_VERIFY },
#endif

#if !(NOMD5)
   { CKM_MD5,                         0,    0, CKF_HW | CKF_DIGEST },
   { CKM_MD5_HMAC,                    0,    0, CKF_HW | CKF_SIGN | CKF_VERIFY },
   { CKM_MD5_HMAC_GENERAL,            0,    0, CKF_HW | CKF_SIGN | CKF_VERIFY },
#endif

   { CKM_SSL3_PRE_MASTER_KEY_GEN,    48,   48, CKF_HW | CKF_GENERATE },
   { CKM_SSL3_MASTER_KEY_DERIVE,     48,   48, CKF_HW | CKF_DERIVE },
   { CKM_SSL3_KEY_AND_MAC_DERIVE,    48,   48, CKF_HW | CKF_DERIVE },
   { CKM_SSL3_MD5_MAC,              384,  384, CKF_HW | CKF_SIGN | CKF_VERIFY },
   { CKM_SSL3_SHA1_MAC,             384,  384, CKF_HW | CKF_SIGN | CKF_VERIFY },

#if !(NOAES)
   { CKM_AES_KEY_GEN,                16,   32, CKF_HW },
   { CKM_AES_ECB,                    16,   32, CKF_HW      |
   					       CKF_ENCRYPT | CKF_DECRYPT |
   					       CKF_WRAP    | CKF_UNWRAP },
   { CKM_AES_CBC,                    16,   32, CKF_HW      |
   					       CKF_ENCRYPT | CKF_DECRYPT |
   					       CKF_WRAP    | CKF_UNWRAP },
   { CKM_AES_MAC,                    16,   32, CKF_HW | CKF_SIGN | CKF_VERIFY },
   { CKM_AES_MAC_GENERAL,            16,   32, CKF_HW | CKF_SIGN | CKF_VERIFY },
   { CKM_AES_CBC_PAD,                16,   32, CKF_HW      |
   					       CKF_ENCRYPT | CKF_DECRYPT |
   					       CKF_WRAP    | CKF_UNWRAP },
#endif
	
#if !(NORIPE)
   { CKM_RIPEMD128,			0,   0, CKF_HW | CKF_DIGEST },
   { CKM_RIPEMD128_HMAC,		0,   0, CKF_HW | CKF_SIGN | CKF_VERIFY },
   { CKM_RIPEMD128_HMAC_GENERAL,	0,   0, CKF_HW | CKF_SIGN | CKF_VERIFY },
   { CKM_RIPEMD160,			0,   0, CKF_HW | CKF_DIGEST },
   { CKM_RIPEMD160_HMAC,		0,   0, CKF_HW | CKF_SIGN | CKF_VERIFY },
   { CKM_RIPEMD160_HMAC_GENERAL,	0,   0, CKF_HW | CKF_SIGN | CKF_VERIFY },
#endif
   
};

CK_ULONG  mech_list_len = (sizeof(mech_list) / sizeof(MECH_LIST_ELEMENT));

// default SO pin hash values
//
// default SO pin = "87654321"
//
CK_BYTE default_so_pin_md5[MD5_HASH_SIZE] = {
  0x5E, 0x86, 0x67, 0xA4, 0x39, 0xC6, 0x8F, 0x51,
  0x45, 0xDD, 0x2F, 0xCB, 0xEC, 0xF0, 0x22, 0x09
};

CK_BYTE default_so_pin_sha[SHA1_HASH_SIZE] = {
  0xA7, 0xD5, 0x79, 0xBA, 0x76, 0x39, 0x80, 0x70,
  0xEA, 0xE6, 0x54, 0xC3, 0x0F, 0xF1, 0x53, 0xA4,
  0xC2, 0x73, 0x27, 0x2A
};

CK_BYTE user_pin_md5[MD5_HASH_SIZE];
CK_BYTE so_pin_md5[MD5_HASH_SIZE];
CK_BYTE master_key[3 * DES_KEY_SIZE];


