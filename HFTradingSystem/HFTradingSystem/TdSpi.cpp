#include "TdSpi.h"

TdSpi::TdSpi(CThostFtdcTraderApi* tdapi, CThostFtdcMdApi* pUserApi_md, MdSpi* pUserSpi_md):
	m_pUserTDApi_trade(tdapi), m_pUserMDApi_trade(pUserApi_md), m_pUserMDSpi_trade(pUserSpi_md)

{

	
}

void TdSpi::OnFrontConnected()
{
}

int TdSpi::ReqUserLogin()
{
	return 0;
}

int TdSpi::ReqAuthenticate()
{
	return 0;
}

void TdSpi::ReqQryOrder()
{
}

void TdSpi::ReqQryTrade()
{
}

void TdSpi::ReqQryInvestorPositionDetail()
{
}

void TdSpi::ReqQryTradingAccount()
{
}

void TdSpi::ReqQryInvestorPosition_All()
{
}

void TdSpi::ReqQryInvestorPosition()
{
}

void TdSpi::ReqQryInstrumetAll()
{
}

void TdSpi::ReqQryInstrumet()
{
}

void TdSpi::ReqSettlementInfoConfirm()
{
}

void TdSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

bool TdSpi::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo)
{
	return false;
}

void TdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField* pSettlementInfo, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField* pUserPasswordUpdate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryTrade(CThostFtdcTradeField* pTrade, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField* pField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
}

void TdSpi::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
}



void TdSpi::CancelOrder(CThostFtdcOrderField* pOrder)
{
}

void TdSpi::ShowPosition()
{
}

void TdSpi::ClosePosition()
{
}

void TdSpi::SetAllowOpen(bool isOk)
{
}

CThostFtdcOrderField* TdSpi::GetOrder(int nBrokerOrderSeq)
{
	return nullptr;
}

bool TdSpi::UpdateOrder(CThostFtdcOrderField* pOrder)
{
	return false;
}

void TdSpi::PlaceOrder(const char* pszCode, const char* ExchangeID, int nDirection, int nOpenClose, int nVolume, double fPrice)
{
}
