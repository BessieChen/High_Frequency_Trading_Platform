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

//"IF2102,IF2103",string类型转换为你char *
void MdSpi::setInstIdList_Position_MD(std::string& inst_holding)
{
	m_InstIdList_Position_MD = new char[inst_holding.length()];

	memcpy(m_InstIdList_Position_MD, inst_holding.c_str(), inst_holding.length());

}

void MdSpi::set_InstIdList_All(std::string& inst_all)
{
}
