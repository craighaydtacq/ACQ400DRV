#!/bin/bash
grep '#define MOD_ID_' acq400_mod_id.h | 
	grep -v MOD_ID_TYPE | sed -e 's/0x00//' -e 's/0x//' | 
	awk '{ print $2"="toupper($3) }' | sed -e 's/=0/=/' >scripts/mod_id.sh


