#include "MarketDataSpi.h"

MarketDataSpi::MarketDataSpi(CThostFtdcMdApi* mdapi)
{
}

MarketDataSpi::~MarketDataSpi()
{
}

void MarketDataSpi::OnFrontConnected()
{
}

void MarketDataSpi::ReqUserLogin(std::string brokerId, std::string userId, std::string passwd)
{
}

void MarketDataSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void MarketDataSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void MarketDataSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void MarketDataSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void MarketDataSpi::SubscribeMarketData(char* instIdList)
{
}

void MarketDataSpi::SubscribeMarketData(std::string instIdList)
{
}

void MarketDataSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)
{
}

void MarketDataSpi::SubscribeAllMarketData()
{
}

void MarketDataSpi::setHoldingInstIdList(std::string& instHolding)
{
}

void MarketDataSpi::setAllInstIdList(std::string& instAll)
{
}
