#!/bin/sh

set -e

FC="../filecryptify"

RED=$(printf '\033[0;31m')
GREEN=$(printf '\033[0;32m')
NC=$(printf '\033[0m')

test_fail() {
    printf "%s[FAIL]%s %s\n" "$RED" "$NC" "$1"
    exit 1
}

test_ok() {
    printf "%s[ OK ]%s %s\n" "$GREEN" "$NC" "$1"
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
if [ "$size0" -ne "$size1" ] || [ "$size0" -ne "$size2" ]; then
    test_fail "Encrypting known plaintext gives expected size"
fi
test_ok "Encrypting known plaintext gives expected size"
