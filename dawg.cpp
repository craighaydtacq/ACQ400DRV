/* ------------------------------------------------------------------------- *
 * dawg.c++  	Digital AWG
 * ------------------------------------------------------------------------- *
 *   Copyright (C) 2013 Peter Milne, D-TACQ Solutions Ltd                
 *                      <peter dot milne at D hyphen TACQ dot com>          
 *                         www.d-tacq.com
 *   Created on: 4 Nov 2013  
 *    Author: pgm                                                         
 *                                                                           *
 *  This program is free software; you can redistribute it and/or modify     *
 *  it under the terms of Version 2 of the GNU General Public License        *
 *  as published by the Free Software Foundation;                            *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program; if not, write to the Free Software              *
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                */
/* ------------------------------------------------------------------------- */


/*
 * AWG def a series of entries
 * delta-time ch01 ch02
  * delta-time begins 0, must be monotonic, increasing. time in msec.
  * ch01, ch02 : 0xHHHHHHHH numbers representing state of ch0x at time dt

 eg
  0    0x0 0x0		# OPEN all channels
  100  0x3 0x0   	# equiv: close CH01.01,CH01.02
  200  0x6 0x0	 	# OPEN previous channels. close CH01.02,CH01,03
  300  			# OPEN previous channels

 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "popt.h"
#include <list>
#include <time.h>


using namespace std;

typedef unsigned int u32;


#include "ScratchPad.h"

#define MAXCHAN	2
#define MAXSW	20

#define ALL_CLOSED_MASK  (((1<<MAXSW)<<1) - 1)


namespace UI {
	const char* fname = "nowhere";
	bool dry_run;
	int verbose;
	int repeat_count = 1;
	int timescaler = 1;
	bool print_quit;
	bool interactive;
	int site = 5;
	int master_site = 1;
};

#define KNOBS	"./TEST"

#include <stdio.h>
#include <sys/select.h>

int is_ready(int fd) {
	fd_set fdset;
	struct timeval timeout;
	int ret;
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);
	timeout.tv_sec = 0;
	timeout.tv_usec = 1;
	//int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	return select(fd+1, &fdset, NULL, NULL, &timeout) == 1 ? 1 : 0;
}

class DawgEntry {
	static int last_serial;
	int serial;

	char* def;

	list<const char*> disable_list;
	list<const char*> enable_list;

	static void setSwitches(list<const char*> &the_list, bool make);

protected:
	u32 chx[MAXCHAN];	/* should be const, but hard to init */

	DawgEntry(const char* _def, const u32 _dt,
		const u32 _ch01, const u32 _ch02, DawgEntry* _prev);


public:
	static char* knobs;
	const u32 dt;
	DawgEntry* prev;

	const list<const char*>& get_disables() {
		return disable_list;
	}
	const list<const char*>& get_enables() {
		return enable_list;
	}
	void finalize();
	void print();

	static DawgEntry *create(const char* _def, DawgEntry* _prev);
	static DawgEntry *createAllOpen();

	static void buildList(
		list<const char*> &the_list, u32 amask[], u32 bmask[]);
	static void printList(list<const char*> &the_list, const char* label);

	virtual void exec();
};

class ScratchpadReportingDawgEntry: public DawgEntry {

public:
	ScratchpadReportingDawgEntry(const char* _def, const u32 _dt,
			const u32 _ch01, const u32 _ch02, DawgEntry* _prev):
	DawgEntry(_def, _dt, _ch01, _ch02, _prev)
	{}

	virtual void exec();
};



int DawgEntry::last_serial;
char* DawgEntry::knobs;

DawgEntry::DawgEntry(const char* _def, const u32 _dt,
		const u32 _ch01, const u32 _ch02, DawgEntry* _prev) :
	dt(_dt), prev(_prev)
{
	def = new char[strlen(_def)+1];
	strcpy(def, _def);
	serial = ++last_serial;
	chx[0] = _ch01;
	chx[1] = _ch02;

	if (!knobs){
		knobs = new char[32];
		snprintf(knobs, 32, "/dev/acq400.%d.knobs", UI::site);
	}
}

void DawgEntry::buildList(
	list<const char*> &the_list, u32 amask[], u32 bmask[])
{
	for (int ic = 0; ic < MAXCHAN; ++ic){
		// select the bits in amask not present in bmask
		u32 active_bits = amask[ic] ^ (amask[ic]&bmask[ic]);

		for (int sw = 0; sw < MAXSW; ++sw){
			if (active_bits & (1<<sw)){
				char *elt = new char[16];
				snprintf(elt, 8, "CH%02d.%02d", ic+1, sw+1);
				the_list.push_back(elt);
			}
		}
	}
}
void DawgEntry::finalize() {
	u32 prev_chx[2] = {};
	if (prev){
		prev_chx[0] = prev->chx[0];
		prev_chx[1] = prev->chx[1];
	}
	buildList(disable_list, prev_chx, chx);
	buildList(enable_list, chx, prev_chx);
}

void DawgEntry::printList(list<const char*> &the_list, const char* label)
{
	char* buf = new char[1024];

	sprintf(buf, "	%10s :", label);
	list<const char*>::iterator it;
	for (it = the_list.begin(); it != the_list.end(); ++it){
		strcat(buf, *it);
		strcat(buf, " ");
	}
	printf("%s\n", buf);
	delete [] buf;
}

void DawgEntry::setSwitches(list<const char*> &the_list, bool make)
{
	list<const char*>::iterator it;
	for (it = the_list.begin(); it != the_list.end(); ++it){
		FILE* fp = fopen(*it, "w");
		if (!fp){
			perror(*it);
			exit(1);
		}
		fprintf(fp, "%d", make);
		fclose(fp);
	}
}

void DawgEntry::exec()
{
	if (UI::verbose > 1){
		print();
		if (UI::dry_run){
			return;
		}
	}
	setSwitches(disable_list, 0);
	setSwitches(enable_list, 1);
}


void DawgEntry::print()
{
	printf("DawgEntry [%d]: dt:%d \"%s\"  prev:[%d]\n",
			serial, dt, def, prev? prev->serial: 0);
	printList(disable_list, "disable");
	printList(enable_list, "enable");
}



DawgEntry* DawgEntry::create(const char* _def, DawgEntry* _prev)
{
	int _dt;
	unsigned  _ch01, _ch02;
	int nscan;

	if ((nscan = sscanf(_def, "%d %x %x", &_dt, &_ch01, &_ch02)) < 3){
		fprintf(stderr, "ERROR in scan [%d] \"%s\"\n", nscan, _def);
		return 0;
	}else{
		if (_prev == 0){
			if (_dt != 0){
				fprintf(stderr, "ERROR: first entry dt not zero\n");
				return 0;
			}
		}else{
			if (_dt <= _prev->dt){
				fprintf(stderr, "ERROR: dt not monotonic\n");
				return 0;
			}
		}
		if (UI::master_site > 0 && !UI::dry_run){
			return new ScratchpadReportingDawgEntry(
					_def, _dt, _ch01, _ch02, _prev);
		}else{
			return new DawgEntry(_def, _dt, _ch01, _ch02, _prev);
		}
	}
}

DawgEntry *DawgEntry::createAllOpen()
{
	DawgEntry *allClosed = new DawgEntry(
		"all closed", 0, ALL_CLOSED_MASK, ALL_CLOSED_MASK, 0);
	DawgEntry *allOpen = new DawgEntry(
		"all open",   0, 0, 0, allClosed);
}

void ScratchpadReportingDawgEntry::exec()
{
	Scratchpad& sp(Scratchpad::instance(UI::master_site));

	sp.set(Scratchpad::SP_MUX_STATUS, Scratchpad::SP_MUX_STATUS_BUSY);
	DawgEntry::exec();
	sp.set(Scratchpad::SP_MUX_CH01, chx[0]);
	sp.set(Scratchpad::SP_MUX_CH02, chx[1]);
	sp.set(Scratchpad::SP_MUX_STATUS, Scratchpad::SP_MUX_STATUS_DONE);
}


struct poptOption opt_table[] = {
	{ "dry-run", 'D', POPT_ARG_NONE, 0, 'D',
			"go through the motions, no action" 	},
	{ "verbose", 'v', POPT_ARG_INT, &UI::verbose, 0,
			"verbose" 				},
	{ "sequence", 's', POPT_ARG_STRING, &UI::fname, 0,
			"sequence definition file" 		},
	{ "print",    'p', POPT_ARG_NONE, 0, 'p'		},
	{ "repeat",   'r', POPT_ARG_INT,  &UI::repeat_count, 0,
			"repeat count [1]"			},
	{ "slow",       0, POPT_ARG_INT,  &UI::timescaler, 0,
			"slow down by factor N [1]"		},
	{ "interactive", 'i', POPT_ARG_NONE, 0, 'i',
			"controlled from stdin"			},
	POPT_AUTOHELP
	POPT_TABLEEND
};
void ui(int argc, const char* argv[])
{
	poptContext opt_context =
			poptGetContext(argv[0], argc, argv, opt_table, 0);

	int rc;
	while ( (rc = poptGetNextOpt( opt_context )) > 0 ){
		switch(rc){
		case 'p':
			UI::print_quit = 1; break;
		case 'D':
			UI::dry_run = 1;
			UI::verbose += 2;
			fprintf(stderr, "Dry Run, verbose set: %d\n", UI::verbose);
			break;
		case 'i':
			UI::interactive = true;
			break;
		default:
			;
		}
	}
}


list <DawgEntry*> instructions;

char *chomp(char *line)
{
	char *nl;

	while((nl = rindex(line, '\n')) != NULL){
		*nl = '\0';
	}
	return line;
}
void build_sequence(void)
{
	FILE* fp = fopen(UI::fname, "r");
	if (fp == 0){
		perror(UI::fname);
		exit(1);
	}
	char* seq_line = new char[256];
	DawgEntry *prev = 0;
	DawgEntry *now;

	instructions.push_back(DawgEntry::createAllOpen());

	for (int nl = 0; fgets(seq_line, 255, fp); ++nl){
		if (strlen(seq_line) < 2){
			continue;
		}else if (seq_line[0] == '#'){
			continue;
		}else{
			now = DawgEntry::create(chomp(seq_line), prev);

			if (now){
				instructions.push_back(now);
				prev = now;
			}else{
				fprintf(stderr,
					"ERROR: parse failed at line %d\n", nl);
				exit(1);
			}
		}
	}
	fclose(fp);

	list<DawgEntry*>::iterator it;

	for (it = instructions.begin(); it != instructions.end(); ++it){
		(*it)->finalize();
	}
}


void print_sequence(void)
{
	list<DawgEntry*>::iterator it;

	for (it = instructions.begin(); it != instructions.end(); ++it){
		(*it)->print();
	}
}

bool please_stop;		/* could be set by signal */

void waitFor(unsigned dt)
{
	static int dt1;

	if (dt > dt1){
		unsigned ddt = dt - dt1;
		struct timespec ts = {};
		struct timespec rem;

		if (ddt > 1000){
			ts.tv_sec = ddt/1000;
			ddt -= 1000*ts.tv_sec;
		}
		ts.tv_nsec = ddt*1000000;
		if (nanosleep(&ts, &rem)){
			nanosleep(&rem, &rem);	/* we had a signal .. continue */
		}
	}
	dt1 = dt;
}

void prompt() {
	printf("dawg> "); fflush(stdout);
}
void run_sequence(void)
{
	chdir(DawgEntry::knobs);
	list<DawgEntry*>::iterator it = instructions.begin();

	(*it)->exec();
	list<DawgEntry*>::iterator start = ++it;

	if (UI::interactive){
		prompt();
	}

	int iter = 0;
	while ( ++iter <= UI::repeat_count){
		for (it = start; it != instructions.end(); ++it){
			waitFor((*it)->dt * UI::timescaler);
			(*it)->exec();
		}

		if (please_stop){
			break;
		}
		if (UI::interactive){
			if(is_ready(0)){
				switch(getchar()){
				case 'Q':
					printf("user quit\n");
					return;
				}
				prompt();
			}
		}
	}

	if (UI::interactive){
		printf("complete\n");
	}
}

int main(int argc, const char* argv[])
{
	ui(argc, argv);
	build_sequence();
	if (UI::print_quit){
		print_sequence();
	}else{
		run_sequence();
	}
	return 0;
}


