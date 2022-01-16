#include "Strategy.h"

void Strategy::OnTick(CThostFtdcDepthMarketDataField* pDethMD)
{
}

void Strategy::OnStrategyStart()
{
}

void Strategy::OnStrategyEnd()
{
}

void Strategy::OnBar()
{
}

void Strategy::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
}

void Strategy::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
}

void Strategy::CancelOrder(CThostFtdcOrderField* pOrder)
{
}

void Strategy::RegisterTimer(int milliSeconds, int nAction, CThostFtdcOrderField* pOrder)
{
}

void Strategy::OnTimer(CThostFtdcOrderField* pOrder, long lData)
{
}

CThostFtdcOrderField* Strategy::Buy(const char* InstrumentID, const char* ExchangeID, int nVolume, double fPrice)
{
	return nullptr;
}

CThostFtdcOrderField* Strategy::Sell(const char* InstrumentID, const char* ExchangeID, int nVolume, double fPrice, char YdorToday)
{
	return nullptr;
}

CThostFtdcOrderField* Strategy::Short(const char* InstrumentID, const char* ExchangeID, int nVolume, double fPrice)
{
	return nullptr;
}

CThostFtdcOrderField* Strategy::BuytoCover(const char* InstrumentID, const char* ExchangeID, int nVolume, double fPrice, char YdorToday)
{
	return nullptr;
}

void Strategy::set_instPostion_map_stgy(std::map<std::string, CThostFtdcInstrumentField*> inst_map)
{
}

void Strategy::CalculateProfitInfo(CThostFtdcDepthMarketDataField* pDethMD)
{
}

void Strategy::SaveTickToVec(CThostFtdcDepthMarketDataField* pDethMD)
{
}

void Strategy::SaveTickToTxtCsv(CThostFtdcDepthMarketDataField* pDethMD)
{
}

void Strategy::CalculateBuySellSignal(CThostFtdcDepthMarketDataField* pDethMD)
{
}
