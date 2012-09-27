#!/bin/bash
RETVAL=0
if test "`echo \'symbol | ./mini`" != "'symbol"; then
  echo "'mini' does not preserve symbols"
  RETVAL=1
fi
exit $RETVAL

