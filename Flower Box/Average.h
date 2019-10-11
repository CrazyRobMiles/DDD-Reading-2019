#pragma once

//#define DEBUG_AVERAGE

class Average
{
private:


	float average;
	float total;
	int averageCount;
	int NoOfValuesToAverage;

public:

	void setNoOfValuesToAverage(int NoOfValuesToAverage);
	bool addToAverage(float value);

	void restartAverage();

	bool averageReady();

	bool getAverage(float * result);

	Average(int NoOfValuesToAverage);
	~Average();
};

