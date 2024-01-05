#!/bin/bash

#COVERAGE_DIR=/share/$​​​​​​​CI_PROJECT_ROOT_NAMESPACE/$CI_PROJECT_NAME​/coverage
COVERAGE_DIR=.coverage

coveragelog=$1

#echo "-->  $CI_PROJECT_ROOT_NAMESPACE"
#echo "-->  $​​​​​​​CI_PROJECT_NAMESPACE"
#echo "-->  $CI_PROJECT_NAME​"
#echo "---> $COVERAGE_DIR"
#env

mkdir -p ${COVERAGE_DIR}
cat $coveragelog | grep -Po '^TOTAL.*\s+(\d+\%)$' | grep -Po '(\d+\%)$' | grep -Po '(\d*)' > ${COVERAGE_DIR}/current_coverage

echo "."
test -f ${COVERAGE_DIR}/max_coverage || cat ${COVERAGE_DIR}/current_coverage | xargs printf %d  > ${COVERAGE_DIR}/max_coverage
echo "-"
test -f ${COVERAGE_DIR}/max_coverage && MAX=$(cat ${COVERAGE_DIR}/max_coverage | xargs printf %d )
echo "--"
test -f ${COVERAGE_DIR}/current_coverage && CURR=$(cat ${COVERAGE_DIR}/current_coverage | xargs printf %d )
echo "---"
test ${CURR} -gt ${MAX} && cat ${COVERAGE_DIR}/max_coverage > ${COVERAGE_DIR}/max_coverage
echo "---- max:${MAX} current:${CURR}"
##test ${MAX} -le ${CURR} && exit 0 // Because coverage is "high enough". We should not drop under 80%
test 80 -le ${CURR} && exit 0

