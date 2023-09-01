#ifndef MYSERIAL_ANALYZER_SETTINGS
#define MYSERIAL_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

namespace MySerialAnalyzerEnums
{
    enum Mode
    {
        Normal,
        MpModeMsbZeroMeansAddress,
        MpModeMsbOneMeansAddress
    };
};

class MySerialAnalyzerSettings : public AnalyzerSettings
{
  public:
    MySerialAnalyzerSettings();
    virtual ~MySerialAnalyzerSettings();

    virtual bool SetSettingsFromInterfaces();
    void UpdateInterfacesFromSettings();
    virtual void LoadSettings( const char* settings );
    virtual const char* SaveSettings();


    Channel mInputChannel;
    U32 mBitRate;
    U32 mBitsPerTransfer;
    AnalyzerEnums::ShiftOrder mShiftOrder;
    double mStopBits;
    AnalyzerEnums::Parity mParity;
    bool mInverted;
    bool mUseAutobaud;
    MySerialAnalyzerEnums::Mode mSerialMode;

  protected:
    std::auto_ptr<AnalyzerSettingInterfaceChannel> mInputChannelInterface;
    std::auto_ptr<AnalyzerSettingInterfaceInteger> mBitRateInterface;
    std::auto_ptr<AnalyzerSettingInterfaceNumberList> mBitsPerTransferInterface;
    std::auto_ptr<AnalyzerSettingInterfaceNumberList> mShiftOrderInterface;
    std::auto_ptr<AnalyzerSettingInterfaceNumberList> mStopBitsInterface;
    std::auto_ptr<AnalyzerSettingInterfaceNumberList> mParityInterface;
    std::auto_ptr<AnalyzerSettingInterfaceNumberList> mInvertedInterface;
    std::auto_ptr<AnalyzerSettingInterfaceBool> mUseAutobaudInterface;
    std::auto_ptr<AnalyzerSettingInterfaceNumberList> mSerialModeInterface;
};

#endif // MYSERIAL_ANALYZER_SETTINGS
