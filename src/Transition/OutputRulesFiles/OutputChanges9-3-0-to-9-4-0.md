Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited.  It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo.  At least a PR number or whatever.


### Daylight Factors output in EIO and DFS now supports more than two reference points

[PR#8017](https://github.com/NREL/EnergyPlus/pull/8017) changed the output format in both EIO and DFS (when using `Output:DaylightFactors`)
to support more than two `Daylighting:ReferencePoint`.

#### EIO

The old format looked like the following, and when no second Reference Point exited, it reported zero for that point. Any additional reference point was not reported.

```
! <Sky Daylight Factors>, MonthAndDay, Zone Name, Window Name, Daylight Fac: Ref Pt #1, Daylight Fac: Ref Pt #2
 Clear Sky Daylight Factors,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,1.9074E-002,1.8074E-002
 Clear Turbid Sky Daylight Factors,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,1.4830E-002,1.4700E-002
 Intermediate Sky Daylight Factors,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,1.0647E-002,1.0547E-002
 Overcast Sky Daylight Factors,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,1.4127E-002,1.3127E-002
```

The new format looks like the following, and specifies the name of the reference point in question,
as well as conforming to EIO format by adding the section name `Sky Daylight Factors` to each record:

```
! <Sky Daylight Factors>, MonthAndDay, Zone Name, Window Name, Reference Point, Daylight Factor
 Sky Daylight Factors,Clear Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT1,1.9074E-002
 Sky Daylight Factors,Clear Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT2,1.8074E-002
 Sky Daylight Factors,Clear Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT3,1.7074E-002
 Sky Daylight Factors,Clear Turbid Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT1,1.4830E-002
 Sky Daylight Factors,Clear Turbid Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT2,1.4700E-002
 Sky Daylight Factors,Clear Turbid Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT3,1.4230E-002
 Sky Daylight Factors,Intermediate Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT1,1.0647E-002
 Sky Daylight Factors,Intermediate Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT2,1.0547E-002
 Sky Daylight Factors,Intermediate Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT3,1.0347E-002
 Sky Daylight Factors,Overcast Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT1,1.4127E-002
 Sky Daylight Factors,Overcast Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT2,1.3127E-002
 Sky Daylight Factors,Overcast Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT3,1.2127E-002
```

#### DFS

The old format looked like this, each actual line of record included the hour, then 2 groups of 4 different Sky Conditions corresponding to two reference points.
The second group was always zero if only one reference point existed (see the 2nd line of the header):

```
This file contains daylight factors for all exterior windows of daylight zones.
If only one reference point the last 4 columns in the data will be zero.
MonthAndDay,Zone Name,Window Name,Window State
Hour,Daylight Factor for Clear Sky at Reference point 1,Daylight Factor for Clear Turbid Sky at Reference point 1,Daylight Factor for Intermediate Sky at Reference point 1,Daylight Factor for Overcast Sky at Reference point 1,Daylight Factor for Clear Sky at Reference point 2,Daylight Factor for Clear Turbid Sky at Reference point 2,Daylight Factor for Intermediate Sky at Reference point 2,Daylight Factor for Overcast Sky at Reference point 2
01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,Base Window
1,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000
[...]
9,2.14657E-002,1.73681E-002,1.38241E-002,1.41272E-002,2.14657E-002,1.73680E-002,1.38240E-002,1.41273E-002
[...]
24,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000
```

In the new format, one line was stripped from the header, and the name of the reference point was added to each record. It now supports more than 2 reference points.
The former format for `Window State` used a *blank* to indicate a window in a shaded state by shades, screens, or blinds with a fixed slat angle.
Instead of a blank, the new format now uses `Blind or Slat Applied`.

```
This file contains daylight factors for all exterior windows of daylight zones.
MonthAndDay,Zone Name,Window Name,Window State
Hour,Reference Point,Daylight Factor for Clear Sky,Daylight Factor for Clear Turbid Sky,Daylight Factor for Intermediate Sky,Daylight Factor for Overcast Sky
01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,Base Window
1,ZN_1_DAYLREFPT1,0.00000,0.00000,0.00000,0.00000
1,ZN_1_DAYLREFPT2,0.00000,0.00000,0.00000,0.00000
1,ZN_1_DAYLREFPT3,0.00000,0.00000,0.00000,0.00000
[...]
9,ZN_1_DAYLREFPT1,2.14657E-002,1.73681E-002,1.38241E-002,1.41272E-002
9,ZN_1_DAYLREFPT2,2.14657E-002,1.73680E-002,1.38240E-002,1.41273E-002
9,ZN_1_DAYLREFPT3,2.14655E-002,1.73678E-002,1.38239E-002,1.41274E-002
[...]
24,ZN_1_DAYLREFPT1,0.00000,0.00000,0.00000,0.00000
24,ZN_1_DAYLREFPT2,0.00000,0.00000,0.00000,0.00000
24,ZN_1_DAYLREFPT3,0.00000,0.00000,0.00000,0.00000
```
### Surface Order in Output Reports

The internal ordering of surfaces has changed. Previously subsurfaces (doors and windows) immdediately followed their respective base surface. 
Now subsurfaces are at the end of each group of zone surfaces.Many reports preserve the old order, but some outputs do not.
Changed outputs include the rdd, edd, eso (and resulting csv), shd, and sci output files.

See [PR#7847](https://github.com/NREL/EnergyPlus/pull/7847)
