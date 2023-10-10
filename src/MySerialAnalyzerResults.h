#ifndef MYSERIAL_ANALYZER_RESULTS
#define MYSERIAL_ANALYZER_RESULTS

#include <AnalyzerResults.h>

#define FRAMING_ERROR_FLAG ( 1 << 0 )
#define PARITY_ERROR_FLAG ( 1 << 1 )
#define MP_MODE_ADDRESS_FLAG ( 1 << 2 )

class MySerialAnalyzer;
class MySerialAnalyzerSettings;

class MySerialAnalyzerResults : public AnalyzerResults
{
  public:
    MySerialAnalyzerResults( MySerialAnalyzer* analyzer, MySerialAnalyzerSettings* settings );
    virtual ~MySerialAnalyzerResults();

    virtual void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base );
    virtual void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id );

    virtual void GenerateFrameTabularText( U64 frame_index, DisplayBase display_base );
    virtual void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base );
    virtual void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base );

  protected: // functions
  protected: // vars
    MySerialAnalyzerSettings* mSettings;
    MySerialAnalyzer* mAnalyzer;
};

#endif // MYSERIAL_ANALYZER_RESULTS
