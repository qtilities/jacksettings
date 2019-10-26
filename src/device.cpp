/*
	Copyright (C) 2019 Andrea Zanellato <redtid3@gmail.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	For a full copy of the GNU General Public License see the LICENSE file
*/
#include "device.h"

#include <alsa/asoundlib.h>

#include <QFile>
#include <QTextStream>

namespace jack
{
Device::Device(int cardIndex, int index, const QString &cardId,
									const QString &cardName, const QString &cardPcmId)
:
cardIndex_(cardIndex),
index_(index),
cardId_(cardId),
cardName_(cardName),
cardPcmId_(cardPcmId)
{
	setup();
}
void Device::setup()
{
// It's not clear how this thing works in ALSA: is this codec#0 file present
// even if no resource is using sound? OTOH if Jack is running, we can't query
// via ALSA API hw_params, so try with both ways:

	const char *deviceName = "hw";
	snd_pcm_t *pcm;
	snd_pcm_hw_params_t *hwParams;
	int errNumber;
	uint min, max;

	errNumber = snd_pcm_open(&pcm, deviceName, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if (errNumber < 0) {
		fprintf(stderr, "Cannot open device '%s': %s\n", deviceName,
			snd_strerror(errNumber));
		goto checkProc;
	}
	snd_pcm_hw_params_alloca(&hwParams);
	errNumber = snd_pcm_hw_params_any(pcm, hwParams);
	if (errNumber < 0) {
		fprintf(stderr, "Cannot get hardware parameters: %s\n",
			snd_strerror(errNumber));
		snd_pcm_close(pcm);
		goto checkProc;
	}
	errNumber = snd_pcm_hw_params_get_channels_min(hwParams, &minChan);
	if (errNumber < 0) {
		fprintf(stderr, "Cannot get minimum channels count: %s\n",
			snd_strerror(errNumber));
		snd_pcm_close(pcm);
		goto checkProc;
	}
	errNumber = snd_pcm_hw_params_get_channels_max(hwParams, &maxChan);
	if (errNumber < 0) {
		fprintf(stderr, "Cannot get maximum channels count: %s\n",
			snd_strerror(errNumber));
		snd_pcm_close(pcm);
		goto checkProc;
	}
	errNumber = snd_pcm_hw_params_get_rate_min(hwParams, &min, NULL);
	if (errNumber < 0) {
		fprintf(stderr, "Cannot get minimum rate: %s\n", snd_strerror(errNumber));
		snd_pcm_close(pcm);
		goto checkProc;
	}
	errNumber = snd_pcm_hw_params_get_rate_max(hwParams, &max, NULL);
	if (errNumber < 0) {
		fprintf(stderr, "Cannot get maximum rate: %s\n", snd_strerror(errNumber));
		snd_pcm_close(pcm);
		goto checkProc;
	}
	for (auto sampleRate: SampleRates)
	{
		if (!snd_pcm_hw_params_test_rate(pcm, hwParams, sampleRate, 0))
			supportedSampleRates_.push_back(sampleRate);
	}
	ulong size;
	errNumber = snd_pcm_hw_params_get_buffer_size_min(hwParams, &size);
	if (errNumber < 0) {
		fprintf(stderr, "Cannot get maximum rate: %s\n", snd_strerror(errNumber));
		snd_pcm_close(pcm);
		goto checkProc;
	}
	return;

checkProc:
	QString filePath = QString("/proc/asound/%1/codec#0").arg(cardId_);
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream in(&file);
	QString line = in.readLine().trimmed();
	while (!line.isNull())
	{
		if (line.startsWith("rates"))
		{
			QStringList lstSampleRates(line.split(':'));
			if (lstSampleRates.size() < 2)
				continue;

			lstSampleRates = lstSampleRates.at(1).trimmed().split(' ');
			foreach (QString strRate, lstSampleRates)
				supportedSampleRates_.push_back(strRate.toUInt());

			break;
		}
		line = in.readLine().trimmed();
	}
}
} // namespace jack
