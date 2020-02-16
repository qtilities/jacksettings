/*
    Copyright (C) 2019 - 2020 Andrea Zanellato <redtid3@gmail.com>

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
#pragma once

#include <QString>
#include <QVector>

#include <array>

namespace jack
{
class Device
{
public:
    explicit Device(int cardIndex, int index, const QString &cardId,
                    const QString &cardName, const QString &cardPcmId);

    int     cardIndex() const {return cardIndex_;}
    int     index()     const {return index_;}
    QString cardId()    const {return cardId_;}
    QString cardName()  const {return cardName_;}
    QString cardPcmId() const {return cardPcmId_;}

    QVector<uint> supportedSampleRates() const {return supportedSampleRates_;}

std::array<int, 12> SampleRates = {
    22050,
    32000,
    44100,
    48000,
    64000,
    88200,
    96000,
    176400,
    192000
};

private:
    void setup();

    uint    minChan, maxChan;
    int     cardIndex_,index_;
    QString cardId_,cardName_,cardPcmId_;

    QVector<uint> supportedSampleRates_;
};
} // namespace jack
