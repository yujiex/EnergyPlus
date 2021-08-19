Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Daylight Map Report now supports more than two reference points
The header of the map file was changed from only allowing two reference points to allow more than two.  The additional points are listed in comma separated order where there used to be only two points.  The format will show up in the header as something like the following:

“ RefPt1=(2.50:2.00:0.80), RefPt2=(2.50:18.00:0.80), RefPt3=(2.50:18.00:0.50)”

[PR#8889](https://github.com/NREL/EnergyPlus/pull/8889) changed the output format in both the MAP file and the SQL output.

### Table Output Column Headings Changed for Central Plant:

 Modified Existing Column Headings:
 (a) "Nominal Capacity [W]" to "Reference Capacity [W]"
 (b) "Nominal Efficiency [W/W]" to "Reference Efficiency [W/W]"

 Added Two New Variables (Added Two New Column Headings):
 (a) "Rated Capacity [W]"
 (b) "Rated Efficiency [W/W]"
 
See [8192](https://github.com/NREL/EnergyPlus/pull/8959/)
See [PR#8889](https://github.com/NREL/EnergyPlus/pull/8889) changed the output format in both the MAP file and the SQL output.

### Table outputs for Space and Space Type

The following table outputs have changed:

**Lighting Summary**

*Interior Lighting* 

  * The "Zone" column heading was changed to "Zone Name".
  * new columns were added for "Space Name" and "Space Type".
  * The "Zone Area" column heading was change to "Space Area".

The following table outputs are new:

**Annual Building Utility Performance Summary**

*End Uses By Space Type*

**Input Verification and Results Summary**

*Space Summary*

*Space Type Summary*


See [PR#8394](https://github.com/NREL/EnergyPlus/pull/8394)
