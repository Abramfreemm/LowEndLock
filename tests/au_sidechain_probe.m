// Throwaway diagnostic: verify that LowEndLock's AU reports a writable input
// bus count and accepts Logic's sidechain bus addition (ElementCount 1 -> 2).
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>

int main()
{
    AudioComponentDescription desc = {};
    desc.componentType    = kAudioUnitType_Effect;
    desc.componentSubType = 'Vtqg';
    desc.componentManufacturer = 'Manu';

    AudioComponent comp = AudioComponentFindNext (NULL, &desc);
    if (comp == NULL) { printf ("COMPONENT NOT FOUND\n"); return 1; }

    AudioUnit au = NULL;
    OSStatus err = AudioComponentInstanceNew (comp, &au);
    if (err != noErr) { printf ("INSTANTIATE FAIL %d\n", (int) err); return 1; }

    // 1) Is kAudioUnitProperty_ElementCount writable on the input scope?
    UInt32 dataSize = 0;
    Boolean writable = false;
    err = AudioUnitGetPropertyInfo (au, kAudioUnitProperty_ElementCount,
                                    kAudioUnitScope_Input, 0, &dataSize, &writable);
    printf ("ElementCount writable=%s (getinfo err=%d)\n", writable ? "YES" : "NO", (int) err);

    // 2) Current input element count.
    UInt32 inCount = 0;
    dataSize = sizeof (inCount);
    err = AudioUnitGetProperty (au, kAudioUnitProperty_ElementCount,
                                kAudioUnitScope_Input, 0, &inCount, &dataSize);
    printf ("Current input element count=%u (err=%d)\n", (unsigned) inCount, (int) err);

    // 3) Try Logic's sidechain dance: add a second input bus.
    UInt32 two = 2;
    err = AudioUnitSetProperty (au, kAudioUnitProperty_ElementCount,
                                kAudioUnitScope_Input, 0, &two, sizeof (two));
    printf ("SetBusCount(2) err=%d\n", (int) err);

    // 4) Set stereo stream formats on both input elements.
    AudioStreamBasicDescription fmt = {};
    fmt.mSampleRate = 44100.0;
    fmt.mFormatID = kAudioFormatLinearPCM;
    fmt.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
    fmt.mBitsPerChannel = 32;
    fmt.mChannelsPerFrame = 2;
    fmt.mBytesPerFrame = 8;
    fmt.mFramesPerPacket = 1;
    fmt.mBytesPerPacket = 8;

    err = AudioUnitSetProperty (au, kAudioUnitProperty_StreamFormat,
                                kAudioUnitScope_Input, 0, &fmt, sizeof (fmt));
    printf ("SetFormat bus0 err=%d\n", (int) err);
    err = AudioUnitSetProperty (au, kAudioUnitProperty_StreamFormat,
                                kAudioUnitScope_Input, 1, &fmt, sizeof (fmt));
    printf ("SetFormat bus1 err=%d\n", (int) err);

    // 5) Initialize.
    err = AudioUnitInitialize (au);
    printf ("AudioUnitInitialize err=%d\n", (int) err);

    AudioUnitUninitialize (au);
    AudioComponentInstanceDispose (au);
    return 0;
}
