#!/bin/bash

# Loop from 1 to 100
for i in $(seq 1 100); do
  key="hot$i"
  value="initial_val_hot$i"

  echo "Sending request for $key with value $value..."
  curl -X POST -d "key=$key&value=$value" http://127.0.0.1:8080/create
done

