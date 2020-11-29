/****************************************************************************
 *                  ray.h
 *
 * This module contains all defines, typedefs, and prototypes for RAY.CPP.
 *
 * from Persistence of Vision(tm) Ray Tracer version 3.6.
 * Copyright 1991-2003 Persistence of Vision Team
 * Copyright 2003-2004 Persistence of Vision Raytracer Pty. Ltd.
 *---------------------------------------------------------------------------
 * NOTICE: This source code file is provided so that users may experiment
 * with enhancements to POV-Ray and to port the software to platforms other
 * than those supported by the POV-Ray developers. There are strict rules
 * regarding how you are permitted to use this file. These rules are contained
 * in the distribution and derivative versions licenses which should have been
 * provided with this file.
 *
 * These licences may be found online, linked from the end-user license
 * agreement that is located at http://www.povray.org/povlegal.html
 *---------------------------------------------------------------------------
 * This program is based on the popular DKB raytracer version 2.12.
 * DKBTrace was originally written by David K. Buck.
 * DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
 *---------------------------------------------------------------------------
 * $File: //depot/povray/3.5/source/ray.h $
 * $Revision: #13 $
 * $Change: 2929 $
 * $DateTime: 2004/07/01 14:01:40 $
 * $Author: calimet $
 * $Log$
 *****************************************************************************/

/* NOTE: FRAME.H contains other ray stuff. */

#ifndef RAY_H
#define RAY_H

BEGIN_POV_NAMESPACE

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/



/*****************************************************************************
* Global typedefs
******************************************************************************/



/*****************************************************************************
* Global variables
******************************************************************************/




/*****************************************************************************
* Global functions
******************************************************************************/

void Initialize_Ray_Containers(RAY *Ray, unsigned int optimisiation_flags = OPTIMISE_NONE);
void Copy_Ray_Containers (RAY *Dest_Ray, RAY *Source_Ray);
void Ray_Enter (RAY *ray, INTERIOR *interior);
void Ray_Exit (RAY *ray, int nr);
int  Interior_In_Ray_Container (RAY *, INTERIOR *);

END_POV_NAMESPACE

#endif
