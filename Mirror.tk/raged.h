#pragma once
#pragma once

#ifndef _CPREDICTION_H_
#define _CPREDICTION_H_
#include "sdk.h"
#include <vector>

class cPrediction
{
private:
	typedef struct matrixentry_s
	{
		std::vector < Vector > vecOrigins;	
		Vector	vOrigin;				
		DWORD	dwTime;					
		DWORD	dwFraction;
		int simulation;
		int oldsimulation;
		int simulationdifference;
	}matrixentry_t;
public:
	void ReportMove(Vector  vOrigin, int iIndex);
	void Reset(void);							
	bool bGetCurOrigin(Vector& vOrigin, int iIndex);
	bool bGetOrigin(Vector& vOrigin, int iIndex, DWORD dwTime);
	void UpdatePlayerPos();
protected:
	matrixentry_t m_Matrix[128];
};
extern cPrediction DataManager;
#endif