#!/usr/bin/env bash
# Tests for ecbuff
# Written and placed into the public domain by
# Elias Oenal <ecbuff@eliasoenal.com>

set -e

BUILD="ecb_build_test"
rm -rf "./${BUILD}"
mkdir -p "./${BUILD}"

CC=cc
COMMON="-Wall -Wextra -DECB_NO_CFG -DECB_ASSERT"
FILES="ecbuff.c ecbuff_tests.c"
ATOMIC="-DECB_ATOMIC_T=sig_atomic_t -DECB_ATOMIC_MAX=SIG_ATOMIC_MAX"
UINT="-DECB_UINT_T=unsigned int"
UINT_MAX="-DECB_UINT_MAX=UINT_MAX"
SINGLE="-DECB_THREAD_SINGLE"
MULTI="-DECB_THREAD_MULTI"

RED="\033[0;31m"
GREEN="\033[0;32m"
NC="\033[0m"
PASS="${GREEN}Passed:${NC}"
PASS_CNT=0
FAIL="${RED}Failed:${NC}"
FAIL_CNT=0

TEST_PARAMS[1]="-DECBT_BUFF_SIZ=2     -DECBT_ELEM_SIZ=1   -DECB_ELEM_ALIGN=1"
TEST_PARAMS[2]="-DECBT_BUFF_SIZ=8     -DECBT_ELEM_SIZ=2   -DECB_ELEM_ALIGN=2"
TEST_PARAMS[3]="-DECBT_BUFF_SIZ=64    -DECBT_ELEM_SIZ=4   -DECB_ELEM_ALIGN=4"
TEST_PARAMS[4]="-DECBT_BUFF_SIZ=1024  -DECBT_ELEM_SIZ=8   -DECB_ELEM_ALIGN=4"
TEST_PARAMS[5]="-DECBT_BUFF_SIZ=4096  -DECBT_ELEM_SIZ=64  -DECB_ELEM_ALIGN=4"
TEST_PARAMS[6]="-DECBT_BUFF_SIZ=65536 -DECBT_ELEM_SIZ=512 -DECB_ELEM_ALIGN=4"

SUFFIX[1]="_8b"
SUFFIX[2]="_16b"
SUFFIX[3]="_32b"
SUFFIX[4]="_64b"
SUFFIX[5]="_512b"
SUFFIX[6]="_4096b"

DACCESS[1]=""
DACCESS[2]="-DECB_DIRECT_ACCESS"
DACCESS_SUFFIX[1]=""
DACCESS_SUFFIX[2]="_da"


for a in {1..2}; do
for i in {1..6}; do
TESTNAME="single_threaded_basic"${DACCESS_SUFFIX[$a]}${SUFFIX[$i]}
${CC} ${COMMON} ${ATOMIC} "${UINT}" ${UINT_MAX} ${TEST_PARAMS[$i]} ${SINGLE} ${DACCESS[a]} ${FILES} -o ./${BUILD}/${TESTNAME}
if ./${BUILD}/${TESTNAME}; then ((++PASS_CNT)); echo -e "${PASS} ${TESTNAME}"; else ((++FAIL_CNT)); echo -e "${FAIL} ${TESTNAME}"; fi

TESTNAME="single_threaded_drop"${DACCESS_SUFFIX[$a]}${SUFFIX[$i]}
${CC} ${COMMON} ${ATOMIC} "${UINT}" ${UINT_MAX} ${TEST_PARAMS[$i]} ${SINGLE} ${DACCESS[a]} ${FILES} -DECB_WRITE_DROP -o ./${BUILD}/${TESTNAME}
if ./${BUILD}/${TESTNAME}; then ((++PASS_CNT)); echo -e "${PASS} ${TESTNAME}"; else ((++FAIL_CNT)); echo -e "${FAIL} ${TESTNAME}"; fi

TESTNAME="single_threaded_drop_extra"${DACCESS_SUFFIX[$a]}${SUFFIX[$i]}
${CC} ${COMMON} ${ATOMIC} "${UINT}" ${UINT_MAX} ${TEST_PARAMS[$i]} ${SINGLE} ${DACCESS[a]} ${FILES} -DECB_EXTRA_CHECKS -DECB_WRITE_DROP -o ./${BUILD}/${TESTNAME}
if ./${BUILD}/${TESTNAME}; then ((++PASS_CNT)); echo -e "${PASS} ${TESTNAME}"; else ((++FAIL_CNT)); echo -e "${FAIL} ${TESTNAME}"; fi

TESTNAME="single_threaded_overwrite"${DACCESS_SUFFIX[$a]}${SUFFIX[$i]}
${CC} ${COMMON} ${ATOMIC} "${UINT}" ${UINT_MAX} ${TEST_PARAMS[$i]} ${SINGLE} ${DACCESS[a]} ${FILES} -DECB_WRITE_OVERWRITE -o ./${BUILD}/${TESTNAME}
if ./${BUILD}/${TESTNAME}; then ((++PASS_CNT)); echo -e "${PASS} ${TESTNAME}"; else ((++FAIL_CNT)); echo -e "${FAIL} ${TESTNAME}"; fi

TESTNAME="single_threaded_overwrite_extra"${DACCESS_SUFFIX[$a]}${SUFFIX[$i]}
${CC} ${COMMON} ${ATOMIC} "${UINT}" ${UINT_MAX} ${TEST_PARAMS[$i]} ${SINGLE} ${DACCESS[a]} ${FILES} -DECB_EXTRA_CHECKS -DECB_WRITE_OVERWRITE -o ./${BUILD}/${TESTNAME}
if ./${BUILD}/${TESTNAME}; then ((++PASS_CNT)); echo -e "${PASS} ${TESTNAME}"; else ((++FAIL_CNT)); echo -e "${FAIL} ${TESTNAME}"; fi
done
done

for i in {1..6}; do
TESTNAME="multi_threaded_barrier_basic"${SUFFIX[$i]}
${CC} ${COMMON} ${ATOMIC} "${UINT}" ${UINT_MAX} ${TEST_PARAMS[$i]} ${MULTI} -DECB_THREAD_BARRIER ${FILES} -o ./${BUILD}/${TESTNAME}
if ./${BUILD}/${TESTNAME}; then ((++PASS_CNT)); echo -e "${PASS} ${TESTNAME}"; else ((++FAIL_CNT)); echo -e "${FAIL} ${TESTNAME}"; fi

TESTNAME="multi_threaded_barrier_drop"${SUFFIX[$i]}
${CC} ${COMMON} ${ATOMIC} "${UINT}" ${UINT_MAX} ${TEST_PARAMS[$i]} ${MULTI} -DECB_THREAD_BARRIER -DECB_WRITE_DROP ${FILES} -o ./${BUILD}/${TESTNAME}
if ./${BUILD}/${TESTNAME}; then ((++PASS_CNT)); echo -e "${PASS} ${TESTNAME}"; else ((++FAIL_CNT)); echo -e "${FAIL} ${TESTNAME}"; fi

TESTNAME="multi_threaded_barrier_drop_extra"${SUFFIX[$i]}
${CC} ${COMMON} ${ATOMIC} "${UINT}" ${UINT_MAX} ${TEST_PARAMS[$i]} ${MULTI} -DECB_THREAD_BARRIER -DECB_EXTRA_CHECKS -DECB_WRITE_DROP ${FILES} -o ./${BUILD}/${TESTNAME}
if ./${BUILD}/${TESTNAME}; then ((++PASS_CNT)); echo -e "${PASS} ${TESTNAME}"; else ((++FAIL_CNT)); echo -e "${FAIL} ${TESTNAME}"; fi

TESTNAME="multi_threaded_volatile_basic"${SUFFIX[$i]}
${CC} ${COMMON} ${ATOMIC} "${UINT}" ${UINT_MAX} ${TEST_PARAMS[$i]} ${MULTI} -DECB_THREAD_VOLATILE ${FILES} -o ./${BUILD}/${TESTNAME}
if ./${BUILD}/${TESTNAME}; then ((++PASS_CNT)); echo -e "${PASS} ${TESTNAME}"; else ((++FAIL_CNT)); echo -e "${FAIL} ${TESTNAME}"; fi

TESTNAME="multi_threaded_volatile_drop"${SUFFIX[$i]}
${CC} ${COMMON} ${ATOMIC} "${UINT}" ${UINT_MAX} ${TEST_PARAMS[$i]} ${MULTI} -DECB_THREAD_VOLATILE -DECB_WRITE_DROP ${FILES} -o ./${BUILD}/${TESTNAME}
if ./${BUILD}/${TESTNAME}; then ((++PASS_CNT)); echo -e "${PASS} ${TESTNAME}"; else ((++FAIL_CNT)); echo -e "${FAIL} ${TESTNAME}"; fi

TESTNAME="multi_threaded_volatile_drop_extra"${SUFFIX[$i]}
${CC} ${COMMON} ${ATOMIC} "${UINT}" ${UINT_MAX} ${TEST_PARAMS[$i]} ${MULTI} -DECB_THREAD_VOLATILE -DECB_EXTRA_CHECKS -DECB_WRITE_DROP ${FILES} -o ./${BUILD}/${TESTNAME}
if ./${BUILD}/${TESTNAME}; then ((++PASS_CNT)); echo -e "${PASS} ${TESTNAME}"; else ((++FAIL_CNT)); echo -e "${FAIL} ${TESTNAME}"; fi
done

echo -e "Total ${PASS} ${PASS_CNT} ${FAIL} ${FAIL_CNT}"