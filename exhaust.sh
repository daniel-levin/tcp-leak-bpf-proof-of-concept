#!/bin/env sh

for i in {1..1025}
do
	nc 127.0.0.1 9001 &
done
