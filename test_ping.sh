#!/bin/bash

end_time=$((SECONDS + 2))

while [ $SECONDS -lt $end_time ]; do
  curl -s -X GET http://localhost:8080/api/v1/ping
  echo
done
