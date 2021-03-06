C_FLAGS = -Wall -pedantic-errors -O3 -funroll-loops -march=native -std=c99 -pipe -DLIBAKRYPT_HAVE_STDIO_H -DLIBAKRYPT_HAVE_STDLIB_H -DLIBAKRYPT_HAVE_STRING_H -DLIBAKRYPT_HAVE_CTYPE_H -DLIBAKRYPT_HAVE_ENDIAN_H -DLIBAKRYPT_HAVE_TIME_H -DLIBAKRYPT_HAVE_SYSLOG_H -DLIBAKRYPT_HAVE_UNISTD_H -DLIBAKRYPT_HAVE_FCNTL_H -DLIBAKRYPT_HAVE_LIMITS_H -DLIBAKRYPT_HAVE_SYSMMAN_H -DLIBAKRYPT_HAVE_SYSSTAT_H -DLIBAKRYPT_HAVE_SYSTYPES_H -DLIBAKRYPT_HAVE_SYSSOCKET_H -DLIBAKRYPT_HAVE_SYSUN_H -DLIBAKRYPT_HAVE_SYSSELECT_H -DLIBAKRYPT_HAVE_ERRNO_H -DLIBAKRYPT_HAVE_TERMIOS_H -DLIBAKRYPT_HAVE_DIRENT_H -DLIBAKRYPT_HAVE_FNMATCH_H -DLIBAKRYPT_HAVE_STDALIGN_H -DLIBAKRYPT_HAVE_STDARG_H -DLIBAKRYPT_HAVE_LOCALE_H -DLIBAKRYPT_HAVE_SIGNAL_H -DLIBAKRYPT_HAVE_PTHREAD -DLIBAKRYPT_HAVE_BUILTIN_MULQ_GCC -DLIBAKRYPT_HAVE_BUILTIN_CLMULEPI64 -O3 -DNDEBUG   -DLIBAKRYPT_CRYPTO_FUNCTIONS=ON -DLIBAKRYPT_NETWORK=ON -DLIBAKRYPT_FIOT=ON -DLIBAKRYPT_ASN1=ON -DLIBAKRYPT_OPTIONS_PATH=\"/etc\" -DLIBAKRYPT_VERSION=\"0.7.15\"

all: client server ak_server ak_client_user ak_client_root

client: client.c client.h common.h config.h
	gcc -o client -pthread client.c

server: server.c server.h common.h config.h
	gcc -o server -pthread -lutil server.c

ak_server: ak_server.c
	gcc -o ak_server $(C_FLAGS) -pthread -lutil -lakrypt-shared ak_server.c

ak_client_user: ak_client_user.c
	gcc -o ak_client_user $(C_FLAGS) -lutil -lakrypt-shared ak_client_user.c

ak_client_root: ak_client_root.c
	gcc -o ak_client_root $(C_FLAGS) -lutil -lakrypt-shared ak_client_root.c

clean:
	rm -rf pty_server pty_client ak_server ak_client_user ak_client_root
