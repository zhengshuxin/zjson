#!/bin/sh

valgrind --show-reachable=yes --tool=memcheck --leak-check=yes --track-origins=yes -v ./test -f ../tools/json.txt
