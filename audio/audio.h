#ifndef __AUDIO_H__
#define __AUDIO_H__

#define SAMPLE_RATE 44100

typedef struct
{
    uint16_t *data;
    uint32_t pos, len;
    vec3 xyz;
} Sample_t;

bool Audio_LoadStatic(char *Filename, Sample_t *Sample);
void Audio_SetListenerOrigin(vec3 pos, vec3 right);
void Audio_PlaySample(Sample_t *Sample, bool looping);
int Audio_Init(void);
void Audio_Destroy(void);

#endif
