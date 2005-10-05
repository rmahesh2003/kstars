#ifndef LX200_16_H
#define LX200_16_H

/*
    LX200 16"
    Copyright (C) 2003 Jasem Mutlaq (mutlaqja@ikarustech.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "lx200autostar.h"

class LX200_16 : public LX200Autostar
{
 public:
  LX200_16();
  ~LX200_16() {}

 void ISGetProperties (const char *dev);
 void ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n);
 void ISNewText (const char *dev, const char *name, char *texts[], char *names[], int n);
 void ISNewSwitch (const char *dev, const char *name, ISState *states, char *names[], int n);
 void ISPoll ();
 void getBasicData();
 void handleAltAzSlew();

 private:

 double currentAlt;
 double currentAz;
 double targetAlt;
 double targetAz;

};

void changeLX200_16DeviceName(const char * newName);

#endif


