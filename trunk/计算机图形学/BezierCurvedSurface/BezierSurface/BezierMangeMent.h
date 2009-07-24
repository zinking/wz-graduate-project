#include <vector>
#include "Beziersurface.h"

using namespace std;



 
class BezierManager{

private:
	vector<BezierPatch*> container;
	BezierPatch* currentpatch;
	BezierSurfacePatch* patch_a;
	BezierSurfacePatch* patch_b;
	TriangularBezierPatch* patch_c;
	int demo_index;
private:

	void Demo1( int ActionNumber ){
		switch( ActionNumber ){
			case 1:
				RemoveAllPatches();
				patch_a = new BezierSurfacePatch(3,3);
				container.push_back((BezierPatch*) patch_a);
				currentpatch = (BezierPatch*) patch_a;
				patch_c = new TriangularBezierPatch;
				patch_c->InitializeCtrlPointOffset( 0, 1 );
				container.push_back((BezierPatch*) patch_c );
				break;
			case 2:
				//patch_a->ConnectWithAnotherPatchWithSameDegree( patch_b, TOP,BOTTOM);
				patch_c =(TriangularBezierPatch*) container[1];
				patch_a =(BezierSurfacePatch*) container[0];
				patch_c ->ConnectWithQuadBezierSurface( *patch_a );
				break;
		}

	}
	void Demo2( int ActionNumber ){
		//patch_a->UpgradeBezierPatch();
		switch ( ActionNumber){
			case 1://initialize
				RemoveAllPatches();
				for( int i=0; i < 3; i++ ){
					for( int j=0; j < 3; j++ ){
						
						if( i==1 && j==1 ){
							patch_b = new BezierSurfacePatch(3,3);
						}
						else patch_b = new BezierSurfacePatch(4,4);
						patch_b->InitializeCtrlPointOffset( i, j );
						container.push_back( patch_b );
					}
				}
				currentpatch = (BezierPatch*) patch_b;
				break;
			case 2://connecting
				//vertical connecting
				for( int j=0;j<3;j++ ){
					for( int i=0; i<2;i++){
						patch_a = (BezierSurfacePatch*)getPatchAt(i,j);
						patch_b = (BezierSurfacePatch*)getPatchAt(i+1,j);
						patch_a->ConnectWithAnotherPatchWithSameDegree(patch_b,LEFT,RIGHT);
					}
				}
				//horizontal connecting
				for( int i=0;i<3;i++ ){
					for( int j=0; j<2; j++ ){
						patch_a = (BezierSurfacePatch*)getPatchAt(i,j);
						patch_b = (BezierSurfacePatch*)getPatchAt(i,j+1);
						patch_a->ConnectWithAnotherPatchWithSameDegree(patch_b,TOP,BOTTOM);
					}
				}

				break;
			case 3:
				for( int i=0; i < container.size(); i++ ){
					container[i]->ReInitialize();
				}
				break;

		}


	}


public:
	BezierManager(){
		demo_index = 1;
		Demo(1);
		
	}
	BezierPatch* getPatchAt( int i, int j, int row_len=3 ){
		int index = i*row_len + j;
		if ( index < container.size() ) return container[index];
		else{
			printf("index exceeds dimension error\n");
			return NULL;
		}
	}



	void RemoveAllPatches(){
		BezierPatch* p = NULL;
		for ( int i =0 ; i < container.size(); i++ ){
			p = container[i];
			delete p;
		}
		container.clear();
	}
	void setDemoNumber( int index ){
		this->demo_index = index;
	}

	void Demo(  int actionNumber ){
		switch ( demo_index )
		{
		case 1:
			Demo1(actionNumber );
			break;
		case 2:
			Demo2(actionNumber );
			break;
		}
	}


	void DrawAllPatches(){
		for( int i=0; i < container.size(); i ++ ){
			container[i]->DrawSurface();
		}
	}

	int getCurrentPatch( vector2D input ){
		int flag = -1,i;
		for (  i=0; i < container.size(); i++ ){
			if( container[i]->checkSurfaceSelected(input ) == 1 ){
				currentpatch = container[i];
				flag = 1;
				break;
			}
		}
		for ( i++ ; i < container.size(); i++ ){
			container[i]->setShowCtrlPoints( 0 );
		}

		return flag;

	}
	int getPatchSelectedControlPoint( vector2D input ){
		return currentpatch->getTheNearestCtrlPointIndex( input );
	}

	int UpdateControlPointPosition( double deltax, double deltay ){
		currentpatch->updateSelectedControlPointPosition( deltax,deltay );
		return 1;
	}


	void UpdateCurrentPatch(){
		currentpatch->UpdateBezierSurface();
	}

	void UpdatePatchOffset(float dx, float dy ){
		currentpatch->offsetX = dx;
		currentpatch->offsetY = dy;
	}


};