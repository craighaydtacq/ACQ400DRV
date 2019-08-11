/* ------------------------------------------------------------------------- */
/* multivent.cpp  D-TACQ ACQ400 FMC  DRIVER handle multiple events.
 * field the event, locate in memory, extract and save
 * Project: ACQ420_FMC
 * Created: 10 August 2019  			/ User: pgm
 * ------------------------------------------------------------------------- *
 *   Copyright (C) 2019 Peter Milne, D-TACQ Solutions Ltd         *
 *                      <peter dot milne at D hyphen TACQ dot com>           *
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                *
 *
 * TODO 
 * TODO
\* ------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "popt.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <vector>

#include <libgen.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>

#include <sched.h>

#include <syslog.h>

#define VERID	"B1000"

#define NCHAN	4

#include <semaphore.h>
#include <syslog.h>


#include "local.h"		/* chomp() hopefully, not a lot of other garbage */
#include "knobs.h"

#include "File.h"


using namespace std;


namespace G {
	unsigned sample_size = sizeof(unsigned);	// bytes per sample
	unsigned offset = 0;
	int devnum = 0;					// full system.
	int verbose;
	FILE* fp_out = stdout;
	FILE* fp_in = stdin;
};

#include "Buffer.h"

struct poptOption opt_table[] = {
	{ "sample-size", 'S', POPT_ARG_INT, &G::sample_size, 0,
			"bytes per sample [deprecated]"
	},
	{
	  "verbose", 'v', POPT_ARG_INT, &G::verbose, 0, "debug"
	},
	POPT_AUTOHELP
	POPT_TABLEEND
};


char *getRoot(int devnum)
{
	char *_root = new char [128];
	struct stat sb;

	sprintf(_root, "/dev/acq420.%d", devnum);
	if (stat(_root, &sb) == 0){
		return _root;
	}

	sprintf(_root, "/dev/acq400.%d", devnum);
	if (stat(_root, &sb) == 0){
		return _root;
	}

	fprintf(stderr, "ERROR: /dev/acq4x0.%d NOT FOUND\n", devnum);
	exit(1);
}

int Buffer::create(const char* root, int _buffer_len)
{
	char* fname = new char[128];
	sprintf(fname, "%s.hb/%03d", root, Buffer::last_buf);

	the_buffers.push_back(new MapBuffer(fname, _buffer_len));
	return 0;
}

int G_nsamples;

/*
acq2106_112> grep ^050, /proc/driver/acq400/0/buffers
050,e1800000,0x21800000,0x400000,0
*/
unsigned getSpecificBufferlen(int ibuf)
{
	char cmd[128];
	sprintf(cmd, "grep ^%03d, /proc/driver/acq400/0/buffers", ibuf);
	unsigned bl = 0;
	FILE * pp = popen(cmd, "r");
	if (fscanf(pp, "%*d,%*x,%*x,%x,%*d", &bl) == 1){
		fprintf(stderr, "success bl:0x%08x\n", bl);
	}else{
		fprintf(stderr, "FAIL: %s\n", cmd);
	}
	pclose(pp);
	return bl;
}


class EventHandler {
	int site;
	int offset;
	int fd;
	char *fname;
	static vector<EventHandler*> handlers;	
	typedef vector<EventHandler*>::iterator EHI;
protected:
	EventHandler(int _site, int _offset): site(_site), offset(_offset)
	{
		fname = new char[80];
		sprintf(fname, "%s.ev", getRoot(site));
		fd = open(fname, O_RDONLY);
	}
	virtual ~EventHandler() {
		close(fd);
		delete [] fname;
	}
	virtual int action() {
		return 0;
	}

public:
	static bool create(int _site);

       	static int poll() {
		sigset_t emptyset;
		struct timespec pto = {};
		fd_set exceptfds, readfds;
		int fd_max = -1;
		int rc;
		
		sigemptyset(&emptyset);
		pto.tv_sec = 5;
		FD_ZERO(&exceptfds);
		FD_ZERO(&readfds);

                for (EHI it = handlers.begin(); it != handlers.end(); ++it){
			int fd = (*it)->fd;
			FD_SET(fd, &exceptfds);
			FD_SET(fd, &readfds);
			if (fd > fd_max){
				fd_max = fd;
			}
		}

		rc = pselect(fd_max+1, &readfds, NULL, &exceptfds, &pto, &emptyset);
		if (rc < 0){
			syslog(LOG_ERR, "ERROR: pselect() fail %d\n", errno);
		}
                for (EHI it = handlers.begin(); it != handlers.end(); ++it){
			int fd = (*it)->fd;
			if (FD_ISSET(fd, &readfds)){
				(*it)->action();
			}
			if (FD_ISSET(fd, &exceptfds)){
				syslog(LOG_ERR, "ERROR: fail on %s %d\n", (*it)->fname, errno);
			}
		}

		return rc;
       }
};

vector<EventHandler*> EventHandler::handlers;


class COSEventHandler: public EventHandler {
public:
	COSEventHandler(int _site, int _offset) : EventHandler(_site, _offset) 
	{}
};

class ATDEventHandler: public EventHandler {
public:
        ATDEventHandler(int _site, int _offset) : EventHandler(_site, _offset)
        {}
};


bool EventHandler::create(int _site) {
	/* WORDTODO : replace hard coded functions */
	switch(_site){
		case 2:
			handlers.push_back(new COSEventHandler(_site, 32));
			return true;	
		case 14:
			handlers.push_back(new ATDEventHandler(_site, 0));
			return true;
		default:
			return false;
	}
}

class BufferManager {
	void init_buffers()
	{
		const char* root = getRoot(G::devnum);

		for (unsigned ii = 0; ii < Buffer::nbuffers; ++ii){
			Buffer::create(root, Buffer::bufferlen);
		}
	}
	void delete_buffers()
	{
		/* this _should_ be automatic. But it's not! */
		for (unsigned ii = 0; ii < Buffer::the_buffers.size(); ++ii){
			delete Buffer::the_buffers[ii];
		}
	}
public:
	BufferManager(unsigned start_buf = 0) {
		Buffer::last_buf = start_buf;
		init_buffers();
	}
	~BufferManager() {
		delete_buffers();
	}
};

void ui(int argc, const char** argv)
{
        poptContext opt_context =
                        poptGetContext(argv[0], argc, argv, opt_table, 0);

        int rc;

        while ( (rc = poptGetNextOpt( opt_context )) >= 0 ){
                switch(rc){
                default:
                        ;
                }
        }
	const char* ssite;
	while ((ssite = poptGetArg(opt_context)) != 0){
		EventHandler::create(atoi(ssite));
	}
}

int run(void)
{
	while(1){
		EventHandler::poll();
	}
	return 0;
}

int main(int argc, const char** argv)
{
	BufferManager bm();
	ui(argc, argv);
	return run();
}

