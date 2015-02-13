/*
 *  AudioDriver.h
 *
 *  Copyright (c) 2001-2015 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _AUDIO_DRIVER_H
#define _AUDIO_DRIVER_H

// useful for avoiding compiler warnings
#define UNUSED_PARAM( x ) (void)x

class AudioDriver
{
public:

	enum { kMaxWriteFrames = 512 };

    virtual ~AudioDriver () { close(); }

    virtual int  open(class Config &) { return -1; }
    virtual void close() {}
    virtual int  write(float *buffer, int frames) { return -1; }
    virtual int  setRealtime() { return -1; }
};

#endif
