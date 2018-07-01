#ifndef SERIAL_ANALYZER_SETTINGS
#define SERIAL_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

namespace SerialAnalyzerEnums
{
	enum Mode { Normal, MpModeMsbZeroMeansAddress, MpModeMsbOneMeansAddress };
};

class SerialAnalyzerSettings : public AnalyzerSettings
{
public:
	SerialAnalyzerSettings();
	virtual ~SerialAnalyzerSettings();

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
	SerialAnalyzerEnums::Mode mSerialMode;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mBitsPerTransferInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mShiftOrderInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mStopBitsInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mParityInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mInvertedInterface;
	std::auto_ptr< AnalyzerSettingInterfaceBool >	mUseAutobaudInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mSerialModeInterface;
};

#endif //SERIAL_ANALYZER_SETTINGS
