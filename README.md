# filecryptify

Simple application that uses `libsodium` to encrypt files using symmetric encryption.

Uses default `libsodium` algorithms, which is the ChaCha20 stream cipher combined with Poly1305 as MAC.
(To be exact, the XChaCha20-Poly1305 construction from `libsodium` is used.)

## Installation

Run `make` in the directory. You need `libsodium` installed on your system.

## Usage

`usage: ./filecryptify -G -k keyfile`

Generates a new random symmetric key and stores it in the file `keyfile`.

`usage: ./filecryptify -E [-p plaintextfile] [-c ciphertextfile] -k keyfile`

Encrypts the file `plaintextfile` (or reads from `stdin` if no file is given), using the symmetric key in `keyfile`.
Stores the encrypted ciphertext in `ciphertextfile` (or writes to `stdout` if no file is given).

`usage: ./filecryptify -D [-p plaintextfile] [-c ciphertextfile] -k keyfile`

Decrypts the file `ciphertextfile` (or reads from `stdin` if no file is given), using the symmetric key in `keyfile`.
Stores the decrypted plaintext in `plaintextfile` (or writes to `stdout` if no file is given).

## License

```
Copyright 2018, Linus Karlsson

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
```

