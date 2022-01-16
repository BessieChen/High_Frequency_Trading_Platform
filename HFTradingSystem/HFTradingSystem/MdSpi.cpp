#include"MdSpi.h"

MdSpi::MdSpi(CThostFtdcMdApi* mdapi):mdapi(mdapi)
{
}

MdSpi::~MdSpi()
{
}

void MdSpi::OnFrontConnected()
{
}

void MdSpi::ReqUserLogin(std::string brokerId, std::string userId, std::string passwd)
{
}

void MdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void MdSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void MdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void MdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void MdSpi::SubscribeMarketData(char* instIdList)
{
}

void MdSpi::SubscribeMarketData(std::string instIdList)
{
}

void MdSpi::SubscribeMarketData_All()
{
}

void MdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)
{
}

void MdSpi::setInstIdList_Position_MD(std::string& inst_holding)
{
}

void MdSpi::set_InstIdList_All(std::string& inst_all)
{
}
