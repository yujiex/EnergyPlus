ASTM C1340 Convection Coefficient Algorithm for Radiant Barriers
================

**Dareum Nam, NREL**

 - Original Date 1/28/2021
 

## Justification for New Feature ##

Radiant barriers are a technology used in residential attics to reduce radiation heat transfer. It has been shown that EnergyPlus underestimates the savings of using these technologies compared to three other building models that specialized in attic models. The ASTM C1340 convection algorithm has been implemented in an EMS program to simulate radiant barriers and demonstrates a vast improvement in predicted performance. Introducing the ASTM C1340 algorithm into EnergyPlus would allow users to access the model more easily, reduce modeling time, and ensure proper implementation by users.

## E-mail and Conference Call Conclusions ##

insert text

## Overview ##

Overall, the task here is to implement the convection coefficients algorithm from ASTM C1340 standard. This convection algorithm for inside surfaces can be used for attics containing radiant barriers.

## Approach ##

Table 1 shows the correlations for convection coefficients from ASTM C1340 Standard.

![figure1](https://github.com/dareumnam/EnergyPlus/blob/RadiantBarrier/design/FY2021/table1.PNG)

ASTM C1340 Convection algorithm is implemented for SurfaceConvectionAlgorithm:Inside. 
For radiant barriers, this can be selected as `Zone Inside Convection Algorithm` of attic zone in `Zone` object.
- Energyplus has a range of 0 to 180 degrees of tilt angle of a surface. Flat roofs are tilted 0 degree, walls are tilted 90 degrees, and flat floors are tilted 180 degrees. Each case (horizontal surface, tilted surface, and vertical surface) can be classified by this tilt angle. 
- Heat flow direction (up and down) can be determined by the temperature difference between the surface and the air.
- For calculating the convection heat transfer coefficient, the following inputs for each surface are needed:
  - Length along the heat flow direction
    - The square root of surface area for floors and ceilings
    - Surface height for gables and walls
    - Length of pitched roof from soffit to ridge
  - Tilt angle
  - Velocity of the air stream
    - For interior surfaces, it is calculated from zone volume and air changes per hour
    - For exterior surfaces, Surface Outside Face Outdoor Air Wind Speed

## Testing/Validation/Data Sources ##

1. Standard unit tests were used to verify that the convective coefficients are calculated properly.
2. The convection coefficient results from the testfile with the EMS implementation were compared to the results from the implemented algorithm. 

## Input Output Reference Documentation ##


## Input Description ##

No new input descriptions are necessary for this work.

## Outputs Description ##

No new output descriptions are necessary for this work.

## Engineering Reference ##

insert text

## Example File and Transition Changes ##

N/A

## References ##

Fontanini, A. D., Aguilar, J. L. C., Mitchell, M. S., Kosny, J., Merket, N., DeGraw, J. W., & Lee, E. (2018). Predicting the performance of radiant technologies in attics: Reducing the discrepancies between attic specific and whole-building energy models. Energy and Buildings, 169, 69-83.

Standard Practice for Estimation of Heat Gain or Loss Through Ceilings Under Attics Containing Radiant Barriers by Use of a Computer Program


