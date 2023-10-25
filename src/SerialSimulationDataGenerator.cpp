#include "SerialSimulationDataGenerator.h"
#include "SerialAnalyzerSettings.h"

SerialSimulationDataGenerator::SerialSimulationDataGenerator() = default;

SerialSimulationDataGenerator::~SerialSimulationDataGenerator() = default;

void SerialSimulationDataGenerator::Initialize( U32 simulation_sample_rate, SerialAnalyzerSettings* settings )
{
    mSimulationSampleRateHz = simulation_sample_rate;
    mSettings = settings;

    mClockGenerator.Init( mSettings->mBitRate, simulation_sample_rate );
    mSerialSimulationData.SetChannel( mSettings->mInputChannel );
    mSerialSimulationData.SetSampleRate( simulation_sample_rate );

    if( mSettings->mInverted == false )
    {
        mBitLow = BIT_LOW;
        mBitHigh = BIT_HIGH;
    }
    else
    {
        mBitLow = BIT_HIGH;
        mBitHigh = BIT_LOW;
    }

    mSerialSimulationData.SetInitialBitState( mBitHigh );
    mSerialSimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod( 10.0 ) ); // insert 10 bit-periods of idle

    mValue = 0;

    mMpModeAddressMask = 0;
    mMpModeDataMask = 0;
    mNumBitsMask = 0;

    U32 num_bits = mSettings->mBitsPerTransfer;
    for( U32 i = 0; i < num_bits; i++ )
    {
        mNumBitsMask <<= 1;
        mNumBitsMask |= 0x1;
    }

    if( mSettings->mSerialMode == SerialAnalyzerEnums::MpModeMsbOneMeansAddress )
        mMpModeAddressMask = 0x1ull << ( mSettings->mBitsPerTransfer );

    if( mSettings->mSerialMode == SerialAnalyzerEnums::MpModeMsbZeroMeansAddress )
        mMpModeDataMask = 0x1ull << ( mSettings->mBitsPerTransfer );
}

U32 SerialSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate,
                                                           SimulationChannelDescriptor** simulation_channels )
{
    U64 adjusted_largest_sample_requested =
        AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

    while( mSerialSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
    {
        if( mSettings->mSerialMode == SerialAnalyzerEnums::Normal )
        {
            CreateSerialByte( mValue++ );

            mSerialSimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod( 10.0 ) ); // insert 10 bit-periods of idle
        }
        else
        {
            U64 address = 0x1 | mMpModeAddressMask;
            CreateSerialByte( address );

            for( U32 i = 0; i < 4; i++ )
            {
                mSerialSimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod( 2.0 ) ); // insert 2 bit-periods of idle
                CreateSerialByte( ( mValue++ & mNumBitsMask ) | mMpModeDataMask );
            }

            mSerialSimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod( 20.0 ) ); // insert 20 bit-periods of idle

            address = 0x2 | mMpModeAddressMask;
            CreateSerialByte( address );

            for( U32 i = 0; i < 4; i++ )
            {
                mSerialSimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod( 2.0 ) ); // insert 2 bit-periods of idle
                CreateSerialByte( ( mValue++ & mNumBitsMask ) | mMpModeDataMask );
            }

            mSerialSimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod( 20.0 ) ); // insert 20 bit-periods of idle
        }
    }

    *simulation_channels = &mSerialSimulationData;


    return 1; // we are retuning the size of the SimulationChannelDescriptor array.  In our case, the "array" is length 1.
}

void SerialSimulationDataGenerator::CreateSerialByte( U64 value )
{
    // assume we start high

    mSerialSimulationData.Transition();                                     // low-going edge for start bit
    mSerialSimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod() ); // add start bit time

    if( mSettings->mInverted == true )
        value = ~value;

    U32 num_bits = mSettings->mBitsPerTransfer;
    if( mSettings->mSerialMode != SerialAnalyzerEnums::Normal )
        num_bits++;

    BitExtractor bit_extractor( value, mSettings->mShiftOrder, num_bits );

    for( U32 i = 0; i < num_bits; i++ )
    {
        mSerialSimulationData.TransitionIfNeeded( bit_extractor.GetNextBit() );
        mSerialSimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod() );
    }

    if( mSettings->mParity == AnalyzerEnums::Even )
    {
        if( AnalyzerHelpers::IsEven( AnalyzerHelpers::GetOnesCount( value ) ) == true )
            mSerialSimulationData.TransitionIfNeeded( mBitLow ); // we want to add a zero bit
        else
            mSerialSimulationData.TransitionIfNeeded( mBitHigh ); // we want to add a one bit

        mSerialSimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod() );
    }
    else if( mSettings->mParity == AnalyzerEnums::Odd )
    {
        if( AnalyzerHelpers::IsOdd( AnalyzerHelpers::GetOnesCount( value ) ) == true )
            mSerialSimulationData.TransitionIfNeeded( mBitLow ); // we want to add a zero bit
        else
            mSerialSimulationData.TransitionIfNeeded( mBitHigh );

        mSerialSimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod() );
    }

    mSerialSimulationData.TransitionIfNeeded( mBitHigh ); // we need to end high

    // let's pad the end a bit for the stop bit:
    mSerialSimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod( mSettings->mStopBits ) );
}
