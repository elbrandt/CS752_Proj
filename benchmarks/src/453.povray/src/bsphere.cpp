/*****************************************************************************
 *               bsphere.cpp
 *
 * This module implements the bounding sphere calculations.
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
 * $File: //depot/povray/3.5/source/bsphere.cpp $
 * $Revision: #19 $
 * $Change: 3010 $
 * $DateTime: 2004/07/27 14:06:43 $
 * $Author: thorsten $
 * $Log$
 *****************************************************************************/

#include "frame.h"
#include "vector.h"
#include "bsphere.h"
#include "povray.h"
#include "spec_qsort.h"

#include <algorithm>

BEGIN_POV_NAMESPACE

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

const int BRANCHING_FACTOR = 4;



/*****************************************************************************
* Local typedefs
******************************************************************************/



/*****************************************************************************
* Local variables
******************************************************************************/

static int maxelements, Axis; // GLOBAL VARIABLE



/*****************************************************************************
* Static functions
******************************************************************************/

static void merge_spheres (VECTOR C, DBL *r, VECTOR C1, DBL r1, VECTOR C2, DBL r2);
static void recompute_bound (BSPHERE_TREE *Node);
static int CDECL comp_elements (const void *in_a, const void *in_b);

/*****************************************************************************
*
* FUNCTION
*
*   merge_spheres
*
* INPUT
*
*   C, r           - Center and radius^2 of new sphere
*   C1, r1, C2, r2 - Centers and radii^2 of spheres to merge
*   
* OUTPUT
*
*   C, r
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Calculate a sphere that encloses two given spheres.
*
* CHANGES
*
*   Jul 1994 : Creation.
*
*   Oct 1994 : Added test for enclosed spheres. Calculate optimal sphere. [DB]
*
******************************************************************************/

static void merge_spheres(VECTOR C, DBL *r, VECTOR  C1, DBL  r1, VECTOR  C2, DBL  r2)
{
  DBL l, r1r, r2r, k1, k2;
  VECTOR D;

  VSub(D, C1, C2);

  VLength(l, D);

  /* Check if one sphere encloses the other. */

  r1r = sqrt(r1);
  r2r = sqrt(r2);

  if (l + r1r <= r2r)
  {
    Assign_Vector(C, C2);

    *r = r2;

    return;
  }

  if (l + r2r <= r1r)
  {
    Assign_Vector(C, C1);

    *r = r1;

    return;
  }
  
  k1 = (1.0 + (r1r - r2r) / l) / 2.0;
  k2 = (1.0 + (r2r - r1r) / l) / 2.0;

  VLinComb2(C, k1, C1, k2, C2);

  *r = Sqr((l + r1r + r2r) / 2.0);
}



/*****************************************************************************
*
* FUNCTION
*
*   recompute_bound
*
* INPUT
*
*   Node - Pointer to node
*
* OUTPUT
*
*   Node
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Recompute the bounding sphere of a given node in the bounding hierarchy,
*   i. e. find the bounding sphere that encloses the bounding spheres
*   of all nodes.
*
*   NOTE: The sphere found is probably not the tightest sphere possible!
*
* CHANGES
*
*   Jul 1994 : Creation.
*
*   Oct 1994 : Improved bounding sphere calculation. [DB]
*
******************************************************************************/

static void recompute_bound(BSPHERE_TREE *Node)
{
  short i;
  DBL r2;
  VECTOR C;

  Assign_Vector(C, Node->Node[0]->C);

  r2 = Node->Node[0]->r2;

  for (i = 1; i < Node->Entries; i++)
  {
    merge_spheres(C, &r2, C, r2, Node->Node[i]->C, Node->Node[i]->r2);
  }

  Assign_Vector(Node->C, C);

  Node->r2 = r2;
}



/*****************************************************************************
*
* FUNCTION
*
*   comp_elements
*
* INPUT
*
*   in_a, in_b - elements to compare
*
* OUTPUT
*
* RETURNS
*
*   int - result of comparison
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Oct 1994 : Creation. (Derived from the bounding slab creation code)
*
******************************************************************************/

static int CDECL comp_elements(const void *in_a, const void *in_b)
{
  DBL am, bm;

  am = (*(BSPHERE_TREE **)in_a)->C[Axis];
  bm = (*(BSPHERE_TREE **)in_b)->C[Axis];

  if (am < bm - EPSILON)
  {
    return (-1);
  }
  else
  {
    if (am > bm + EPSILON)
    {
      return (1);
    }
    else
    {
      return (0);
    }
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   find_axis
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Oct 1994 : Creation. (Derived from the bounding slab creation code)
*
******************************************************************************/

static int find_axis(BSPHERE_TREE **Elements, int first, int  last)
{
  int which = X;
  int i;
  DBL e, d = - BOUND_HUGE;
  VECTOR C, mins, maxs;

  Make_Vector(mins,  BOUND_HUGE,  BOUND_HUGE,  BOUND_HUGE);
  Make_Vector(maxs, -BOUND_HUGE, -BOUND_HUGE, -BOUND_HUGE);

  for (i = first; i < last; i++)
  {
    Assign_Vector(C, Elements[i]->C);

    mins[X] = min(mins[X], C[X]);
    maxs[X] = max(maxs[X], C[X]);

    mins[Y] = min(mins[Y], C[Y]);
    maxs[Y] = max(maxs[Y], C[Y]);

    mins[Z] = min(mins[Z], C[Z]);
    maxs[Z] = max(maxs[Z], C[Z]);
  }

  e = maxs[X] - mins[X];

  if (e > d)
  {
    d = e;  which = X;
  }

  e = maxs[Y] - mins[Y];

  if (e > d)
  {
    d = e;  which = Y;
  }

  e = maxs[Z] - mins[Z];

  if (e > d)
  {
    which = Z;
  }

  return (which);
}



/*****************************************************************************
*
* FUNCTION
*
*   build_area_table
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Generates a table of bounding sphere surface areas.
*
* CHANGES
*
*   Oct 1994 : Creation. (Derived from the bounding slab creation code)
*
******************************************************************************/

static void build_area_table(BSPHERE_TREE **Elements, int a, int  b, DBL *areas)
{
  int i, imin, dir;
  DBL r2;
  VECTOR C;

  if (a < b)
  {
    imin = a;  dir = 1;
  }
  else
  {
    imin = b;  dir = -1;
  }

  Assign_Vector(C, Elements[a]->C);

  r2 = Elements[a]->r2;

  for (i = a; i != (b + dir); i += dir)
  {
    merge_spheres(C, &r2, C, r2, Elements[i]->C, Elements[i]->r2);

    areas[i-imin] = r2;
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   sort_and_split
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Oct 1994 : Creation. (Derived from the bounding slab creation code)
*
******************************************************************************/

static int sort_and_split(BSPHERE_TREE **Root, BSPHERE_TREE ***Elements, int *nElem, int  first, int  last)
{
  int size, i, best_loc;
  DBL *area_left, *area_right;
  DBL best_index, new_index;
  BSPHERE_TREE *cd;

  Axis = find_axis(*Elements, first, last);

  size = last - first;

  if (size <= 0)
  {
    return (1);
  }

  Do_Cooperate(1);

  /*
   * Actually, we could do this faster in several ways. We could use a
   * logn algorithm to find the median along the given axis, and then a
   * linear algorithm to partition along the axis. Oh well.
   */

#if !defined(SPEC_CPU)
 QSORT((void *)(*Elements + first), size, sizeof(BSPHERE_TREE *), comp_elements);
#else
 spec_qsort((void *)(*Elements + first), size, sizeof(BSPHERE_TREE *), comp_elements);
#endif /* SPEC_CPU */
  /*
   * area_left[] and area_right[] hold the surface areas of the bounding
   * boxes to the left and right of any given point. E.g. area_left[i] holds
   * the surface area of the bounding box containing Elements 0 through i and
   * area_right[i] holds the surface area of the box containing Elements
   * i through size-1.
   */

  area_left  = (DBL *)POV_MALLOC(size * sizeof(DBL), "blob bounding hierarchy");
  area_right = (DBL *)POV_MALLOC(size * sizeof(DBL), "blob bounding hierarchy");

  /* Precalculate the areas for speed. */

  build_area_table(*Elements, first, last - 1, area_left);
  build_area_table(*Elements, last - 1, first, area_right);

  best_index = area_right[0] * (size - 3.0);

  best_loc = - 1;

  /*
   * Find the most effective point to split. The best location will be
   * the one that minimizes the function N1*A1 + N2*A2 where N1 and N2
   * are the number of objects in the two groups and A1 and A2 are the
   * surface areas of the bounding boxes of the two groups.
   */

  for (i = 0; i < size - 1; i++)
  {
    new_index = (i + 1) * area_left[i] + (size - 1 - i) * area_right[i + 1];

    if (new_index < best_index)
    {
      best_index = new_index;
      best_loc = i + first;
    }
  }

  POV_FREE(area_left);
  POV_FREE(area_right);

  /*
   * Stop splitting if the BRANCHING_FACTOR is reached or
   * if splitting stops being effective.
   */

  if ((size <= BRANCHING_FACTOR) || (best_loc < 0))
  {
    cd = (BSPHERE_TREE *)POV_MALLOC(sizeof(BSPHERE_TREE), "blob bounding hierarchy");

    cd->Entries = (short)size;

    cd->Node = (BSPHERE_TREE **)POV_MALLOC(size*sizeof(BSPHERE_TREE *), "blob bounding hierarchy");

    for (i = 0; i < size; i++)
    {
      cd->Node[i] = (*Elements)[first+i];
    }

    recompute_bound(cd);

    *Root = cd;

    if (*nElem >= maxelements)
    {
      /* Prim array overrun, increase array by 50%. */

      maxelements = 1.5 * maxelements;

      /* For debugging only. */

      Debug_Info("Reallocing elements to %d\n", maxelements);

      *Elements = (BSPHERE_TREE **)POV_REALLOC(*Elements, maxelements * sizeof(BSPHERE_TREE *), "bounding slabs");
    }

    (*Elements)[*nElem] = cd;

    (*nElem)++;

    return (1);
  }
  else
  {
    sort_and_split(Root, Elements, nElem, first, best_loc + 1);

    sort_and_split(Root, Elements, nElem, best_loc + 1, last);

    return (0);
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Build_Bounding_Sphere_Hierarchy
*
* INPUT
*
*   nElem    - number of elements in Elements
*   Elements - array containing nElem elements
*
* OUTPUT
*
*   Root     - root node of the hierarchy
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Create the bounding sphere hierarchy for a given set of elements.
*   One element consists of an element number (index into the array
*   of elements; e.g. blob components, triangles) and a bounding
*   sphere enclosing this element (center and squared radius).
*
* CHANGES
*
*   Oct 1994 : Creation. (Derived from the bounding slab creation code)
*
******************************************************************************/

void Build_Bounding_Sphere_Hierarchy(BSPHERE_TREE **Root, int nElem, BSPHERE_TREE ***Elements)
{
  int low, high;

  /*
   * This is a resonable guess at the number of elements needed.
   * This array will be reallocated as needed if it isn't.
   */

  maxelements = 2 * nElem;

  /*
   * Do a sort on the elements, with the end result being
   * a tree of objects sorted along the x, y, and z axes.
   */

  if (nElem > 0)
  {
    low  = 0;
    high = nElem;

    while (sort_and_split(Root, Elements, &nElem, low, high) == 0)
    {
      low  = high;
      high = nElem;

      Do_Cooperate(0);
    }
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Bounding_Sphere_Hierarchy
*
* INPUT
*
*   Node - Pointer to current node
*
* OUTPUT
*
*   Node
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Destroy bounding sphere hierarchy.
*
* CHANGES
*
*   Aug 1994 : Creation.
*
*   Dec 1994 : Fixed memory leakage. [DB]
*
******************************************************************************/

void Destroy_Bounding_Sphere_Hierarchy(BSPHERE_TREE *Node)
{
  short i;

  if (Node != NULL)
  {
    if (Node->Entries > 0)
    {
      /* This is a node. Free sub-nodes. */

      for (i = 0; i < Node->Entries; i++)
      {
        Destroy_Bounding_Sphere_Hierarchy(Node->Node[i]);
      }

      POV_FREE(Node->Node);
    }

    POV_FREE(Node);
  }
}

END_POV_NAMESPACE
