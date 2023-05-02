c++ openfoam code for transforming openfoam mesh to SU2 mesh
First activate your openfoam (tested on openfoam2206) and type
```./Allwmake```
thet you can call ```foamToSU2``` to create SU2 file.
Sadly SU2 suports limited amount of cells, so it works only with blockMesh and simple tetMesh (modification of CFMesh)