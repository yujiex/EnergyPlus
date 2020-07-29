# Caching CTF Data

**Matt Mitchell, NREL**

- Draft Date: July 21, 2020
- Revised: July 27, 2020. Adding comments from Rick Strand
- Revised: July 29, 2020. Adding comments from Amir Roth

## Introduction

EnergyPlus simulations are nearly always run multiple times during model development and design cycles; however, some of the computations performed are likely to never change for subsequent runs. One of these data which are regenerated during each run are conduction transfer functions (CTF). Once the building type and associated constructions have been specified, the CTF values computed will never change. Currently, these CTF values are recomputed during each simulation but this could potentially be eliminated if methods were developed for storing and reusing the CTF values.

The computation time for computing CTF values increases non-linearly with the number of layers in the construction. Similarly, the time required for generating CTF values for 2D radiant systems also increases due to the increasing number of nodes. In general, as the number of layers or nodes increases, so too does the required computation time.

HPC users may also see benefits from a supported data caching method. HPC users make many millions of simulations annually using EnergyPlus, oftentimes varying only a handful of parameters. It's likely that some of the data computed during each simulation, (e.g. CTFs) could be distributed with the simulations prior to execution to help reduce simulation times. Even if the fraction of simulation time saved per simulation is small, the resulting total time to execute a batch of simulations could see meaningful reductions in runtime.
 
Beyond the CTFs and HPC special use case, there likely exists other data and use cases which could benefit from methods for reusing calculation data. For example, the ground heat exchanger model already implements a basic data caching method, but perhapse there are other model data which could be stored or saved. However, this work will focus on testing whether saving CTF values can be done effectively. Saving other data will be treated separately.

## Approach

### Cache file format

The proposes implementation will not attempt to combine multiple data types. The CTF data will be stored in a file named "eplus.cache.ctfs.<ext>". File formatting will be largely determined by the selected file format (e.g. JSON, text, ect.). This will continue to be evaluated during development.
### Testing

Precision needs to be preserved so that round off error doesn't cause problems. However, it's not currently know how what effect a given round off error will have on the results. To test this, tests need to be generated to test the following:

- Lightweight constructions
- Heavyweight constructions
- Various combinations of insulation and mass
- Constructions with up to 10 layers
- Combinations of all of the above

2D systems and more nodes will also require additional computation time. To thoroughly test this, it would be useful to extend the API to not only be able to access the CTF generation methods, but it would also be useful to access the methods consuming the CTFs. I haven't thought through all of the implications of this, but it would be useful to see the implications of the above test without needing to run a full simulation.

### Next steps

Begin developing tests as indicated above to determine whether caching can be effective, and under what situations it make sense.

## Remaining questions

### Where does cache data live?

- Install directory?
- Run directory?
- User-specified location?
- If the simulation can access multiple caches, which is given priority?

### Cache size limits?

- When do we warn the user to clear the cache?
- Does it get continually appended to, or updated each time, or do values roll off after a certain number of simulation calls?

### Cache matching

- How are the parameter tolerance values determined? 
- What happens when you begin to chain tolerances together?
- Do all parameters values need to match or could some fuzzy matching be accepted?

### Caching effectiveness

At some point, caching may not be the fastest solution. Perhaps if they have 3 CTFs to compute the time to load the file and compare the results is greater than just recomputing. This will be evaluated during testing.

