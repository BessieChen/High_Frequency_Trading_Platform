#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include"TdSpi.h"

extern std::map<std::string, std::string> accountConfig_map;//保存账户信息的map

class Strategy
{
public:
    Strategy(TdSpi* pUserSpi_trade) :m_pUserTDSpi_stgy(pUserSpi_trade)
    {
        strcpy(m_instId, accountConfig_map["contract"].c_str());
        tickCount = 0;
    }
    //tick事件
    void OnTick(CThostFtdcDepthMarketDataField* pDethMD);
    //策略启动事件
    void OnStrategyStart();
    //策略停止事件
    void OnStrategyEnd();
    //k线事件
    void OnBar();
    //订单状态事件
    void OnRtnOrder(CThostFtdcOrderField* pOrder);
    //成交事件
    void OnRtnTrade(CThostFtdcTradeField* pTrade);
    //撤单操作
    void CancelOrder(CThostFtdcOrderField* pOrder);
    //注册计时器，做高频交易时，需要多少毫秒以后不成交撤单，需要计时器
    void RegisterTimer(int milliSeconds, int nAction, CThostFtdcOrderField* pOrder);
    //计时器通知时间到了
    void OnTimer(CThostFtdcOrderField* pOrder, long lData);
    
    //buy open
    CThostFtdcOrderField* Buy(const char* InstrumentID, const char* ExchangeID, int nVolume, double fPrice);

    //sell close
    CThostFtdcOrderField* Sell(const char* InstrumentID, const char* ExchangeID, int nVolume, double fPrice, char YdorToday = '0');

    //sell open;
    CThostFtdcOrderField* Short(const char* InstrumentID, const char* ExchangeID, int nVolume, double fPrice);

    // buy to close;
    CThostFtdcOrderField* BuytoCover(const char* InstrumentID, const char* ExchangeID, int nVolume, double fPrice, char YdorToday = '0');

    //把持仓的合约设置到策略里面
    void set_instPostion_map_stgy(std::map<std::string, CThostFtdcInstrumentField*> inst_map);
private:

    std::map<std::string, CThostFtdcInstrumentField*> m_instField_map;
    TdSpi* m_pUserTDSpi_stgy;
    //计算账户盈亏
    void CalculateProfitInfo(CThostFtdcDepthMarketDataField* pDethMD);
    
    //保存tick数据到vector
    void SaveTickToVec(CThostFtdcDepthMarketDataField* pDethMD);

    //保存tick数据到txt和csv
    void SaveTickToTxtCsv(CThostFtdcDepthMarketDataField* pDethMD);
    
    //计算开仓平仓信号
    void CalculateBuySellSignal(CThostFtdcDepthMarketDataField* pDethMD);


    char m_instId[32];
    int tickCount;

};