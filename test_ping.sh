#!/bin/bash

end_time=$((SECONDS + 2))

while [ $SECONDS -lt $end_time ]; do
  curl -s -X GET http://130.193.57.100:8080/api/v1/ping
  echo
done
