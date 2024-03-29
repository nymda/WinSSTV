﻿/*
 * This file is part of WinSSTV (https://github.com/nymda/WinSSTV).
 * Copyright (c) 2022 github/nymda
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys.h>
#include "wav.h"
#include "SSTV.h"

namespace wav {
    int ampl = 30000;
    
    const float pi = 3.1415926535;
	double angle = 0.0;

	wavDistortions* distortions = new wavDistortions();
	wavHeader header = {};
    void* wavheap = 0;

    double expectedDurationMS = 0;
    double actualDurationMS = 0;
    int balance_AddedSamples = 0;
    int balance_SkippedSamples = 0;
    int bytesWritten = 0;
    int writeIndex = 0;

    int init(int sampleRate) {
        if (wavheap) { free(wavheap); }
        
        expectedDurationMS = 0;
        actualDurationMS = 0;
        balance_AddedSamples = 0;
        balance_SkippedSamples = 0;
        bytesWritten = 0;
        writeIndex = 0;

		header.sampleRate = sampleRate;
        header.SBC = (header.sampleRate * header.BPS * header.channels) / 8;
        header.dataSize = (static_cast<unsigned long long>(header.sampleRate) * 15) * sizeof(short);
        header.fileSize = header.dataSize + sizeof(wavHeader);

        wavheap = malloc(header.fileSize);
        if (!wavheap) {
            return 0;
        }
        return 1;
    }

	//add a tone to the wav file, duration in MS
    void addTone(short frequency, float duration, generatorType gt) {
        //timing distortion
		int sampleRateDistorted = header.sampleRate + distortions->timingOffset;
        
        //number of samples required for the requested duration. sometimes.
        int sampleCount = round((sampleRateDistorted) * (duration / 1000.f));

        //balancing
        expectedDurationMS += duration;
        actualDurationMS += (sampleCount / static_cast<double>(sampleRateDistorted)) * 1000;
        float msPerSample = 1000.f / sampleRateDistorted;

        //if you're gonna run out of space in the wav then add more in 15 second chunks.
        while (bytesWritten + (sampleCount * sizeof(short)) > header.dataSize) {
            header.dataSize += (sampleRateDistorted * 15) * sizeof(short);
            header.fileSize = header.dataSize + sizeof(wavHeader);
            void* reallocated = realloc(wavheap, header.fileSize);
            if (reallocated) { wavheap = reallocated; }
            else {
                printf_s("Issue while expanding WAV memory!\n");
                return;
            }
        }

        //actually calculate and add the data
        monoSample* wavData = (monoSample*)((uintptr_t)wavheap + sizeof(header));
        for (int i = 0; i < sampleCount; i++) {
            bytesWritten += (int)sizeof(short);

            //noise disortion
            int noiseFactor = (int)(distortions->noiseLvl * ampl);
            int noisePointAddition = 0;
            if (noiseFactor > 0) { noisePointAddition = (rand() % noiseFactor) - (noiseFactor / 2); }
            
            //calculates the actual waveform
            switch (gt) {
                case GT_SINE:
                    wavData[writeIndex].S = (short)(ampl * sin(angle)) + noisePointAddition;
                    break;
                case GT_SQUARE:
                    wavData[writeIndex].S = (short)(ampl * (sin(angle) > 0 ? 1 : -1)) + noisePointAddition;
                    break;
                case GT_TRIANGLE:
                    wavData[writeIndex].S = (short)(ampl * (2 / pi) * asin(sin(angle))) + noisePointAddition;
                    break;
            }
            angle += ((2 * pi * frequency) / sampleRateDistorted);
            writeIndex++;

            //balances issues with timing. see note at the bottom.
            float diff = actualDurationMS - expectedDurationMS;
            if (diff > msPerSample) {
                sampleCount--;
                actualDurationMS -= msPerSample;
                balance_SkippedSamples++;
            }
            if (diff < -msPerSample) {
                sampleCount++;
                actualDurationMS += msPerSample;
                balance_AddedSamples++;
            }

            while (angle > 2 * pi) { angle -= 2 * pi; } //avoid floating point weirdness
        }
    }

    wasapiDevicePkg* WASAPIGetDevices() {

        wasapiDevicePkg* pkg = (wasapiDevicePkg*)malloc(sizeof(wasapiDevicePkg));
        if (!pkg) { return 0; }

        IMMDevice* defaultDevice = NULL;

		IMMDeviceEnumerator* pEnumerator = NULL;
		IMMDeviceCollection* pCollection = NULL;
		UINT count;
        HRESULT hr = CoInitializeEx(nullptr, COINIT_SPEED_OVER_MEMORY); //coinit again
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to CoInitialize!\n");
            return 0 ;
        }
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator); //create instance
		if (FAILED(hr)) {
			printf_s("[ERR] Failed to create device enumerator!\n");
			return 0 ;
		}
		hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection); //get all audio endpoints
		if (FAILED(hr)) {
			printf_s("[ERR] Failed to enumerate audio endpoints!\n");
			return 0 ;
		}
		hr = pCollection->GetCount(&count); //get the number of audio endpoints
		if (FAILED(hr)) {
			printf_s("[ERR] Failed to get device count!\n");
			return 0;
		}

        hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to get default device!\n");
            return 0;
        }

        pkg->deviceCount = count;
        pkg->devices = (wasapiDevice*)calloc(count, sizeof(wasapiDevice));
        if (!pkg->devices) { return 0; }

		for (UINT i = 0; i < count; i++) {
			IMMDevice* pDevice = NULL;
			IPropertyStore* pProps = NULL;
			PROPVARIANT varName;
			PropVariantInit(&varName);
			hr = pCollection->Item(i, &pDevice); //get the audio endpoint at index i
			if (FAILED(hr)) {
				printf_s("[ERR] Failed to get device %d!\n", i);
				return 0;
			}
			hr = pDevice->OpenPropertyStore(STGM_READ, &pProps); //get its properties
			if (FAILED(hr)) {
				printf_s("[ERR] Failed to open property store for device %d!\n", i);
				return 0;
			}
			hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName); //get its name
			if (FAILED(hr)) {
				printf_s("[ERR] Failed to get friendly name for device %d!\n", i);
				return 0;
			}

            //print its index and name
            pkg->devices[i].ID = i;
            swprintf_s(pkg->devices[i].name, 128, varName.pwszVal);

			PropVariantClear(&varName);
			pProps->Release();
			pDevice->Release();
		}
		pCollection->Release();
		pEnumerator->Release();

        return pkg;
    }
    
    wchar_t* WASAPIGetDeviceIdByIndex(int index) {
        IMMDeviceEnumerator* pEnumerator = NULL;
        IMMDeviceCollection* pCollection = NULL;
        UINT count;
        HRESULT hr = CoInitializeEx(nullptr, COINIT_SPEED_OVER_MEMORY); //coinit again
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to CoInitialize!\n");
            return 0;
        }
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator); //create instance
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to create device enumerator!\n");
            return 0;
        }
        hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection); //get all audio endpoints
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to enumerate audio endpoints!\n");
            return 0;
        }
        hr = pCollection->GetCount(&count); //get number of audio endpoints
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to get device count!\n");
            return 0;
        }

        if (index < 0 || index > count) { //make sure the user requested a valid audio device
			printf_s("[ERR] Invalid device index, see -D\n");
			return 0;
        }
        
        //buffers for the devices ID
        wchar_t* deviceID = (wchar_t*)malloc(128);
        if (!deviceID) { return 0; }
        
        IMMDevice* pDevice = NULL;
        IPropertyStore* pProps = NULL;
        PROPVARIANT varName;
        PropVariantInit(&varName);
        hr = pCollection->Item(index, &pDevice); //get the actual device
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to get device!\n");
            return 0;
        }
        hr = pDevice->GetId((LPWSTR*)&deviceID); //get its id
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to get ID of device!\n");
            return 0;
        }

        PropVariantClear(&varName);
        pDevice->Release();

        pCollection->Release();
        pEnumerator->Release();

        return deviceID;
    }

    void ShowConsoleCursor(bool showFlag)
    {
        HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(out, &cursorInfo);
        cursorInfo.bVisible = showFlag;
        SetConsoleCursorInfo(out, &cursorInfo);
    }
    
    void generateProgressBar(int progress, char* buffer, int bufferWidth) {
		int barWidth = bufferWidth - 2;
		int barProgress = (barWidth * progress) / 100;
		int barRemainder = barWidth - barProgress;

        for (int i = 0; i < barWidth; i++) {
            if (i < barProgress) {
                buffer[i] = '=';
            }
            else if (i == barProgress) {
                buffer[i] = '>';
            }
            else {
                buffer[i] = '-';
            }
        }
    }

    //i have no fucking idea what this does, i copied it from an example and just monkey-typewriter'd it until it worked
    char progressBarTxt[50] = {};
    void beginPlayback(int iDeviceID, playbackReporter* reporter) {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_SPEED_OVER_MEMORY);
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to CoInitialize!\n");
            return;
        }
        
        IMMDeviceEnumerator* deviceEnumerator;
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)(&deviceEnumerator));
        IMMDevice* audioDevice;
        hr = deviceEnumerator->GetDevice(WASAPIGetDeviceIdByIndex(iDeviceID), &audioDevice);
        if (FAILED(hr)) {
            return; //probably a failure in WASAPIGetDeviceIdByIndex, user was warned above, just quit
        }
        
        deviceEnumerator->Release();

        IAudioClient2* audioClient;
        hr = audioDevice->Activate(__uuidof(IAudioClient2), CLSCTX_ALL, nullptr, (LPVOID*)(&audioClient));

        audioDevice->Release();
        
        //pretty much the same as the wav header class
        WAVEFORMATEX mixFormat = {};
        mixFormat.wFormatTag = WAVE_FORMAT_PCM;
        mixFormat.nChannels = header.channels;
        mixFormat.nSamplesPerSec = header.sampleRate;
        mixFormat.wBitsPerSample = header.BPS;
        mixFormat.nBlockAlign = (mixFormat.nChannels * mixFormat.wBitsPerSample) / 8;
        mixFormat.nAvgBytesPerSec = mixFormat.nSamplesPerSec * mixFormat.nBlockAlign;

        //initialize the audio client
        DWORD initStreamFlags = (AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY);
        hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, initStreamFlags, (REFERENCE_TIME)10000000, 0, &mixFormat, nullptr);

        //initialize the renderer
        IAudioRenderClient* audioRenderClient;
        hr = audioClient->GetService(__uuidof(IAudioRenderClient), (LPVOID*)(&audioRenderClient));

        UINT32 bufferSizeInFrames;
        hr = audioClient->GetBufferSize(&bufferSizeInFrames);
        hr = audioClient->Start();

        int wavPlaybackSample = 0;
        int lastPrintedPercentage = 0;
        bool finished = false;

        reporter->running = true;

        while (!finished)
        {
            //i dont know what the padding does
            UINT32 bufferPadding;
            hr = audioClient->GetCurrentPadding(&bufferPadding);

            //some form of maths ¯\_(._.)_/¯
            int soundBufferLatency = bufferSizeInFrames / 50;
            int numFramesToWrite = soundBufferLatency - bufferPadding;
            
            //gets the buffer from the audio client that you can proceed to squirt audio data into
            short* buffer;
            hr = audioRenderClient->GetBuffer(numFramesToWrite, (BYTE**)(&buffer));
            
            //data squirtery loop
            for (int frameIndex = 0; frameIndex < numFramesToWrite; ++frameIndex)
            {                    
				//squirt the audio data from the wav file into the playback buffer
                *buffer++ = (short)((float)((short*)wavheap)[wavPlaybackSample] * reporter->volume);
                
                float playbackMS = ((float)wavPlaybackSample / (float)header.sampleRate) * 1000.f;

				if ((int)playbackMS % 100 == 0) { //report progress every 100ms
                    if (reporter->abort) {
                        reporter->running = false;
                        return;
                    }
                    
                    int percentage = ceil((float)wavPlaybackSample / (float)writeIndex * 1000.f);

					reporter->currentMin = ((int)playbackMS / 1000) / 60;
					reporter->currentSec = ((int)playbackMS / 1000) % 60;
                    reporter->currentMs =  ((int)playbackMS % 1000);
                    reporter->totalMin =   ((int)actualDurationMS / 1000) / 60;
                    reporter->totalSec =   ((int)actualDurationMS / 1000) % 60;
                    reporter->totalMs =    ((int)actualDurationMS % 1000);

					reporter->playedPercent = percentage;
				}
              
                //next sample
                ++wavPlaybackSample;
            }

            //if its done then quit out of the loop
            if (wavPlaybackSample >= writeIndex) {
                finished = true;
                break;
            }
            
            //free up the buffer because memory
            audioRenderClient->ReleaseBuffer(numFramesToWrite, 0);
        }

        //set played percentage to full
        reporter->playedPercent = 1000;

        Sleep(500); //wait for the gui to update to 1000 to totally fill the bar

        reporter->running = false;
        audioClient->Stop();
        audioClient->Release();
        audioRenderClient->Release();
    }
    
    int save(FILE* fptr) {
        header.dataSize -= header.dataSize - bytesWritten;
        header.fileSize = header.dataSize + sizeof(wavHeader);
        memcpy(wavheap, &header, sizeof(header));
        return fwrite(wavheap, header.fileSize, 1, fptr);
    }
}

//todo: explain balancing again