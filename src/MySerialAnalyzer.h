#ifndef MYSERIAL_ANALYZER_H
#define MYSERIAL_ANALYZER_H

#include <Analyzer.h>
#include "MySerialAnalyzerResults.h"
#include "MySerialSimulationDataGenerator.h"

class MySerialAnalyzerSettings;
class MySerialAnalyzer : public Analyzer2
{
  public:
    MySerialAnalyzer();
    virtual ~MySerialAnalyzer();
    virtual void SetupResults();
    virtual void WorkerThread();

    virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
    virtual U32 GetMinimumSampleRateHz();

    virtual const char* GetAnalyzerName() const;
    virtual bool NeedsRerun();


#pragma warning( push )
#pragma warning(                                                                                                                           \
    disable : 4251 ) // warning C4251: 'MySerialAnalyzer::<...>' : class <...> needs to have dll-interface to be used by clients of class

  protected: // functions
    void ComputeSampleOffsets();

  protected: // vars
    std::auto_ptr<MySerialAnalyzerSettings> mSettings;
    std::auto_ptr<MySerialAnalyzerResults> mResults;
    AnalyzerChannelData* mSerial;

    MySerialSimulationDataGenerator mSimulationDataGenerator;
    bool mSimulationInitilized;

    // Serial analysis vars:
    U32 mSampleRateHz;
    std::vector<U32> mSampleOffsets;
    U32 mParityBitOffset;
    U32 mStartOfStopBitOffset;
    U32 mEndOfStopBitOffset;
    BitState mBitLow;
    BitState mBitHigh;

#pragma warning( pop )
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif // MYSERIAL_ANALYZER_H
