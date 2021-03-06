#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../system/system.h"
#include "../math/math.h"
#include "audio.h"

#define RIFF_MAGIC ('R'|('I'<<8)|('F'<<16)|('F'<<24))
#define WAVE_MAGIC ('W'|('A'<<8)|('V'<<16)|('E'<<24))
#define FMT_MAGIC ('f'|('m'<<8)|('t'<<16)|(' '<<24))
#define DATA_MAGIC ('d'|('a'<<8)|('t'<<16)|('a'<<24))

// Simple resampling function, based off of what id Software used in Quake, seems to work well enough.
void Resample(void *in, int inwidth, int inrate, int inlength, int16_t *out, int outrate)
{
    float stepscale=(float)inrate/SAMPLE_RATE;

    uint32_t outcount=(uint32_t)(inlength/stepscale);

    for(uint32_t i=0, samplefrac=0;i<outcount; i++, samplefrac+=(int32_t)(stepscale*256))
    {
        int32_t sample;
        int32_t srcsample=samplefrac>>8;

        if(inwidth==2)
            sample=((int16_t *)in)[srcsample];
        else
            sample=(int16_t)(((int8_t *)in)[srcsample]-128)<<8;

        out[i]=sample;
    }
}

// Load a WAVE sound file, this *probably* should search for chunks, not blindly load... Whatever.
// This also will only accept PCM audio streams and mono, ideally stereo isn't needed
// because most sound effcts will be panned/spatialized into stereo for "3D" audio.
bool Audio_LoadStatic(char *Filename, Sample_t *Sample)
{
    FILE *stream=NULL;
    uint32_t riff_magic=0, wave_magic=0, fmt_magic=0, data_magic=0;
    uint16_t format=0;
    uint16_t channels=0;
    uint32_t frequency=0;
    uint16_t sample=0;
    uint32_t length=0;
    uint32_t fileSize=0, RIFFsize=0;

    if((stream=fopen(Filename, "rb"))==NULL)
        return 0;

    // This should match the RIFF chuck size
    fseek(stream, 0, SEEK_END);
    fileSize=ftell(stream)-(sizeof(uint32_t)*2);
    fseek(stream, 0, SEEK_SET);

    // Header
    fread(&riff_magic, sizeof(uint32_t), 1, stream);    // RIFF magic marker ("RIFF")
    fread(&RIFFsize, sizeof(uint32_t), 1, stream);      // RIFF chuck size

    if(riff_magic!=RIFF_MAGIC&&fileSize!=RIFFsize)
    {
        fclose(stream);
        return 0;
    }

    fread(&wave_magic, sizeof(uint32_t), 1, stream);    // WAVE magic marker ("WAVE")

    if(wave_magic!=WAVE_MAGIC)
    {
        fclose(stream);
        return 0;
    }

    // Wave format block
    fread(&fmt_magic, sizeof(uint32_t), 1, stream);     // Wave format header magic marker ("fmt ")
    if(fmt_magic!=FMT_MAGIC)
    {
        fclose(stream);
        return 0;
    }

    fseek(stream, sizeof(uint32_t), SEEK_CUR);          // Format header size?

    fread(&format, sizeof(uint16_t), 1, stream);        // wFormatTag
    fread(&channels, sizeof(uint16_t), 1, stream);      // nChannels
    fread(&frequency, sizeof(uint32_t), 1, stream);     // nSamplesPerSec
    fseek(stream, sizeof(uint32_t), SEEK_CUR);          // nAvgBytesPerSec
    fseek(stream, sizeof(uint16_t), SEEK_CUR);          // nBlockAlign
    fread(&sample, sizeof(uint16_t), 1, stream);        // wBitsPerSample

    if(format!=1&&channels!=1)
    {
        // Only support PCM streams and mono
        fclose(stream);
        return 0;
    }

    fread(&data_magic, sizeof(uint32_t), 1, stream);    // Data block magic marker ("data")

    if(data_magic!=DATA_MAGIC)
    {
        fclose(stream);
        return 0;
    }

    fread(&length, sizeof(uint32_t), 1, stream);        // Length of data block

    // Allocate memory for data
    uint8_t *buffer=(uint8_t *)malloc(length);

    if(buffer==NULL)
    {
        fclose(stream);
        return 0;
    }

    // Read in the audio data
    fread(buffer, 1, length, stream);

    fclose(stream);

    // Divide by sample size, so that legnth is the number of actual samples, not the data length.
    length/=sample>>3;

    // Convert to match primary buffer sampling rate.
    uint32_t outputSize=(uint32_t)(length/((float)frequency/SAMPLE_RATE));

    // Allocate memory for new resampled audio buffer.
    int16_t *resampled=(int16_t *)malloc(sizeof(int16_t)*outputSize);

    if(resampled==NULL)
    {
        FREE(buffer);
        return 0;
    }

    // Resample the audio buffer
    Resample(buffer, sample>>3, frequency, length, resampled, SAMPLE_RATE);

    // Free the original buffer
    FREE(buffer);

    // Assign pointers and other data
    Sample->data=resampled;
    Sample->len=outputSize;
    Sample->pos=0;
    Vec3_Sets(Sample->xyz, 0.0f);

    return 1;
}
