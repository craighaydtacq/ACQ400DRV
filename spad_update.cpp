/*
 * spad_update.cpp
 *
 *  Created on: 10 May 2024
 *      Author: pgm
 *
 *  Intended to run as a service at 4255/tcp
 *  Input:
 *  S0,S1,S2,S3,S4,S5,S6,S7,S8
 *  Replace Sn with:
 *   :: BLANK or N : no-touch
 *   :: or a decimal number or a 0xhex number
 *   :: extension: +[N] for increment. +-10 :: decrement 10
 *
 *  NB: spadN are represented as UNSIGNED numbers
 *  WARNING: there's something of a fault here: 0xffffffff -1 => 0x7fffffff @@todo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <iostream>
#include <fstream>
#include <string>

#include <assert.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "split2.h"

#include "popt.h"

#include "local.h"
#include "Env.h"
#include "File.h"
#include "Knob.h"

typedef std::vector<std::string> VS;

void spad_set(unsigned ix, std::string value)
{
	std::string knob = "/etc/acq400/0/spad" + std::to_string(ix);
	File fp(knob.c_str(), "w");

	// spadN ALWAYS takes a hex number
	unsigned vx = strtoul(value.c_str(), 0, 0);
	fprintf(fp.fp(), "%x\n", vx);
}

void spad_increment(unsigned ix, std::string value)
// @@todo: read value, add value, spad_set(value)
{
	std::string knob = "/etc/acq400/0/spad" + std::to_string(ix);
	File fp(knob.c_str(), "r");
	unsigned old_value;
	if (fscanf(fp.fp(), "%x", &old_value) == 1){
		long incr = strtol(value.c_str(), 0, 0);
		while (incr < 0){
			old_value--; incr++;
		}
		while (incr > 0){
			old_value++; incr--;
		}
		//old_value = old_value + strtol(value.c_str(), 0, 0);
		spad_set(ix, std::to_string(old_value));
	}
}
void process(std::string& cmd){
	VS vs;
	split2(cmd, vs, ',');

	for (unsigned ix = 0; ix != vs.size(); ix++) {
		std::string& cmd = vs[ix];
		//std::cerr << "ix:" << ix << " cmd:" << cmd << " sz:" << cmd.size() << std::endl;
		if (cmd.size() == 0 || cmd[0] == 'N'){
			continue;
		}else if (cmd[0] == '+' || cmd[0] == 'I'){
			spad_increment(ix, cmd.substr(1));
		}else{
			spad_set(ix, cmd);
		}

	}
}
void ui(int argc, const char* argv[])
{

}
int main(int argc, const char* argv[])
{
	ui(argc, argv);

	std::string cmd;

	while(std::getline(std::cin, cmd)){
		std::cerr << cmd << std::endl;
		if (cmd.length() > 2 && cmd[0] != '#'){
			process(cmd);
		}
	}
}


