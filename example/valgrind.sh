#!/bin/sh

valgrind --show-reachable=yes --tool=memcheck --leak-check=yes --track-origins=yes -v ./json_test
