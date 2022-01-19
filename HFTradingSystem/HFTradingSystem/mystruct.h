#pragma once
#include<string>
#include<vector>

//仓位信息结构体
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
//tick数据
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


//Kbar数据
struct Kbar
{
	std::string date;
	std::string time;
	//时间周期 多少分钟
	int Internval;
	double open;
	double high;
	double low;
	double close;
	double vol;
	double openInterest;
};
//读取k线
void ReadKbarSeries(std::string fileName, std::vector<Kbar>& kbar_vec);

//保存文件名
void Save_FileName(std::string path, std::vector<std::string>& fileName_vec);

void StringToCharPP(std::string & str,char *  pp);