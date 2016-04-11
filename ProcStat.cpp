/*
 * ProcStat.cpp
 *
 *  Created on: Mar 14, 2016
 *      Author: root
 */

#include "ProcStat.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#define NCPUSTATES 4

#define NMEMSTATS 6

static long cp_time[NCPUSTATES];
static long cp_old[NCPUSTATES];
static long cp_diff[NCPUSTATES];
static int cpu_states[NCPUSTATES];
static int memory_stats[NMEMSTATS];

static inline char *
skip_ws(const char *p)
{
    while (isspace(*p)) p++;
    return (char *)p;
}

static inline char *
skip_token(const char *p)
{
    while (isspace(*p)) p++;
    while (*p && !isspace(*p)) p++;
    return (char *)p;
}


long percentages(int cnt, int *out, register long * newC, register long *oldC, long *diffs)
{
    register int i;
    register long change;
    register long total_change;
    register long *dp;
    long half_total;

    /* initialization */
    total_change = 0;
    dp = diffs;

    /* calculate changes for each state and the overall change */
    for (i = 0; i < cnt; i++)
    {
        if ((change = *newC - *oldC) < 0)
        {
            /* this only happens when the counter wraps */
            //change = (int)((unsigned long)*newC-(unsigned long)*oldC);
            change = *oldC - *newC;
        }
        total_change += (*dp++ = change);
        *oldC++ = *newC++;
    }

    /* avoid divide by zero potential */
    if (total_change == 0)
    {
    total_change = 1;
    }

    /* calculate percentages based on overall change, rounding up */
    half_total = 0.0;//total_change / 2l;
    for (i = 0; i < cnt; i++)
    {
    *out++ = (int)((*diffs++ * 1000 + half_total) / total_change);
    }

    /* return the total in case the caller wants to use it */
    return(total_change);
}

ProcStat::ProcStat() {
	// TODO Auto-generated constructor stub

}

ProcStat::~ProcStat() {
	// TODO Auto-generated destructor stub
}

float ProcStat::GetUser()
{
	return (float)cpustates[0]/10.0;
}

float ProcStat::GetNice()
{
	return (float)cpustates[1]/10.0;
}

float ProcStat::GetSystem()
{
	return (float)cpustates[2]/10.0;
}

float ProcStat::GetIdle()
{
	return (float)cpustates[3]/10.0;
}

int ProcStat::GetTotalMem()
{
	return memory[0];
}
int ProcStat::GetFreeMem()
{
	return memory[1];
}

void ProcStat::GetSystemInfo()
{
	char buffer[4096+1];
	int fd, len;
	char *p;
	int i;

	/* get load averages */
	{
	    fd = open("/proc/loadavg", O_RDONLY);
	    len = read(fd, buffer, sizeof(buffer)-1);
	    close(fd);
	    buffer[len] = '\0';

	    load_avg[0] = strtod(buffer, &p);
	    load_avg[1] = strtod(p, &p);
	    load_avg[2] = strtod(p, &p);
	    p = skip_token(p);			/* skip running/tasks */
	    p = skip_ws(p);
	    if (*p)
	    {
	    	last_pid = atoi(p);
	    }
	    else
	    {
	    	last_pid = -1;
	    }
	}

	/* get the cpu time info */
	{
		fd = open("/proc/stat", O_RDONLY);
		len = read(fd, buffer, sizeof(buffer)-1);
		close(fd);
		buffer[len] = '\0';

		p = skip_token(buffer);			/* "cpu" */
		cp_time[0] = strtoul(p, &p, 0);
		cp_time[1] = strtoul(p, &p, 0);
		cp_time[2] = strtoul(p, &p, 0);
		cp_time[3] = strtoul(p, &p, 0);

		/* convert cp_time counts to percentages */
		percentages(4, cpu_states, cp_time, cp_old, cp_diff);
	}

	/* get system wide memory usage */
	{
		char *p;
		fd = open("/proc/meminfo", O_RDONLY);
		len = read(fd, buffer, sizeof(buffer)-1);
		close(fd);
		buffer[len] = '\0';

		/* be prepared for extra columns to appear be seeking
		to ends of lines */

		p = buffer;
		p = skip_token(p);
		memory_stats[0] = strtoul(p, &p, 10); /* total memory */

		p = strchr(p, '\n');
		p = skip_token(p);
		memory_stats[1] = strtoul(p, &p, 10); /* free memory */

		p = strchr(p, '\n');
		p = skip_token(p);
		memory_stats[2] = strtoul(p, &p, 10); /* buffer memory */

		p = strchr(p, '\n');
		p = skip_token(p);
		memory_stats[3] = strtoul(p, &p, 10); /* cached memory */

		for(i = 0; i< 8 ;i++)
		{
				p++;
				p = strchr(p, '\n');
		}

		p = skip_token(p);
		memory_stats[4] = strtoul(p, &p, 10); /* total swap */

		p = strchr(p, '\n');
		p = skip_token(p);
		memory_stats[5] = strtoul(p, &p, 10); /* free swap */

	}

	/* set arrays and strings */
	cpustates = cpu_states;
	memory = memory_stats;
}

