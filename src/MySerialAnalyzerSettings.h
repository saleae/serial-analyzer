#ifndef MYSERIAL_ANALYZER_SETTINGS
#define MYSERIAL_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>
#include <map>

namespace MySerialAnalyzerEnums
{
    enum Mode
    {
        Normal,
        MpModeMsbZeroMeansAddress,
        MpModeMsbOneMeansAddress
    };
};

typedef std::map<float, U32> BRTime ;

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

    std::string mBitRateChangeStr; // string containing when to change bitrate, in the form time:bitrate, separated by spaces
    
    // this will contain when to change baudrate. Keys: timestamp, value: baudrate to set.
    BRTime mBRChange ; 
    
    U32 mBitsPerTransfer;
    AnalyzerEnums::ShiftOrder mShiftOrder;
    double mStopBits;
    AnalyzerEnums::Parity mParity;
    bool mInverted;
    bool mUseAutobaud;
    MySerialAnalyzerEnums::Mode mSerialMode;

  protected:
    void SyncBitRateChange();
    std::auto_ptr<AnalyzerSettingInterfaceChannel> mInputChannelInterface;
    std::auto_ptr<AnalyzerSettingInterfaceInteger> mBitRateInterface;
    std::auto_ptr<AnalyzerSettingInterfaceText> mBitRateChangeInterface;
    std::auto_ptr<AnalyzerSettingInterfaceNumberList> mBitsPerTransferInterface;
    std::auto_ptr<AnalyzerSettingInterfaceNumberList> mShiftOrderInterface;
    std::auto_ptr<AnalyzerSettingInterfaceNumberList> mStopBitsInterface;
    std::auto_ptr<AnalyzerSettingInterfaceNumberList> mParityInterface;
    std::auto_ptr<AnalyzerSettingInterfaceNumberList> mInvertedInterface;
    std::auto_ptr<AnalyzerSettingInterfaceBool> mUseAutobaudInterface;
    std::auto_ptr<AnalyzerSettingInterfaceNumberList> mSerialModeInterface;
};

#endif // MYSERIAL_ANALYZER_SETTINGS
