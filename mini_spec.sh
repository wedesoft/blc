#!/bin/bash
RETVAL=0
if test "`echo 'sym' | ./mini`" != "sym"; then
  echo "'mini' does not preserve symbol"
  RETVAL=1
fi
exit $RETVAL

