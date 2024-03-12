#! /bin/bash

ls -1 testcases/test??.c | cut -f2 -d'/' | cut -f1 -d'.' | while read line
do
  echo "TESTCASE $line"

  make $line
  if [[ $? != 0 ]]; then
    echo "ERROR: make did not complete correctly"
    echo
    continue
  fi

  ./$line > testcases/$line.student_out 2>&1
  diff testcases/$line.out testcases/$line.student_out
  echo
done

