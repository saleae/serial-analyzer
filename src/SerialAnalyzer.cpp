#include "SerialAnalyzer.h"
#include "SerialAnalyzerSettings.h"
#include <AnalyzerChannelData.h>


SerialAnalyzer::SerialAnalyzer() : Analyzer2(), mSettings( new SerialAnalyzerSettings() ), mSimulationInitialized( false )
{
    SetAnalyzerSettings( mSettings.get() );
    UseFrameV2();
}

SerialAnalyzer::~SerialAnalyzer()
{
    KillThread();
}

void SerialAnalyzer::ComputeSampleOffsets()
{
    ClockGenerator clock_generator;
    clock_generator.Init( mSettings->mBitRate, mSampleRateHz );

    mSampleOffsets.clear();

    U32 num_bits = mSettings->mBitsPerTransfer;

    if( mSettings->mSerialMode != SerialAnalyzerEnums::Normal )
        num_bits++;

    mSampleOffsets.push_back( clock_generator.AdvanceByHalfPeriod( 1.5 ) ); // point to the center of the 1st bit (past the start bit)
    num_bits--;                                                             // we just added the first bit.

    for( U32 i = 0; i < num_bits; i++ )
    {
        mSampleOffsets.push_back( clock_generator.AdvanceByHalfPeriod() );
    }

    if( mSettings->mParity != AnalyzerEnums::None )
        mParityBitOffset = clock_generator.AdvanceByHalfPeriod();

    // to check for framing errors, we also want to check
    // 1/2 bit after the beginning of the stop bit
    mStartOfStopBitOffset = clock_generator.AdvanceByHalfPeriod(
        1.0 ); // i.e. moving from the center of the last data bit (where we left off) to 1/2 period into the stop bit

    // and 1/2 bit before end of the stop bit period
    mEndOfStopBitOffset = clock_generator.AdvanceByHalfPeriod( mSettings->mStopBits - 1.0 ); // if stopBits == 1.0, this will be 0
}


void SerialAnalyzer::SetupResults()
{
    // Unlike the worker thread, this function is called from the GUI thread
    // we need to reset the Results object here because it is exposed for direct access by the GUI, and it can't be deleted from the
    // WorkerThread

    mResults.reset( new SerialAnalyzerResults( this, mSettings.get() ) );
    SetAnalyzerResults( mResults.get() );
    mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}

void SerialAnalyzer::WorkerThread()
{
    mSampleRateHz = GetSampleRate();
    ComputeSampleOffsets();

    U32 bits_per_transfer = mSettings->mBitsPerTransfer;
    if( mSettings->mSerialMode != SerialAnalyzerEnums::Normal )
        bits_per_transfer++;

    // used for HLA byte count, this should not include an extra bit for MP/MDB
    const U32 bytes_per_transfer = ( mSettings->mBitsPerTransfer + 7 ) / 8;

    if( mSettings->mInverted == false )
    {
        mBitHigh = BIT_HIGH;
        mBitLow = BIT_LOW;
    }
    else
    {
        mBitHigh = BIT_LOW;
        mBitLow = BIT_HIGH;
    }

    U64 bit_mask = 0;
    U64 mask = 0x1ULL;
    for( U32 i = 0; i < bits_per_transfer; i++ )
    {
        bit_mask |= mask;
        mask <<= 1;
    }

    mSerial = GetAnalyzerChannelData( mSettings->mInputChannel );
    mSerial->TrackMinimumPulseWidth();

    if( mSerial->GetBitState() == mBitLow )
        mSerial->AdvanceToNextEdge();

    for( ;; )
    {
        // we're starting high. (we'll assume that we're not in the middle of a byte.)

        mSerial->AdvanceToNextEdge();

        // we're now at the beginning of the start bit.  We can start collecting the data.
        U64 frame_starting_sample = mSerial->GetSampleNumber();

        U64 data = 0;
        bool parity_error = false;
        bool framing_error = false;
        bool mp_is_address = false;

        DataBuilder data_builder;
        data_builder.Reset( &data, mSettings->mShiftOrder, bits_per_transfer );
        U64 marker_location = frame_starting_sample;

        for( U32 i = 0; i < bits_per_transfer; i++ )
        {
            mSerial->Advance( mSampleOffsets[ i ] );
            data_builder.AddBit( mSerial->GetBitState() );

            marker_location += mSampleOffsets[ i ];
            mResults->AddMarker( marker_location, AnalyzerResults::Dot, mSettings->mInputChannel );
        }

        if( mSettings->mInverted == true )
            data = ( ~data ) & bit_mask;

        if( mSettings->mSerialMode != SerialAnalyzerEnums::Normal )
        {
            // extract the MSB
            U64 msb = data >> ( bits_per_transfer - 1 );
            msb &= 0x1;
            if( mSettings->mSerialMode == SerialAnalyzerEnums::MpModeMsbOneMeansAddress )
            {
                mp_is_address = msb == 0x1;
            }
            else if( mSettings->mSerialMode == SerialAnalyzerEnums::MpModeMsbZeroMeansAddress )
            {
                mp_is_address = msb == 0x0;
            }
            // now remove the msb.
            data &= ( bit_mask >> 1 );
        }

        parity_error = false;

        if( mSettings->mParity != AnalyzerEnums::None )
        {
            mSerial->Advance( mParityBitOffset );
            bool is_even = AnalyzerHelpers::IsEven( AnalyzerHelpers::GetOnesCount( data ) );

            if( mSettings->mParity == AnalyzerEnums::Even )
            {
                if( is_even == true )
                {
                    if( mSerial->GetBitState() != mBitLow ) // we expect a low bit, to keep the parity even.
                        parity_error = true;
                }
                else
                {
                    if( mSerial->GetBitState() != mBitHigh ) // we expect a high bit, to force parity even.
                        parity_error = true;
                }
            }
            else // if( mSettings->mParity == AnalyzerEnums::Odd )
            {
                if( is_even == false )
                {
                    if( mSerial->GetBitState() != mBitLow ) // we expect a low bit, to keep the parity odd.
                        parity_error = true;
                }
                else
                {
                    if( mSerial->GetBitState() != mBitHigh ) // we expect a high bit, to force parity odd.
                        parity_error = true;
                }
            }

            marker_location += mParityBitOffset;
            mResults->AddMarker( marker_location, AnalyzerResults::Square, mSettings->mInputChannel );
        }

        // now we must determine if there is a framing error.
        framing_error = false;

        mSerial->Advance( mStartOfStopBitOffset );

        if( mSerial->GetBitState() != mBitHigh )
        {
            framing_error = true;
        }
        else
        {
            U32 num_edges = mSerial->Advance( mEndOfStopBitOffset );
            if( num_edges != 0 )
                framing_error = true;
        }

        if( framing_error == true )
        {
            marker_location += mStartOfStopBitOffset;
            mResults->AddMarker( marker_location, AnalyzerResults::ErrorX, mSettings->mInputChannel );

            if( mEndOfStopBitOffset != 0 )
            {
                marker_location += mEndOfStopBitOffset;
                mResults->AddMarker( marker_location, AnalyzerResults::ErrorX, mSettings->mInputChannel );
            }
        }

        // ok now record the value!
        // note that we're not using the mData2 or mType fields for anything, so we won't bother to set them.
        Frame frame;
        frame.mStartingSampleInclusive = static_cast<S64>( frame_starting_sample );
        frame.mEndingSampleInclusive = static_cast<S64>( mSerial->GetSampleNumber() );
        frame.mData1 = data;
        frame.mFlags = 0;
        if( parity_error == true )
            frame.mFlags |= PARITY_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG;

        if( framing_error == true )
            frame.mFlags |= FRAMING_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG;

        if( mp_is_address == true )
            frame.mFlags |= MP_MODE_ADDRESS_FLAG;

        if( mp_is_address == true )
            mResults->CommitPacketAndStartNewPacket();

        mResults->AddFrame( frame );

        FrameV2 frameV2;

        U8 bytes[ 8 ];
        for( U32 i = 0; i < bytes_per_transfer; ++i )
        {
            auto bit_offset = ( bytes_per_transfer - i - 1 ) * 8;
            bytes[ i ] = static_cast<U8>( data >> bit_offset );
        }
        frameV2.AddByteArray( "data", bytes, bytes_per_transfer );

        if( parity_error )
        {
            frameV2.AddString( "error", "parity" );
        }
        else if( framing_error )
        {
            frameV2.AddString( "error", "framing" );
        }

        if( mSettings->mSerialMode != SerialAnalyzerEnums::Normal )
        {
            frameV2.AddBoolean( "address", mp_is_address );
        }

        mResults->AddFrameV2( frameV2, "data", frame_starting_sample, mSerial->GetSampleNumber() );

        mResults->CommitResults();

        ReportProgress( frame.mEndingSampleInclusive );
        CheckIfThreadShouldExit();

        if( framing_error == true ) // if we're still low, let's fix that for the next round.
        {
            if( mSerial->GetBitState() == mBitLow )
                mSerial->AdvanceToNextEdge();
        }
    }
}

bool SerialAnalyzer::NeedsRerun()
{
    if( mSettings->mUseAutobaud == false )
        return false;

    // ok, lets see if we should change the bit rate, base on mShortestActivePulse

    U64 shortest_pulse = mSerial->GetMinimumPulseWidthSoFar();

    if( shortest_pulse == 0 )
        AnalyzerHelpers::Assert( "Alg problem, shortest_pulse was 0" );

    U32 computed_bit_rate = U32( double( mSampleRateHz ) / double( shortest_pulse ) );

    if( computed_bit_rate > mSampleRateHz )
        AnalyzerHelpers::Assert( "Alg problem, computed_bit_rate is higher than sample rate" ); // just checking the obvious...

    if( computed_bit_rate > ( mSampleRateHz / 4 ) )
        return false; // the baud rate is too fast.
    if( computed_bit_rate == 0 )
    {
        // bad result, this is not good data, don't bother to re-run.
        return false;
    }

    U32 specified_bit_rate = mSettings->mBitRate;

    double error = double( AnalyzerHelpers::Diff32( computed_bit_rate, specified_bit_rate ) ) / double( specified_bit_rate );

    if( error > 0.1 )
    {
        mSettings->mBitRate = computed_bit_rate;
        mSettings->UpdateInterfacesFromSettings();
        return true;
    }
    else
    {
        return false;
    }
}

U32 SerialAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate,
                                            SimulationChannelDescriptor** simulation_channels )
{
    if( mSimulationInitialized == false )
    {
        mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
        mSimulationInitialized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 SerialAnalyzer::GetMinimumSampleRateHz()
{
    return mSettings->mBitRate * 4;
}

const char* SerialAnalyzer::GetAnalyzerName() const
{
    return "Async Serial";
}

const char* GetAnalyzerName()
{
    return "Async Serial";
}

Analyzer* CreateAnalyzer()
{
    return new SerialAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
    delete analyzer;
}
