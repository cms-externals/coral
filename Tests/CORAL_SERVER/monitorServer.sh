#!/bin/bash
( tail -F -s 0.1 /tmp/$USER/csm.txt > /dev/tty ) > /dev/null 2>&1
