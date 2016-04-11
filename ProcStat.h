/*
 * ProcStat.h
 *
 *  Created on: Mar 14, 2016
 *      Author: root
 */

#ifndef PROCSTAT_H_
#define PROCSTAT_H_

class ProcStat {
public:
	ProcStat();
	~ProcStat();
public:
	float GetUser();
	float GetNice();
	float GetSystem();
	float GetIdle();
	int GetTotalMem();
	int GetFreeMem();
private:
	void GetSystemInfo();
private:
	int    last_pid;
	double load_avg[3];
	int    p_total;
	int    p_active;     /* number of procs considered "active" */
	int    *procstates;
	int    *cpustates;
	int    *memory;
};

#endif /* PROCSTAT_H_ */
