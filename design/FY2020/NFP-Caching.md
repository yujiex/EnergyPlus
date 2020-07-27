# Caching EnergyPlus Data

**Matt Mitchell, NREL**

- Draft Date: July 21, 2020

## Introduction

EnergyPlus simulations are nearly always run multiple times during model development and design cycles, however, some of the computations performed are likely to never change for subsequent runs. Examples of these are calculations to generate conduction transfer functions. Once the building type and associated constructions have been specified, the CTF values computed will never change. Currently, these CTF values are recomputed during each simulation but this could potentially be eliminated if methods were developed for storing and reusing the CTF values.

Generation of the ground heat exchanger response factor values suffer from a similar problem; however, a simple caching method has been implemented. Once computed, the response factors along with the key GHE configuration parameters are stored in a JSON file in the simulation run directory. During the next simulation, EnergyPlus checks for this file and compares the GHE parameters to those in the input file. If they match to within tolerance, the response factors are loaded from the file; if not, the response factors are recomputed and the GHE cache file is overwritten with the new parameters and values.

HPC users may also see benefits from a supported data caching method. HPC users make many millions of simulations annually using EnergyPlus, oftentimes varying only a handful of parameters. It's likely that some of the data computed during each simulation, (e.g. CTFs and other data) could be distributed with the simulations prior to execution to help reduce simulation times. Even if the fraction of simulation time saved per simulation is small, the resulting total time to execute a batch of simulations could see meaningful reductions in runtime.
 
Beyond the CTFs and GHE data and HPC special use case, there likely exists other data and use cases which could benefit from methods for reusing calculation data. Here we should outline the major questions that need to be answered and work toward developing a design that enables computation reuse.

## Major Questions

## What values could/should be saved?

- CTFs
- GHE response factors
- ... is that it? There's got to be other data.

### Number of caches/cache files?

- I'm not sure if having one file is "ideal" or not. Perhaps there's a case to be made for multiple files, but there should be an upper limit.
- Maybe it's possible to separate the cache types in some logical way? (e.g. constructions, HVAC, plant, environment, etc.)

### Format?

- JSON
- Binary JSON
- TOML
- CSV

CTFs need to preserve high precision, so this use case needs to be enabled.

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

- At some point, caching may not be the fastest solution. Perhaps if they have 3 CTFs to compute the time to load the file and compare the results is greater than just recomputing.
- How is this determined?

### Is this related to "reset state" work?

- Same data format between cache and reset-state?
- Does the cache file eventually become the file needed to enable state resets?
- We don't need to get too hung up on this, but there may be some overlap.

### Testing

**Some notes from Rick Strand**

Precision needs to be preserved so that round off error doesn't cause problems. However, it's not currently know how what effect a given round off error will have on the results. To test this, tests need to be generated to test the following:

- Lightweight constructions
- Heavyweight constructions
- Various combinations of insulation and mass
- Constructions with up to 10 layers
- Combinations of all of the above

2D systems and more nodes will also require additional computation time. To thoroughly test this, it would be useful to extend the API to not only be able to access the CTF generation methods, but it would also be useful to access the methods consuming the CTFs. I haven't thought through all of the implications of this, but it would be useful to see the implications of the above test without needing to run a full simulation.
