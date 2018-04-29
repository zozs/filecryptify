#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <sodium.h>

#define CIPHER_ABYTES crypto_secretstream_xchacha20poly1305_ABYTES
#define CIPHER_HEADERBYTES crypto_secretstream_xchacha20poly1305_HEADERBYTES
#define CIPHER_KEYBYTES crypto_secretstream_xchacha20poly1305_KEYBYTES
#define CHUNK_SIZE (4096)

/* Fatal error function. */
static void fatal(const char *error)
{
    fprintf(stderr, "%s\n", error);
    exit(1);
}

/* Decryption function. */
static void decrypt_file(unsigned char (*key)[CIPHER_KEYBYTES], FILE *pf, FILE *cf)
{
    /* Use libsodium's stream API to perform decryption. */
    unsigned char buf_in[CHUNK_SIZE + CIPHER_ABYTES];
    unsigned char buf_out[CHUNK_SIZE];
    unsigned char header[CIPHER_HEADERBYTES];

    crypto_secretstream_xchacha20poly1305_state state;

    unsigned long long out_len;
    size_t read_bytes;
    unsigned char tag;

    if (fread(header, sizeof header, 1, cf) != 1)
        fatal("Failed to read header");
    if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, *key) != 0)
        fatal("Invalid header");
    do {
        read_bytes = fread(buf_in, 1, sizeof buf_in, cf);
        if (crypto_secretstream_xchacha20poly1305_pull(&state, buf_out, &out_len, &tag,
                buf_in, read_bytes, NULL, 0) != 0) {
            /* Corrupted data */
            fatal("Data is corrupted! Decryption incomplete!");
        }
        if (tag == crypto_secretstream_xchacha20poly1305_TAG_FINAL && !feof(cf)) {
            /* Premature end, end of stream reached before EOF) */
            fatal("Premature ending of data");
        }
        if (fwrite(buf_out, 1, (size_t)out_len, pf) != (size_t)out_len)
            fatal("Failed to write plaintext");

    } while (!feof(cf));
}

/* Encryption function. */
static void encrypt_file(unsigned char (*key)[CIPHER_KEYBYTES], FILE *pf, FILE *cf)
{
    /* Use libsodium's stream API to perform encryption. */
    unsigned char buf_in[CHUNK_SIZE];
    unsigned char buf_out[CHUNK_SIZE + CIPHER_ABYTES];
    unsigned char header[CIPHER_HEADERBYTES];

    crypto_secretstream_xchacha20poly1305_state state;

    size_t read_bytes;
    unsigned long long out_len;
    unsigned char tag = 0;

    crypto_secretstream_xchacha20poly1305_init_push(&state, header, *key);
    if (fwrite(header, sizeof header, 1, cf) != 1)
        fatal("Failed to write ciphertext header");

    do {
        read_bytes = fread(buf_in, 1, sizeof buf_in, pf);
        if (feof(pf))
            tag = crypto_secretstream_xchacha20poly1305_TAG_FINAL;
        crypto_secretstream_xchacha20poly1305_push(&state, buf_out, &out_len, buf_in,
            read_bytes, NULL, 0, tag);
        if (fwrite(buf_out, 1, (size_t)out_len, cf) != (size_t)out_len)
            fatal("Failed to write ciphertext to file");
    } while (!feof(pf));
}

/* Generates a new random key suitable for encryption. */
static void generate_key(const char *keyfile)
{
    unsigned char key[CIPHER_KEYBYTES];
    FILE *f;

    umask(0077);
    f = fopen(keyfile, "wb");
    if (!f)
        fatal("Failed to open key file for writing");

    crypto_secretstream_xchacha20poly1305_keygen(key);

    if (fwrite(key, CIPHER_KEYBYTES, 1, f) != 1)
        fatal("Failed to write key to file");

    fclose(f);
}

/* Helper function to open files etc. */
static void setup_files(const char *keyfile, const char *plaintextfile, const char *ciphertextfile,
    bool plainread, unsigned char (*key)[CIPHER_KEYBYTES], FILE **pf, FILE **cf)
{
    assert(key);
    assert(pf);
    assert(cf);

    FILE *kf = fopen(keyfile, "rb");
    if (!kf)
        fatal("Failed to open key file for reading");
    
    if (plaintextfile) {
        *pf = fopen(plaintextfile, plainread ? "rb" : "wb");
        if (!*pf)
            fatal("Failed to open plaintext file");
    } else {
        *pf = plainread ? stdin : stdout;
    }
    
    if (ciphertextfile) {
        *cf = fopen(ciphertextfile, plainread ? "wb" : "rb");
        if (!*cf)
            fatal("Failed to open ciphertext file");
    } else {
        *cf = plainread ? stdout : stdin;
    }

    /* Read key to memory from file. */
    if (fread(*key, CIPHER_KEYBYTES, 1, kf) != 1)
        fatal("Failed to read key from keyfile");
}

/* Usage function */
static void usage(const char *argv0)
{
    fprintf(stderr, "usage: %s -G -k keyfile\n", argv0);
    fprintf(stderr, "usage: %s -E [-p plaintextfile] [-c ciphertextfile] -k keyfile\n", argv0);
    fprintf(stderr, "usage: %s -D [-p plaintextfile] [-c ciphertextfile] -k keyfile\n", argv0);
    exit(1);
}

int main(int argc, char **argv)
{
    /* Parse command-line arguments. */
    int c;
    bool decrypt_flag = false;
    bool encrypt_flag = false;
    bool generate_flag = false;
    int command_count = 0;

    char *keyfile = NULL;
    char *ciphertextfile = NULL;
    char *plaintextfile = NULL;

    while ((c = getopt(argc, argv, "GEDk:p:c:")) != -1) {
        switch (c) {
        case 'G':
            generate_flag = true;
            command_count++;
            break;
        case 'E':
            encrypt_flag = true;
            command_count++;
            break;
        case 'D':
            decrypt_flag = true;
            command_count++;
            break;
        case 'k':
            if (!keyfile)
                keyfile = strdup(optarg);
            break;
        case 'c':
            if (!ciphertextfile)
                ciphertextfile = strdup(optarg);
            break;
        case 'p':
            if (!plaintextfile)
                plaintextfile = strdup(optarg);
            break;
        default:
            usage(argv[0]);
        }
    }

    /* Check command-line argument validity. */
    if (command_count != 1 || !keyfile) {
        /* Multiple commands (-E, -G, -D) were given, or missing keyfile. */
        usage(argv[0]);
    }

    /* Actually do stuff. */
    if (sodium_init() != 0)
        fatal("Failed to initialize libsodium");

    if (generate_flag) {
        generate_key(keyfile);
    } else {
        /* Common code between encrypt and decrypt. Open required files.
         * If ciphertextfile or plaintextfile is missing, assume stdin/stdout.
         * Keyfile must always be readable. */
        FILE *cf = NULL;
        FILE *pf = NULL;
        unsigned char key[CIPHER_KEYBYTES];
        setup_files(keyfile, plaintextfile, ciphertextfile, encrypt_flag, &key, &pf, &cf);
        
        if (encrypt_flag)
            encrypt_file(&key, pf, cf);
        else if (decrypt_flag)
            decrypt_file(&key, pf, cf);
        
        /* Close file descriptors, if they aren't stdout/stdin. */
        if (cf != stdin && cf != stdout)
            fclose(cf);
        if (pf != stdin && pf != stdout)
            fclose(pf);
    }

    /* Cleanup */
    free(keyfile);
    free(ciphertextfile);
    free(plaintextfile);

    return 0;
}
