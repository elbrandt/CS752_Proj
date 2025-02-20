/****************************************************************************
 *                  super.cpp
 *
 * This module implements functions that manipulate superellipsoids.
 *
 * Original code written by Alexander Enzmann.
 * Adaption to POV-Ray by Dieter Bayer [DB].
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
 * $File: //depot/povray/3.5/source/super.cpp $
 * $Revision: #21 $
 * $Change: 3010 $
 * $DateTime: 2004/07/27 14:06:43 $
 * $Author: thorsten $
 * $Log$
 *****************************************************************************/

/****************************************************************************
*
*  Explanation:
*
*    Superellipsoids are defined by the implicit equation
*
*      f(x,y,z) = (|x|^(2/e) + |y|^(2/e))^(e/n) + |z|^(2/n) - 1
*
*    Where e is the east/west exponent and n is the north/south exponent.
*
*    NOTE: The exponents are precomputed and stored in the Power element.
*
*    NOTE: Values of e and n that are close to degenerate (e.g.,
*          below around 0.1) appear to give the root solver fits.
*          Not sure quite where the problem lays just yet.
*
*  Syntax:
*
*    superellipsoid { <e, n> }
*
*
*  ---
*
*  Oct 1994 : Creation.
*
*****************************************************************************/

#include "frame.h"
#include "povray.h"
#include "vector.h"
#include "bbox.h"
#include "matrices.h"
#include "objects.h"
#include "super.h"
#include "spec_qsort.h"

BEGIN_POV_NAMESPACE

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

/* Minimal intersection depth for a valid intersection. */

const DBL DEPTH_TOLERANCE = 1.0e-4;

/* If |x| < ZERO_TOLERANCE it is regarded to be 0. */

const DBL ZERO_TOLERANCE = 1.0e-10;

/* This is not the signum function because SGNX(0) is 1. */

#define SGNX(x) (((x) >= 0.0) ? 1.0 : -1.0)

const DBL MIN_VALUE = -1.0;
const DBL MAX_VALUE  = 1.0;

const int MAX_ITERATIONS = 20;

const int PLANECOUNT = 9;



/*****************************************************************************
* Local typedefs
******************************************************************************/



/*****************************************************************************
* Static functions
******************************************************************************/

static int intersect_superellipsoid (RAY *Ray, SUPERELLIPSOID *Superellipsoid, ISTACK *Depth_Stack);
static int intersect_box (VECTOR P, VECTOR D, DBL *dmin, DBL *dmax);
static DBL power (DBL x, DBL e);
static DBL evaluate_superellipsoid (VECTOR P, SUPERELLIPSOID *Superellipsoid);
static int compdists (const void *in_a, const void *in_b);

static int find_ray_plane_points (VECTOR P, VECTOR D, int cnt, DBL *dists, DBL mindist, DBL maxdist);
static void solve_hit1 (SUPERELLIPSOID *Super, DBL v0, VECTOR tP0, DBL v1, VECTOR tP1, VECTOR P);
static int check_hit2 (SUPERELLIPSOID *Super, VECTOR P, VECTOR D, DBL t0, VECTOR P0, DBL v0, DBL t1, DBL *t, VECTOR Q);
static int insert_hit (OBJECT *Object, RAY *Ray, DBL Depth, ISTACK *Depth_Stack);

static int   All_Superellipsoid_Intersections (OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack);
static int   Inside_Superellipsoid (VECTOR point, OBJECT *Object);
static void  Superellipsoid_Normal (VECTOR Result, OBJECT *Object, INTERSECTION *Inter);
static SUPERELLIPSOID *Copy_Superellipsoid (OBJECT *Object);
static void  Translate_Superellipsoid (OBJECT *Object, VECTOR Vector, TRANSFORM *Trans);
static void  Rotate_Superellipsoid (OBJECT *Object, VECTOR Vector, TRANSFORM *Trans);
static void  Scale_Superellipsoid (OBJECT *Object, VECTOR Vector, TRANSFORM *Trans);
static void  Transform_Superellipsoid (OBJECT *Object, TRANSFORM *Trans);
static void  Invert_Superellipsoid (OBJECT *Object);
static void  Destroy_Superellipsoid (OBJECT *Object);

/*****************************************************************************
* Local variables
******************************************************************************/

static METHODS Superellipsoid_Methods =
{
  All_Superellipsoid_Intersections,
  Inside_Superellipsoid, Superellipsoid_Normal, Default_UVCoord,
  (COPY_METHOD)Copy_Superellipsoid,
  Translate_Superellipsoid, Rotate_Superellipsoid,
  Scale_Superellipsoid, Transform_Superellipsoid, Invert_Superellipsoid, Destroy_Superellipsoid
};

const DBL planes[PLANECOUNT][4] =
  {{1, 1, 0, 0}, {1,-1, 0, 0},
   {1, 0, 1, 0}, {1, 0,-1, 0},
   {0, 1, 1, 0}, {0, 1,-1, 0},
   {1, 0, 0, 0}, 
   {0, 1, 0, 0}, 
   {0, 0, 1, 0}};



/*****************************************************************************
*
* FUNCTION
*
*   All_Superellipsoid_Intersections
*
* INPUT
*
*   Object      - Object
*   Ray         - Ray
*   Depth_Stack - Intersection stack
*   
* OUTPUT
*
*   Depth_Stack
*   
* RETURNS
*
*   int - true, if a intersection was found
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Determine ray/superellipsoid intersection and clip intersection found.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

static int All_Superellipsoid_Intersections(OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack)
{
  Increase_Counter(stats[Ray_Superellipsoid_Tests]);

  if (intersect_superellipsoid(Ray, (SUPERELLIPSOID *)Object, Depth_Stack))
  {
    Increase_Counter(stats[Ray_Superellipsoid_Tests_Succeeded]);

    return(true);
  }
  else
  {
    return(false);
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   intersect_superellipsoid
*
* INPUT
*
*   Ray            - Ray
*   Superellipsoid - Superellipsoid
*   Depth_Stack    - Depth stack
*   
* OUTPUT
*
*   Intersection
*   
* RETURNS
*
*   int - true if intersections were found.
*   
* AUTHOR
*
*   Alexander Enzmann, Dieter Bayer
*   
* DESCRIPTION
*
*   Determine ray/superellipsoid intersection.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

static int intersect_superellipsoid(RAY *Ray, SUPERELLIPSOID *Superellipsoid, ISTACK *Depth_Stack)
{
  int i, cnt, Found = false;
  DBL dists[PLANECOUNT+2];
  DBL t, t1, t2, v0, v1, len;
  VECTOR P, D, P0, P1, P2, P3;

  /* Transform the ray into the superellipsoid space. */

  MInvTransPoint(P, Ray->Initial, Superellipsoid->Trans);

  MInvTransDirection(D, Ray->Direction, Superellipsoid->Trans);

  VLength(len, D);

  VInverseScaleEq(D, len);

  /* Intersect superellipsoid's bounding box. */

  if (!intersect_box(P, D, &t1, &t2))
  {
    return(false);
  }

  /* Test if superellipsoid lies 'behind' the ray origin. */

  if (t2 < DEPTH_TOLERANCE)
  {
    return(false);
  }

  cnt = 0;

  if (t1 < DEPTH_TOLERANCE)
  {
    t1 = DEPTH_TOLERANCE;
  }

  dists[cnt++] = t1;
  dists[cnt++] = t2;

  /* Intersect ray with planes cutting superellipsoids in pieces. */

  cnt = find_ray_plane_points(P, D, cnt, dists, t1, t2);

  if (cnt <= 1)
  {
    return(false);
  }

  VEvaluateRay(P0, P, dists[0], D);

  v0 = evaluate_superellipsoid(P0, Superellipsoid);

  if (fabs(v0) < ZERO_TOLERANCE)
  {
    if (insert_hit((OBJECT *)Superellipsoid, Ray, dists[0] / len, Depth_Stack))
    {
      if (Superellipsoid->Type & IS_CHILD_OBJECT)
      {
        Found = true;
      }
      else
      {
        return(true);
      }
    }
  }

  for (i = 1; i < cnt; i++)
  {
    VEvaluateRay(P1, P, dists[i], D);

    v1 = evaluate_superellipsoid(P1, Superellipsoid);

    if (fabs(v1) < ZERO_TOLERANCE)
    {
      if (insert_hit((OBJECT *)Superellipsoid, Ray, dists[i] / len, Depth_Stack))
      {
        if (Superellipsoid->Type & IS_CHILD_OBJECT)
        {
          Found = true;
        }
        else
        {
          return(true);
        }
      }
    }
    else
    {
      if (v0 * v1 < 0.0)
      {
        /* Opposite signs, there must be a root between */

        solve_hit1(Superellipsoid, v0, P0, v1, P1, P2);

        VSub(P3, P2, P);

        VLength(t, P3);

        if (insert_hit((OBJECT *)Superellipsoid, Ray, t / len, Depth_Stack))
        {
          if (Superellipsoid->Type & IS_CHILD_OBJECT)
          {
            Found = true;
          }
          else
          {
            return(true);
          }
        }
      }
      else
      {
        /* 
         * Although there was no sign change, we may actually be approaching
         * the surface. In this case, we are being fooled by the shape of the
         * surface into thinking there isn't a root between sample points. 
         */

        if (check_hit2(Superellipsoid, P, D, dists[i-1], P0, v0, dists[i], &t, P2))
        {
          if (insert_hit((OBJECT *)Superellipsoid, Ray, t / len, Depth_Stack))
          {
            if (Superellipsoid->Type & IS_CHILD_OBJECT)
            {
              Found = true;
            }
            else
            {
              return(true);
            }
          }
          else
          {
            break;
          }
        }
      }
    }

    v0 = v1;

    Assign_Vector(P0, P1);
  }

  return(Found);
}



/*****************************************************************************
*
* FUNCTION
*
*   Inside_Superellipsoid
*
* INPUT
*
*   IPoint - Intersection point
*   Object - Object
*   
* OUTPUT
*   
* RETURNS
*
*   int - true if inside
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Test if a point lies inside the superellipsoid.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

static int Inside_Superellipsoid(VECTOR IPoint, OBJECT *Object)
{
  DBL val;
  VECTOR P;
  SUPERELLIPSOID *Superellipsoid = (SUPERELLIPSOID *)Object;

  /* Transform the point into the superellipsoid space. */

  MInvTransPoint(P, IPoint, Superellipsoid->Trans);

  val = evaluate_superellipsoid(P, Superellipsoid);

  if (val < EPSILON)
  {
    return(!Test_Flag(Superellipsoid, INVERTED_FLAG));
  }
  else
  {
    return(Test_Flag(Superellipsoid, INVERTED_FLAG));
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Superellipsoid_Normal
*
* INPUT
*
*   Result - Normal vector
*   Object - Object
*   Inter  - Intersection found
*   
* OUTPUT
*
*   Result
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Calculate the normal of the superellipsoid in a given point.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

static void Superellipsoid_Normal(VECTOR Result, OBJECT *Object, INTERSECTION *Inter)
{
  SUPERELLIPSOID *Superellipsoid = (SUPERELLIPSOID *)Object;
  VECTOR const& E = Superellipsoid->Power;
  VECTOR P;

   /* Transform the point into the superellipsoid space. */
   MInvTransPoint(P, Inter->IPoint, Superellipsoid->Trans);

  DBL r, z2n = 0;
  if (P[Z] != 0)
  {
    z2n = power(fabs(P[Z]), E[Z]);
    P[Z] = z2n / P[Z];
  }

  if (fabs(P[X]) > fabs(P[Y]))
  {
    r = power(fabs(P[Y] / P[X]), E[X]);

    P[X] = (1-z2n)  /  P[X];
    P[Y] = P[Y] ? (1-z2n) * r / P[Y] : 0;
  }
  else if (P[Y] != 0)
  {
    r = power(fabs(P[X] / P[Y]), E[X]);

    P[X] = P[X] ? (1-z2n) * r / P[X] : 0;
    P[Y] = (1-z2n) / P[Y];
  }
  if(P[Z])
    P[Z] *= (1 + r);

  /* Transform the normalt out of the superellipsoid space. */
  MTransNormal(Result, P, Superellipsoid->Trans);

  VNormalize(Result, Result);
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Superellipsoid
*
* INPUT
*
*   Object - Object
*   Vector - Translation vector
*
* OUTPUT
*
*   Object
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Translate a superellipsoid.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

static void Translate_Superellipsoid(OBJECT *Object, VECTOR, TRANSFORM *Trans)
{
  Transform_Superellipsoid(Object, Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Superellipsoid
*
* INPUT
*
*   Object - Object
*   Vector - Rotation vector
*   
* OUTPUT
*
*   Object
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Rotate a superellipsoid.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

static void Rotate_Superellipsoid(OBJECT *Object, VECTOR, TRANSFORM *Trans)
{
  Transform_Superellipsoid(Object, Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Superellipsoid
*
* INPUT
*
*   Object - Object
*   Vector - Scaling vector
*   
* OUTPUT
*
*   Object
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Scale a superellipsoid.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

static void Scale_Superellipsoid(OBJECT *Object, VECTOR, TRANSFORM *Trans)
{
  Transform_Superellipsoid(Object, Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Superellipsoid
*
* INPUT
*
*   Object - Object
*   Trans  - Transformation to apply
*   
* OUTPUT
*
*   Object
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Transform a superellipsoid and recalculate its bounding box.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

static void Transform_Superellipsoid(OBJECT *Object, TRANSFORM *Trans)
{
  Compose_Transforms(((SUPERELLIPSOID *)Object)->Trans, Trans);

  Compute_Superellipsoid_BBox((SUPERELLIPSOID *)Object);
}



/*****************************************************************************
*
* FUNCTION
*
*   Invert_Superellipsoid
*
* INPUT
*
*   Object - Object
*   
* OUTPUT
*
*   Object
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Invert a superellipsoid.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

static void Invert_Superellipsoid(OBJECT *Object)
{
  Invert_Flag(Object, INVERTED_FLAG);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Superellipsoid
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   SUPERELLIPSOID * - new superellipsoid
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Create a new superellipsoid.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

SUPERELLIPSOID *Create_Superellipsoid()
{
  SUPERELLIPSOID *New;

  New = (SUPERELLIPSOID *)POV_MALLOC(sizeof(SUPERELLIPSOID), "superellipsoid");

  INIT_OBJECT_FIELDS(New,SUPERELLIPSOID_OBJECT,&Superellipsoid_Methods)

  New->Trans = Create_Transform();

  Make_Vector(New->Power, 2.0, 2.0, 2.0);

  return(New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Superellipsoid
*
* INPUT
*
*   Object - Object
*   
* OUTPUT
*   
* RETURNS
*
*   void * - New superellipsoid
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Copy a superellipsoid structure.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

static SUPERELLIPSOID *Copy_Superellipsoid(OBJECT *Object)
{
  SUPERELLIPSOID *New, *Superellipsoid = (SUPERELLIPSOID *)Object;

  New = Create_Superellipsoid();

  /* Get rid of the transformation created in Create_Superellipsoid(). */

  Destroy_Transform(New->Trans);

  /* Copy superellipsoid. */

  *New = *Superellipsoid;

  New->Trans = Copy_Transform(Superellipsoid->Trans);

  return(New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Superellipsoid
*
* INPUT
*
*   Object - Object
*   
* OUTPUT
*
*   Object
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Destroy a superellipsoid.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

static void Destroy_Superellipsoid(OBJECT *Object)
{
  SUPERELLIPSOID *Superellipsoid = (SUPERELLIPSOID *)Object;

  Destroy_Transform(Superellipsoid->Trans);

  POV_FREE (Object);
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Superellipsoid_BBox
*
* INPUT
*
*   Superellipsoid - Superellipsoid
*   
* OUTPUT
*
*   Superellipsoid
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Calculate the bounding box of a superellipsoid.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

void Compute_Superellipsoid_BBox(SUPERELLIPSOID *Superellipsoid)
{
  Make_BBox(Superellipsoid->BBox, -1.0001, -1.0001, -1.0001, 2.0002, 2.0002, 2.0002);

  Recompute_BBox(&Superellipsoid->BBox, Superellipsoid->Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   intersect_box
*
* INPUT
*
*   P, D       - Ray origin and direction
*   dmin, dmax - Intersection depths
*   
* OUTPUT
*
*   dmin, dmax
*   
* RETURNS
*
*   int - true, if hit
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Intersect a ray with an axis aligned unit box.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

static int intersect_box(VECTOR P, VECTOR  D, DBL *dmin, DBL  *dmax)
{
  DBL tmin = 0.0, tmax = 0.0;

  /* Left/right. */

  if (fabs(D[X]) > EPSILON)
  {
    if (D[X] > EPSILON)
    {
      *dmin = (MIN_VALUE - P[X]) / D[X];

      *dmax = (MAX_VALUE - P[X]) / D[X];

      if (*dmax < EPSILON) return(false);
    }
    else
    {
      *dmax = (MIN_VALUE - P[X]) / D[X];

      if (*dmax < EPSILON) return(false);

      *dmin = (MAX_VALUE - P[X]) / D[X];
    }

    if (*dmin > *dmax) return(false);
  }
  else
  {
    if ((P[X] < MIN_VALUE) || (P[X] > MAX_VALUE))
    {
      return(false);
    }

    *dmin = -BOUND_HUGE;
    *dmax =  BOUND_HUGE;
  }

  /* Top/bottom. */

  if (fabs(D[Y]) > EPSILON)
  {
    if (D[Y] > EPSILON)
    {
      tmin = (MIN_VALUE - P[Y]) / D[Y];

      tmax = (MAX_VALUE - P[Y]) / D[Y];
    }
    else
    {
      tmax = (MIN_VALUE - P[Y]) / D[Y];

      tmin = (MAX_VALUE - P[Y]) / D[Y];
    }

    if (tmax < *dmax)
    {
      if (tmax < EPSILON) return(false);

      if (tmin > *dmin)
      {
        if (tmin > tmax) return(false);

        *dmin = tmin;
      }
      else
      {
        if (*dmin > tmax) return(false);
      }

      *dmax = tmax;
    }
    else
    {
      if (tmin > *dmin)
      {
        if (tmin > *dmax) return(false);

        *dmin = tmin;
      }
    }
  }
  else
  {
    if ((P[Y] < MIN_VALUE) || (P[Y] > MAX_VALUE))
    {
      return(false);
    }
  }

  /* Front/back. */

  if (fabs(D[Z]) > EPSILON)
  {
    if (D[Z] > EPSILON)
    {
      tmin = (MIN_VALUE - P[Z]) / D[Z];

      tmax = (MAX_VALUE - P[Z]) / D[Z];
    }
    else
    {
      tmax = (MIN_VALUE - P[Z]) / D[Z];

      tmin = (MAX_VALUE - P[Z]) / D[Z];
    }

    if (tmax < *dmax)
    {
      if (tmax < EPSILON) return(false);

      if (tmin > *dmin)
      {
        if (tmin > tmax) return(false);

        *dmin = tmin;
      }
      else
      {
        if (*dmin > tmax) return(false);
      }

      *dmax = tmax;
    }
    else
    {
      if (tmin > *dmin)
      {
        if (tmin > *dmax) return(false);

        *dmin = tmin;
      }
    }
  }
  else
  {
    if ((P[Z] < MIN_VALUE) || (P[Z] > MAX_VALUE))
    {
      return(false);
    }
  }

  return(true);
}



/*****************************************************************************
*
* FUNCTION
*
*   evaluate_g
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   DBL
*   
* AUTHOR
*
*   Massimo Valentini
*   
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

static DBL evaluate_g(DBL x, DBL y, DBL e)
{
  DBL g = 0;

  if (x > y)
  {
    g = 1 + power(y/x, e);
    if(g != 1)
      g = power(g, 1/e);
    g *= x;
  }
  else if (y != 0)
  {
    g = 1 + power(x/y, e);
    if(g != 1)
      g = power(g, 1/e);
    g *= y;
  }

  return g;
}



/*****************************************************************************
*
* FUNCTION
*
*   evaluate_superellipsoid
*
* INPUT
*
*   P          - Point to evaluate
*   Superellipsoid - Pointer to superellipsoid
*   
* OUTPUT
*   
* RETURNS
*
*   DBL
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Get superellipsoid value in the given point.
*
* CHANGES
*
*   Oct 1994 : Creation.
*   Dec 2002 : Rewritten by Massimo Valentini.
*
******************************************************************************/

static DBL evaluate_superellipsoid(VECTOR P, SUPERELLIPSOID *Superellipsoid)
{
  return evaluate_g(evaluate_g(fabs(P[X]), fabs(P[Y]), Superellipsoid->Power[X]), fabs(P[Z]), Superellipsoid->Power[Z]) - 1;
}



/*****************************************************************************
*
* FUNCTION
*
*   power
*
* INPUT
*
*   x - Argument
*   e - Power
*   
* OUTPUT
*   
* RETURNS
*
*   DBL
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Raise x to the power of e.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

static DBL power(DBL x, DBL  e)
{
  register int i;
  register DBL b;

  b = x;

  i = (int)e;

  /* Test if we have an integer power. */

  if (e == (DBL)i)
  {
    switch (i)
    {
      case 0: return(1.0);

      case 1: return(b);

      case 2: return(Sqr(b));

      case 3: return(Sqr(b) * b);

      case 4: b *= b; return(Sqr(b));

      case 5: b *= b; return(Sqr(b) * x);

      case 6: b *= b; return(Sqr(b) * b);

      default: return(pow(x, e));
    }
  }
  else
  {
    return(pow(x, e));
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   insert_hit
*
* INPUT
*
*   Object      - Object
*   Ray         - Ray
*   Depth       - Intersection depth
*   Depth_Stack - Intersection stack
*   
* OUTPUT
*
*   Depth_Stack
*   
* RETURNS
*
*   int - true, if intersection is valid
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Push an intersection onto the depth stack if it is valid.
*
* CHANGES
*
*   Nov 1994 : Creation.
*
******************************************************************************/

static int insert_hit(OBJECT *Object, RAY *Ray, DBL Depth, ISTACK *Depth_Stack)
{
  VECTOR IPoint;

  if ((Depth > DEPTH_TOLERANCE) && (Depth < Max_Distance))
  {
    VEvaluateRay(IPoint, Ray->Initial, Depth, Ray->Direction);

    if (Point_In_Clip(IPoint, Object->Clip))
    {
      push_entry(Depth, IPoint, Object, Depth_Stack);

      return(true);
    }
  }

  return(false);
}



/*****************************************************************************
*
* FUNCTION
*
*   compdists
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
*   
* DESCRIPTION
*
*   Compare two slabs.
*
* CHANGES
*
*   Nov 1994 : Creation.
*
******************************************************************************/

static int compdists(const void *in_a, const void *in_b)
{
  DBL a, b;

  a = *((DBL *)in_a);
  b = *((DBL *)in_b);

  if (a < b)
  {
    return(-1);
  }

  if (a == b)
  {
    return(0);
  }
  else
  {
    return(1);
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   find_ray_plane_points
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
*   
* DESCRIPTION
*
*   Find all the places where the ray intersects the set of
*   subdividing planes through the superquadric.  Return the
*   number of valid hits (within the bounding box).
*
* CHANGES
*
*   Nov 1994 : Creation.
*
******************************************************************************/

static int find_ray_plane_points(VECTOR P, VECTOR  D, int cnt, DBL *dists, DBL  mindist, DBL  maxdist)
{
  int i;
  DBL t, d;

  /* Since min and max dist are the distance to two of the bounding planes
     we are considering, there is a high probablity of missing them due to
     round off error. Therefore we adjust min and max. */

  t = EPSILON * (maxdist - mindist);

  mindist -= t;
  maxdist += t;

  /* Check the sets of planes that cut apart the superquadric. */

  for (i = 0; i < PLANECOUNT; i++)
  {
    d = (D[0] * planes[i][0] + D[1] * planes[i][1] + D[2] * planes[i][2]);

    if (fabs(d) < EPSILON)
    {
      /* Can't possibly get a hit for this combination of ray and plane. */

      continue;
    }

    t = (planes[i][3] - (P[0] * planes[i][0] + P[1] * planes[i][1] + P[2] * planes[i][2])) / d;

    if ((t >= mindist) && (t <= maxdist))
    {
      dists[cnt++] = t;
    }
  }

  /* Sort the results for further processing. */
#if !defined(SPEC_CPU)
  QSORT((void *)(dists), cnt, sizeof(DBL), compdists);
#else
  spec_qsort((void *)(dists), cnt, sizeof(DBL), &compdists);
#endif /* SPEC_CPU */
  return(cnt);
}



/*****************************************************************************
*
* FUNCTION
*
*   solve_hit1
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
*   
* DESCRIPTION
*
*   Home in on the root of a superquadric using a combination of
*   secant and bisection methods.  This routine requires that
*   the sign of the function be different at P0 and P1, it will
*   fail drastically if this isn't the case.
*
* CHANGES
*
*   Nov 1994 : Creation.
*
******************************************************************************/

static void solve_hit1(SUPERELLIPSOID *Super, DBL v0, VECTOR tP0, DBL  v1, VECTOR  tP1, VECTOR  P)
{
  int i;
  DBL x, v2, v3;
  VECTOR P0, P1, P2, P3;

  Assign_Vector(P0, tP0);
  Assign_Vector(P1, tP1);

  /* The sign of v0 and v1 changes between P0 and P1, this
     means there is an intersection point in there somewhere. */

  for (i = 0; i < MAX_ITERATIONS; i++)
  {
    if (fabs(v0) < ZERO_TOLERANCE)
    {
      /* Near point is close enough to an intersection - just use it. */

      Assign_Vector(P, P0);

      break;
    }

    if (fabs(v1) < ZERO_TOLERANCE)
    {
      /* Far point is close enough to an intersection. */

      Assign_Vector(P, P1);

      break;
    }

    /* Look at the chord connecting P0 and P1. */

    /* Assume a line between the points. */

    x = fabs(v0) / fabs(v1 - v0);

    VSub(P2, P1, P0);

    VAddScaled(P2, P0, x, P2);

    v2 = evaluate_superellipsoid(P2, Super);

    /* Look at the midpoint between P0 and P1. */

    VSub(P3, P1, P0);

    VAddScaled(P3, P0, 0.5, P3);

    v3 = evaluate_superellipsoid(P3, Super);

	if( v2 * v3 < 0.0 )
	{
         /* We can move both ends. */

		  v0 = v2;
          Assign_Vector(P0, P2);

		  v1 = v3;
          Assign_Vector(P1, P3);
	}
	else
	{
		if( fabs(v2) < fabs(v3) )
		{
			/* secant method is doing better */
			if( v0 * v2 < 0)
			{
				v1 = v2;
		        Assign_Vector(P1, P2);
			}
			else
			{
				v0 = v2;
		        Assign_Vector(P0, P2);
			}
		}
		else
		{
			/* bisection method is doing better */
			if( v0 * v3 < 0)
			{
				v1 = v3;
		        Assign_Vector(P1, P3);
			}
			else
			{
				v0 = v3;
		        Assign_Vector(P0, P3);
			}
		}
	}
  }

  if (i == MAX_ITERATIONS)
  {
    // The loop never quite closed in on the result - just use the point
    // closest to zero.  This really shouldn't happen since the max number
    // of iterations is enough to converge with straight bisection.

    if (fabs(v0) < fabs(v1))
    {
      Assign_Vector(P, P0);
    }
    else
    {
      Assign_Vector(P, P1);
    }
  }
}




/*****************************************************************************
*
* FUNCTION
*
*   check_hit2
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
*   
* DESCRIPTION
*
*   Try to find the root of a superquadric using Newtons method.
*
* CHANGES
*
*   Nov 1994 : Creation.
*
******************************************************************************/

static int check_hit2(SUPERELLIPSOID *Super, VECTOR P, VECTOR  D, DBL t0, VECTOR  P0, DBL  v0, DBL  t1, DBL  *t, VECTOR  Q)
{
  int i;
  DBL dt0, dt1, v1, deltat, maxdelta;
  VECTOR P1;

  dt0 = t0;
  dt1 = t0 + 0.0001 * (t1 - t0);

  maxdelta = t1 - t0;

  for (i = 0; (dt0 < t1) && (i < MAX_ITERATIONS); i++)
  {
    VEvaluateRay(P1, P, dt1, D);

    v1 = evaluate_superellipsoid(P1, Super);

    if (v0 * v1 < 0)
    {
      /* Found a crossing point, go back and use normal root solving. */

      solve_hit1(Super, v0, P0, v1, P1, Q);

      VSub(P0, Q, P);

      VLength(*t, P0);

      return(true);
    }
    else
    {
      if (fabs(v1) < ZERO_TOLERANCE)
      {
         VEvaluateRay(Q, P, dt1, D);

         *t = dt1;

         return(true);
      }
      else
      {
        if (((v0 > 0.0) && (v1 > v0)) || ((v0 < 0.0) && (v1 < v0)))
        {
          /* We definitely failed */

          break;
        }
        else
        {
          if (v1 == v0)
          {
            break;
          }
          else
          {
            deltat = v1 * (dt1 - dt0) / (v1 - v0);
          }
        }
      }
    }

    if (fabs(deltat) > maxdelta)
    {
       break;
    }

    v0 = v1;

    dt0 = dt1;

    dt1 -= deltat;
  }

  return(false);
}

END_POV_NAMESPACE
