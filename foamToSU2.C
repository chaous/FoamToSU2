/*---------------------------------------------------------------------------*\
Transforming openfoam mesh to su2 format

\*---------------------------------------------------------------------------*/
#include <fstream>

#include "fvCFD.H"



// transform foam Face to su2 format
int foamConToSU2(int elemSize)
{
    switch (elemSize)
    {
    case 2:
        return 3;

    case 3:
        return 5;

    case 4:
        return 9;

    case 5:
        return 10;

    case 6:
        return 12;

    case 7:
        return 13;
    
    case 8:
        return 14;

    default:
    Info << elemSize << endl;
        //throw std::runtime_error("Unsupported element size");
    }

}


// foam cells to su2 format
int foamCellToSU2(const Foam::word name)
{
    if (name == "hex")
        return 12;
    
    if (name == "wedge")
        throw std::runtime_error("Wage cell type not supported");
        //return 13;

    if (name == "prism")
        return 13;

    if (name == "pyr")
        return 14;

    if (name == "tet")
        return 10;
    
    if (name == "tetWedge")
        throw std::runtime_error("Polyhedron cell type not supported");

    Info  << "Cell type: " << name << endl;
    // Info << "Number of faces: " << cell().nFaces() << endl;
    //Info << "Number of edges: " << cell().faces() << endl;
    //Info << "Number of points: " << cell().points().size() << endl;
    //throw std::runtime_error("Cell type not supported (boundary writing))");
    return -1;

}

// writing cell indexes to file only for hex cells 
void hexCellToFile(const Foam::cellShape cell, std::ofstream& myfile)
{

    if (cell.model().name() != "hex")
        throw std::runtime_error("Cell type not supported");


    unsigned counter = 0;

        forAll(cell.faces(), faceI)
        {
            if (counter < 2)
                {
                    ++counter;
                    continue;
                }
            break;
            if (counter == 3 || counter == 4)
                {
                    forAll(cell.faces()[faceI], pointI)
                        myfile << cell.faces()[faceI][pointI] << ' ';
                ++counter;
                }
            else
                break;
                
        }
        myfile << '\n';
}

//write cell indexes to file in vtk/su2 format 
void cellToFile(const Foam::cellShape cell, std::ofstream& myfile)
{
    if (cell.model().name() == "hex")
    {
        myfile << cell[0] << ' ' << cell[1] << ' ' << cell[2] << ' ' << cell[3] << ' ' << cell[4] << ' ' << cell[5] << ' ' << cell[6] << ' ' << cell[7];
    }
    else if (cell.model().name() == "tet")
    {
        myfile << cell[0] << ' ' << cell[1] << ' ' << cell[2] << ' ' << cell[3];
    }
    else if (cell.model().name() == "pyr")
    {
        myfile << cell[0] << ' ' << cell[1] << ' ' << cell[2] << ' ' << cell[3] << ' ' << cell[4];
    }
    else if (cell.model().name() == "prism")
    {
        myfile << cell[0] << ' ' << cell[1] << ' ' << cell[2] << ' ' << cell[3] << ' ' << cell[4] << ' ' << cell[5];
    }
    else if (cell.model().name() == "wedge")
    {
        myfile << cell[0] << ' ' << cell[1] << ' ' << cell[2] << ' ' << cell[3] << ' ' << cell[4] << ' ' << cell[5];
    }
    else
    {
        //throw std::runtime_error("Cell type not supported (cell writing)");
        Info  << "Cell type: " << cell.model() << endl;
        //Info << "Number of faces: " << cell().nFaces() << endl;
    }
}

int getNumberOfNotEmpyCells(const cellShapeList& cellShapes)
{
    int counter = 0;
    forAll(cellShapes, cellI)
    {
        if (cellShapes[cellI].model().name() != "unknown")
            ++counter;
    }
    return counter;
}


int main(int argc, char *argv[])
{
    #include "setRootCase.H"

	
    #include "createTime.H"
    #include "createMesh.H"

	std::ofstream myfile;
    myfile.open("foam.su2");

    myfile << "NDIME=3\n";

    const List<point>& pts = mesh.points();

    myfile << "NPOIN=" << mesh.points().size() << "\n";

    // Write points
    unsigned counter = 0;
    for (label pointI = 0; pointI < mesh.points().size(); pointI++)
    {
        myfile << pts[pointI].x() << " " << pts[pointI].y() << " " << pts[pointI].z()  << ' ' << counter << "\n";
        ++counter;
    }

    Info << "Number of points: " << counter << ' '<< mesh.points().size() << endl;


    // Write cells indexes
    //myfile << "NELEM= " <<  mesh.C().size() << "\n";
    myfile << "NELEM= " <<  getNumberOfNotEmpyCells(mesh.cellShapes()) << "\n";

    const cellShapeList& cellShapes = mesh.cellShapes();
    counter = 0;
    forAll(cellShapes, cellI)
    {
        int cellTyoe = foamCellToSU2(cellShapes[cellI].model().name());
        if (cellTyoe == -1)
            continue;
        myfile << foamCellToSU2(cellShapes[cellI].model().name()) << " ";
        cellToFile(cellShapes[cellI], myfile);
        myfile << ' ' << counter << "\n";
        ++counter;
    }

    // Write boundary faces
    counter = 0;
    myfile << "NMARK= " << mesh.boundaryMesh().size() << "\n";
    forAll(mesh.boundaryMesh(), patchI)
    {
        myfile << "MARKER_TAG= " << mesh.boundaryMesh()[patchI].name() << "\n";
        myfile << "MARKER_ELEMS= " << mesh.boundary()[patchI].Cf().size() << "\n";
        
        for(label faceI = mesh.boundary()[patchI].start(); faceI < mesh.boundary()[patchI].start() + mesh.boundary()[patchI].Cf().size(); faceI++)
        {
            myfile << foamConToSU2(mesh.faces()[faceI].size()) << " ";
            for (label pointI = 0; pointI < mesh.faces()[faceI].size(); pointI++)
            {
                myfile << mesh.faces()[faceI][pointI] << " ";
            }
            myfile << counter << "\n";
            ++counter;
        }

    }

    myfile.close();
    return 0;
	
}

