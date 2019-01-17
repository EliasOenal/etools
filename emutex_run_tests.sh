#!/usr/bin/env bash
# Tests for emutex
# Written and placed into the public domain by
# Elias Oenal <emutex@eliasoenal.com>

set -e

BUILD="emutex_build_test"
rm -rf "./${BUILD}"
mkdir -p "./${BUILD}"

CC=cc
COMMON="-Wall -Wextra"
FILES="emutex.c emutex_tests.c"
SINGLE="-DEMUTEX_THREAD_SINGLE"
MULTI="-pthread -DEMUTEX_THREAD_MULTI"

RED="\033[0;31m"
GREEN="\033[0;32m"
NC="\033[0m"
PASS="${GREEN}Passed:${NC}"
PASS_CNT=0
FAIL="${RED}Failed:${NC}"
FAIL_CNT=0



TESTNAME="single_threaded"
${CC} ${COMMON} ${SINGLE} ${FILES} -o ./${BUILD}/${TESTNAME}
if ./${BUILD}/${TESTNAME}; then ((++PASS_CNT)); echo -e "${PASS} ${TESTNAME}"; else ((++FAIL_CNT)); echo -e "${FAIL} ${TESTNAME}"; fi

TESTNAME="multi_threaded"
${CC} ${COMMON} ${MULTI} ${FILES} -o ./${BUILD}/${TESTNAME}
if ./${BUILD}/${TESTNAME}; then ((++PASS_CNT)); echo -e "${PASS} ${TESTNAME}"; else ((++FAIL_CNT)); echo -e "${FAIL} ${TESTNAME}"; fi


echo -e "Total ${PASS} ${PASS_CNT} ${FAIL} ${FAIL_CNT}"