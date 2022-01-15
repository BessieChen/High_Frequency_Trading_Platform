#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include"TdSpi.h"

extern std::map<std::string, std::string> accountConfig_map;//�����˻���Ϣ��map

class Strategy
{
public:
    Strategy(TdSpi* pUserSpi_trade) :m_pUserTDSpi_stgy(pUserSpi_trade)
    {
        strcpy(m_instId, accountConfig_map["contract"].c_str());
        tickCount = 0;
    }
    //tick�¼�
    void OnTick(CThostFtdcDepthMarketDataField* pDethMD);
    //���������¼�
    void OnStrategyStart();
    //����ֹͣ�¼�
    void OnStrategyEnd();
    //k���¼�
    void OnBar();
    //����״̬�¼�
    void OnRtnOrder(CThostFtdcOrderField* pOrder);
    //�ɽ��¼�
    void OnRtnTrade(CThostFtdcTradeField* pTrade);
    //��������
    void CancelOrder(CThostFtdcOrderField* pOrder);
    //ע���ʱ��������Ƶ����ʱ����Ҫ���ٺ����Ժ󲻳ɽ���������Ҫ��ʱ��
    void RegisterTimer(int milliSeconds, int nAction, CThostFtdcOrderField* pOrder);
    //��ʱ��֪ͨʱ�䵽��
    void OnTimer(CThostFtdcOrderField* pOrder, long lData);
    
    //buy open
    CThostFtdcOrderField* Buy(const char* InstrumentID, const char* ExchangeID, int nVolume, double fPrice);

    //sell close
    CThostFtdcOrderField* Sell(const char* InstrumentID, const char* ExchangeID, int nVolume, double fPrice, char YdorToday = '0');

    //sell open;
    CThostFtdcOrderField* Short(const char* InstrumentID, const char* ExchangeID, int nVolume, double fPrice);

    // buy to close;
    CThostFtdcOrderField* BuytoCover(const char* InstrumentID, const char* ExchangeID, int nVolume, double fPrice, char YdorToday = '0');

    //�ѳֲֵĺ�Լ���õ���������
    void set_instPostion_map_stgy(std::map<std::string, CThostFtdcInstrumentField*> inst_map);
private:

    std::map<std::string, CThostFtdcInstrumentField*> m_instField_map;
    TdSpi* m_pUserTDSpi_stgy;
    //�����˻�ӯ��
    void CalculateProfitInfo(CThostFtdcDepthMarketDataField* pDethMD);
    
    //����tick���ݵ�vector
    void SaveTickToVec(CThostFtdcDepthMarketDataField* pDethMD);

    //����tick���ݵ�txt��csv
    void SaveTickToTxtCsv(CThostFtdcDepthMarketDataField* pDethMD);
    
    //���㿪��ƽ���ź�
    void CalculateBuySellSignal(CThostFtdcDepthMarketDataField* pDethMD);


    char m_instId[32];
    int tickCount;

};