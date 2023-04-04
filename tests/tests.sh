#!/bin/bash

set -e

FC="../filecryptify"

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

test_fail() {
    printf "${RED}[FAIL]${NC} ${1}\n"
    exit 1
}

test_ok() {
    printf "${GREEN}[ OK ]${NC} ${1}\n"
}

# Check that two generated keys are not the same
$FC -G -k tmp1.key
$FC -G -k tmp2.key
if diff -q tmp1.key tmp2.key >/dev/null; then
    test_fail "Keys should be different"
fi
test_ok "Keys should be different"

# Check that known-good encrypted file generated with test key can be decrypted
$FC -D -k testkey1.key -c testciphertext1.enc -p tmpplaintext1.txt
if ! diff -q tmpplaintext1.txt knownplaintext1.txt; then
    test_fail "Decrypt known-good encrypted file from filecryptify 1.0.3"
fi
test_ok "Decrypt known-good encrypted file from filecryptify 1.0.3"

# Check that encrypting the same file twice give different ciphertexts (because different salts)
$FC -E -k testkey1.key -p knownplaintext1.txt -c tmpciphertext1.enc
$FC -E -k testkey1.key -p knownplaintext1.txt -c tmpciphertext2.enc
if diff -q tmpciphertext1.enc tmpciphertext2.enc >/dev/null; then
    test_fail "Encrypting same file twice give different ciphertext"
fi
test_ok "Encrypting same file twice give different ciphertext"

# However, the two files above should both have the same size and be identical in size to the known-good ciphertext
size0=$(wc -c < testciphertext1.enc)
size1=$(wc -c < tmpciphertext1.enc)
size2=$(wc -c < tmpciphertext2.enc)
if [[ $size0 -ne $size1 || $size0 -ne $size2 ]]; then
    test_fail "Encrypting known plaintext gives expected size"
fi
test_ok "Encrypting known plaintext gives expected size"

# Password-based encryption should work
$FC -E -w testtesttest -p knownplaintext1.txt -c tmpciphertextpw1.enc
$FC -D -w testtesttest -c tmpciphertextpw1.enc -p tmpplaintextpw1.txt
if ! diff tmpplaintextpw1.txt knownplaintext1.txt; then
    test_fail "Password-based encryption should work"
fi
test_ok "Password-based encryption should work"

# Password-based encrypted files should be 16 bytes larger than key (because of salt for kdf)
size0=$(wc -c < testciphertext1.enc)
size1=$(wc -c < tmpciphertextpw1.enc)
if [[ $(( size1 - 16 )) -ne $size0 ]]; then
    test_fail "Password-based encrypted file has correct size"
fi
test_ok "Password-based encrypted file has correct size"

# Both keyfile and password should not be a valid
if $FC -E -k testkey1.key -w test -p knownplaintext1.txt -c tmpciphertext1.enc 2>/dev/null; then
    test_fail "Giving both key file and password should not be possible"
fi
test_ok "Giving both key file and password should not be possible"
