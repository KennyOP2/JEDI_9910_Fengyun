
INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/share/polarssl/include
    )

SET(POLARSSL_SRCS
     library/aes.c
     library/arc4.c
     library/base64.c
     library/bignum.c
     library/camellia.c
     library/certs.c
     library/cipher.c
     library/cipher_wrap.c
     library/debug.c
     library/des.c
     library/dhm.c
     library/error.c
     library/havege.c
     library/md.c
     library/md_wrap.c
     library/md2.c
     library/md4.c
     library/md5.c
     #library/net.c
     library/padlock.c
     library/pem.c
     library/pkcs11.c
     library/rsa.c
     library/sha1.c
     library/sha2.c
     library/sha4.c
     #library/ssl_cli.c 
     #library/ssl_srv.c 
     #library/ssl_tls.c
     library/timing.c
     library/version.c
     library/x509parse.c
     library/xtea.c
    )

ADD_LIBRARY(polarssl STATIC
    ${POLARSSL_SRCS}
    )
