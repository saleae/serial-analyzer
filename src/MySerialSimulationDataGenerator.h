#ifndef MYSERIAL_SIMULATION_DATA_GENERATOR
#define MYSERIAL_SIMULATION_DATA_GENERATOR

#include <AnalyzerHelpers.h>

class MySerialAnalyzerSettings;

class MySerialSimulationDataGenerator
{
  public:
    MySerialSimulationDataGenerator();
    ~MySerialSimulationDataGenerator();

    void Initialize( U32 simulation_sample_rate, MySerialAnalyzerSettings* settings );
    U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channelss );

  protected:
    MySerialAnalyzerSettings* mSettings;
    U32 mSimulationSampleRateHz;
    BitState mBitLow;
    BitState mBitHigh;
    U64 mValue;

    U64 mMpModeAddressMask;
    U64 mMpModeDataMask;
    U64 mNumBitsMask;

  protected: // Serial specific
    void CreateSerialByte( U64 value );
    ClockGenerator mClockGenerator;
    SimulationChannelDescriptor mSerialSimulationData; // if we had more than one channel to simulate, they would need to be in an array
};
#endif // UNIO_SIMULATION_DATA_GENERATOR
