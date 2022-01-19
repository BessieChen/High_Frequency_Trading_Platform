#pragma once
#include<string>
#include<vector>

//��λ��Ϣ�ṹ��
struct position_field
{
	std::string instId;
	unsigned int LongPosition;
	unsigned int TodayLongPosition;
	unsigned int YdLongPosition;
	double LongCloseProfit;
	double LongPositionProfit;
	unsigned int ShortPosition;
	unsigned int TodayShortPosition;
	unsigned int YdShortPosition;
	double ShortCloseProfit;
	double ShortPositionProfit;
	double lastPrice;
	double PreSettlementPrice;

};
//tick����
struct Tick
{
	std::string date;
	std::string time;
	double bidPrice;
	int bidVolume;
	double askPrice;
	int askVolume;
	double vol;
	double openInterest;
};


//Kbar����
struct Kbar
{
	std::string date;
	std::string time;
	//ʱ������ ���ٷ���
	int Internval;
	double open;
	double high;
	double low;
	double close;
	double vol;
	double openInterest;
};
//��ȡk��
void ReadKbarSeries(std::string fileName, std::vector<Kbar>& kbar_vec);

//�����ļ���
void Save_FileName(std::string path, std::vector<std::string>& fileName_vec);

void StringToCharPP(std::string & str,char *  pp);