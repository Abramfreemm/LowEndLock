// Diagnostic: verify that LowEndLock's AU exposes a static stereo sidechain
// input bus (Input + Sidechain) and initializes cleanly.
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>

int main()
{
    AudioComponentDescription desc = {};
    desc.componentType    = kAudioUnitType_Effect;
    desc.componentSubType = 'Vtqi';
    desc.componentManufacturer = 'Manu';

    AudioComponent comp = AudioComponentFindNext (NULL, &desc);
    if (comp == NULL) { printf ("COMPONENT NOT FOUND\n"); return 1; }

    AudioUnit au = NULL;
    OSStatus err = AudioComponentInstanceNew (comp, &au);
    if (err != noErr) { printf ("INSTANTIATE FAIL %d\n", (int) err); return 1; }

    // 1) Current input element count should be 2 (main + sidechain).
    UInt32 dataSize = 0;
    UInt32 inCount = 0;
    dataSize = sizeof (inCount);
    err = AudioUnitGetProperty (au, kAudioUnitProperty_ElementCount,
                                kAudioUnitScope_Input, 0, &inCount, &dataSize);
    printf ("Input element count=%u (expect 2) (err=%d)\n", (unsigned) inCount, (int) err);

    // 2) Set stereo stream formats on both input elements.
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
    printf ("SetFormat bus1 (sidechain) err=%d\n", (int) err);

    // 3) Initialize.
    err = AudioUnitInitialize (au);
    printf ("AudioUnitInitialize err=%d\n", (int) err);

    AudioUnitUninitialize (au);
    AudioComponentInstanceDispose (au);
    return 0;
}
