#include "MySerialAnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <sstream>
#include <cstring>
#include <vector>

#pragma warning( disable : 4800 ) // warning C4800: 'U32' : forcing value to bool 'true' or 'false' (performance warning)

MySerialAnalyzerSettings::MySerialAnalyzerSettings()
    : mInputChannel( UNDEFINED_CHANNEL ),
      mBitRate( 9600 ),
      mBitsPerTransfer( 8 ),
      mStopBits( 1.0 ),
      mParity( AnalyzerEnums::None ),
      mShiftOrder( AnalyzerEnums::LsbFirst ),
      mInverted( false ),
      mUseAutobaud( false ),
      mSerialMode( MySerialAnalyzerEnums::Normal )
{
    mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mInputChannelInterface->SetTitleAndTooltip( "Input Channel", "My Standard Async Serial" );
    mInputChannelInterface->SetChannel( mInputChannel );

    mBitRateInterface.reset( new AnalyzerSettingInterfaceInteger() );
    mBitRateInterface->SetTitleAndTooltip( "Bit Rate (Bits/s)", "Specify the bit rate in bits per second." );
    mBitRateInterface->SetMax( 100000000 );
    mBitRateInterface->SetMin( 1 );
    mBitRateInterface->SetInteger( mBitRate );

    mBitRateChangeInterface.reset(new AnalyzerSettingInterfaceText() );
    mBitRateChangeInterface->SetTitleAndTooltip( "Bitrate changes", "Timestamp:Bitrate, separated by spaces." );
    mBitRateChangeInterface->SetTextType(AnalyzerSettingInterfaceText::NormalText);

    mUseAutobaudInterface.reset( new AnalyzerSettingInterfaceBool() );
    mUseAutobaudInterface->SetTitleAndTooltip(
        "", "With Autobaud turned on, the analyzer will run as usual, with the current bit rate.  At the same time, it will also keep "
            "track of the shortest pulse it detects. \nAfter analyzing all the data, if the bit rate implied by this shortest pulse is "
            "different by more than 10% from the specified bit rate, the bit rate will be changed and the analysis run again." );
    mUseAutobaudInterface->SetCheckBoxText( "Use Autobaud" );
    mUseAutobaudInterface->SetValue( mUseAutobaud );

    mBitsPerTransferInterface.reset( new AnalyzerSettingInterfaceNumberList() );
    mBitsPerTransferInterface->SetTitleAndTooltip( "Bits per Frame", "Select the number of bits per frame" );
    for( U32 i = 1; i <= 64; i++ )
    {
        std::stringstream ss;

        if( i == 1 )
            ss << "1 Bit per Transfer";
        else if( i == 8 )
            ss << "8 Bits per Transfer (Standard)";
        else
            ss << i << " Bits per Transfer";

        mBitsPerTransferInterface->AddNumber( i, ss.str().c_str(), "" );
    }
    mBitsPerTransferInterface->SetNumber( mBitsPerTransfer );


    mStopBitsInterface.reset( new AnalyzerSettingInterfaceNumberList() );
    mStopBitsInterface->SetTitleAndTooltip( "Stop Bits", "Specify the number of stop bits." );
    mStopBitsInterface->AddNumber( 1.0, "1 Stop Bit (Standard)", "" );
    mStopBitsInterface->AddNumber( 1.5, "1.5 Stop Bits", "" );
    mStopBitsInterface->AddNumber( 2.0, "2 Stop Bits", "" );
    mStopBitsInterface->SetNumber( mStopBits );


    mParityInterface.reset( new AnalyzerSettingInterfaceNumberList() );
    mParityInterface->SetTitleAndTooltip( "Parity Bit", "Specify None, Even, or Odd Parity." );
    mParityInterface->AddNumber( AnalyzerEnums::None, "No Parity Bit (Standard)", "" );
    mParityInterface->AddNumber( AnalyzerEnums::Even, "Even Parity Bit", "" );
    mParityInterface->AddNumber( AnalyzerEnums::Odd, "Odd Parity Bit", "" );
    mParityInterface->SetNumber( mParity );


    mShiftOrderInterface.reset( new AnalyzerSettingInterfaceNumberList() );
    mShiftOrderInterface->SetTitleAndTooltip( "Significant Bit",
                                              "Select if the most significant bit or least significant bit is transmitted first" );
    mShiftOrderInterface->AddNumber( AnalyzerEnums::LsbFirst, "Least Significant Bit Sent First (Standard)", "" );
    mShiftOrderInterface->AddNumber( AnalyzerEnums::MsbFirst, "Most Significant Bit Sent First", "" );
    mShiftOrderInterface->SetNumber( mShiftOrder );


    mInvertedInterface.reset( new AnalyzerSettingInterfaceNumberList() );
    mInvertedInterface->SetTitleAndTooltip( "Signal inversion", "Specify if the serial signal is inverted" );
    mInvertedInterface->AddNumber( false, "Non Inverted (Standard)", "" );
    mInvertedInterface->AddNumber( true, "Inverted", "" );

    mInvertedInterface->SetNumber( mInverted );
    enum Mode
    {
        Normal,
        MpModeRightZeroMeansAddress,
        MpModeRightOneMeansAddress,
        MpModeLeftZeroMeansAddress,
        MpModeLeftOneMeansAddress
    };

    mSerialModeInterface.reset( new AnalyzerSettingInterfaceNumberList() );
    mSerialModeInterface->SetTitleAndTooltip( "Mode", "" );
    mSerialModeInterface->AddNumber( MySerialAnalyzerEnums::Normal, "Normal", "" );
    mSerialModeInterface->AddNumber( MySerialAnalyzerEnums::MpModeMsbZeroMeansAddress, "MP - Address indicated by MSB=0",
                                     "Multi-processor, 9-bit serial" );
    mSerialModeInterface->AddNumber( MySerialAnalyzerEnums::MpModeMsbOneMeansAddress, "MDB - Address indicated by MSB=1 (TX only)",
                                     "Multi-drop, 9-bit serial" );
    mSerialModeInterface->SetNumber( mSerialMode );

    AddInterface( mInputChannelInterface.get() );
    AddInterface( mBitRateInterface.get() );
    AddInterface( mBitRateChangeInterface.get() );
    AddInterface( mBitsPerTransferInterface.get() );
    AddInterface( mStopBitsInterface.get() );
    AddInterface( mParityInterface.get() );
    AddInterface( mShiftOrderInterface.get() );
    AddInterface( mInvertedInterface.get() );
    AddInterface( mSerialModeInterface.get() );

    // AddExportOption( 0, "Export as text/csv file", "text (*.txt);;csv (*.csv)" );
    AddExportOption( 0, "Export as text/csv file" );
    AddExportExtension( 0, "text", "txt" );
    AddExportExtension( 0, "csv", "csv" );

    ClearChannels();
    AddChannel( mInputChannel, "Serial", false );
}

MySerialAnalyzerSettings::~MySerialAnalyzerSettings()
{
}

/*
 * Fills mBRChange map according to mBitRateChange string contents
 * eg: if mBitRatechange contains "0:9600 7.765:250000"
 * mBRChange will contain { 0 => 9600, 7.765 => 250000}
 */
void MySerialAnalyzerSettings::SyncBitRateChange()
{
    mBRChange.clear() ;

    if (mBitRateChangeStr.empty()) return ;
   
    std::string s=mBitRateChangeStr;
    std::string delimiter = " " ;
    std::vector<std::string> token;
    
    size_t pos = 0 ;

    float time ;
    U32 bitrate ;

    // parse string separated by spaces, and fill array of found tokens
    while ((pos = s.find(delimiter)) != std::string::npos) {
        std::string t=s.substr(0,pos);
        // std::cout << "token:{" << t << "}" << std::endl ;
        s.erase(0,pos+delimiter.length());
        
        if (t.empty()) { // multiple spaces
            continue ;
        }

        token.push_back(t) ;
    }

    // remaining of the string after last seen separator
    if (!s.empty()) {
        token.push_back(s) ;
    }

    // parse every token and fill mBRChange
    for (const std::string&t : token) {
        if (sscanf(t.c_str(),"%f:%u",&time, &bitrate) != 2) { // parsing error
            // std::cout << "parsing error {" << token[mszBRChange] << "}" << std::endl ;
            //TODO: display error to user.
            continue;
        }
        
        mBRChange[time] = bitrate ;
    }

    return ;
}

bool MySerialAnalyzerSettings::SetSettingsFromInterfaces()
{
    if( AnalyzerEnums::Parity( U32( mParityInterface->GetNumber() ) ) != AnalyzerEnums::None )
        if( MySerialAnalyzerEnums::Mode( U32( mSerialModeInterface->GetNumber() ) ) != MySerialAnalyzerEnums::Normal )
        {
            SetErrorText( "Sorry, but we don't support using parity at the same time as MP mode." );
            return false;
        }

    mInputChannel = mInputChannelInterface->GetChannel();
    mBitRate = mBitRateInterface->GetInteger();
    mBitRateChangeStr = mBitRateChangeInterface->GetText();
    SyncBitRateChange() ;

    mBitsPerTransfer = U32( mBitsPerTransferInterface->GetNumber() );
    mStopBits = mStopBitsInterface->GetNumber();
    mParity = AnalyzerEnums::Parity( U32( mParityInterface->GetNumber() ) );
    mShiftOrder = AnalyzerEnums::ShiftOrder( U32( mShiftOrderInterface->GetNumber() ) );
    mInverted = bool( U32( mInvertedInterface->GetNumber() ) );
    mUseAutobaud = mUseAutobaudInterface->GetValue();
    mSerialMode = MySerialAnalyzerEnums::Mode( U32( mSerialModeInterface->GetNumber() ) );

    ClearChannels();
    AddChannel( mInputChannel, "Serial", true );

    return true;
}

void MySerialAnalyzerSettings::UpdateInterfacesFromSettings()
{
    mInputChannelInterface->SetChannel( mInputChannel );
    mBitRateInterface->SetInteger( mBitRate );
    mBitRateChangeInterface->SetText( mBitRateChangeStr.c_str()) ;
    mBitsPerTransferInterface->SetNumber( mBitsPerTransfer );
    mStopBitsInterface->SetNumber( mStopBits );
    mParityInterface->SetNumber( mParity );
    mShiftOrderInterface->SetNumber( mShiftOrder );
    mInvertedInterface->SetNumber( mInverted );
    mUseAutobaudInterface->SetValue( mUseAutobaud );
    mSerialModeInterface->SetNumber( mSerialMode );
}

void MySerialAnalyzerSettings::LoadSettings( const char* settings )
{
    SimpleArchive text_archive;
    text_archive.SetString( settings );

    const char* name_string; // the first thing in the archive is the name of the protocol analyzer that the data belongs to.
    text_archive >> &name_string;
    if( strcmp( name_string, "SaleaeAsyncMySerialAnalyzer" ) != 0 )
        AnalyzerHelpers::Assert( "SaleaeAsyncMySerialAnalyzer: Provided with a settings string that doesn't belong to us;" );

    text_archive >> mInputChannel;
    text_archive >> mBitRate;
    text_archive >> mBitsPerTransfer;
    text_archive >> mStopBits;
    text_archive >> *( U32* )&mParity;
    text_archive >> *( U32* )&mShiftOrder;
    text_archive >> mInverted;

    // check to make sure loading it actual works befor assigning the result -- do this when adding settings to an anylzer which has been
    // previously released.
    bool use_autobaud;
    if( text_archive >> use_autobaud )
        mUseAutobaud = use_autobaud;

    MySerialAnalyzerEnums::Mode mode;
    if( text_archive >> *( U32* )&mode )
        mSerialMode = mode;

    const char * bitratechange_string ;
    if ( text_archive >> &bitratechange_string ) {
        mBitRateChangeStr = name_string ;
        SyncBitRateChange() ;
    }

    ClearChannels();
    AddChannel( mInputChannel, "Serial", true );

    UpdateInterfacesFromSettings();
}

const char* MySerialAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << "SaleaeAsyncMySerialAnalyzer";
    text_archive << mInputChannel;
    text_archive << mBitRate;
    text_archive << mBitsPerTransfer;
    text_archive << mStopBits;
    text_archive << mParity;
    text_archive << mShiftOrder;
    text_archive << mInverted;

    text_archive << mUseAutobaud;

    text_archive << mSerialMode;
    text_archive << mBitRateChangeStr.c_str() ;

    return SetReturnString( text_archive.GetString() );
}
