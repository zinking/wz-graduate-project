package gs
{
	import com.adobe.flex.extras.controls.springgraph.Item;
	
	import flash.geom.Point;

	public class State extends Item{
		
		public var g:int = 0;
		public var f:int = 0;
		public var cost:int = 0;
		public var posX:int = 2;
		public var posY:int = 1;
		public var parent:State;
		public var parent2childDirection:int = 0;
		private var state_id:Number = 0;
		
		public var label:String;
		
		public var state_data:Array = [
			[7,8,9],
			[4,5,6],
			[1,2,3]
		];
		public function State( id:Number ){
			this.state_id = id;
			super( state_id.toString() );
			
		}
		
		public function go( pos:Point ):void{
			state_data[posX][posY] = state_data[pos.x][pos.y];
			state_data[pos.x][pos.y] = 2;
			posX = pos.x;
			posY = pos.y;
		}
		
		public function toString():String{
			return "f:"+f+" g:"+g+" x:"+posX+" y:"+posY;
		}
		public function Copy( instance:State ):void{
			this.cost = instance.cost;
			this.f 	  = instance.f;
			this.g    = instance.g;
			this.posX = instance.posX;
			this.posY = instance.posY;
			
			for ( var i:int = 0; i < 3; i++){
				for ( var j:int = 0; j < 3; j++ ){
					 this.state_data[i][j] = instance.state_data[i][j];
				}
			} 
			
		}
		
		public function Initiallize(  f:int,  dst:State ):void{
			this.f = f;
			CalclateG( dst );
			cost = f + g;
		}
		
		public function Equal( ins:State):Boolean{
			return ( g == ins.g && f == ins.f &&
				posX == ins.posX && posY == ins.posY )
		}
		
		public function getAvailableDirections():Array{
				var directions:Array = new Array();
				var ds:Array = [UP,DOWN,LEFT,RIGHT];
				for each ( var d:int in ds ){
					if ( d != - parent2childDirection ){
						switch( d ){
							case UP:
								if ( posX -1 >= 0 ) directions.push(d);
							break;
							case DOWN:
								if ( posX +1 <= 2 ) directions.push(d);
							break;
							case RIGHT:
								if ( posY +1 <= 2 ) directions.push(d);
							break;
							case LEFT:
								if ( posY -1 >= 0 ) directions.push(d);
							break;
						}
					}
				}
				//trace("Get available directions" + directions.length );
				
				return directions;	
		}
		public function CalclateG(  dst:State ):void{
			var diff_num :int = 0;
			for ( var i:int = 0; i < 3; i++){
				for ( var j:int = 0; j < 3; j++ ){
					if ( state_data[i][j] != dst.state_data[i][j] ) diff_num ++;
				}
			} 
			
			g = diff_num;
		}
		
		private function MoveContent( x:int,  y:int ):void{
			var tmp:int = state_data[posX][posY];
			state_data[posX][posY] = state_data[x][y];
			state_data[x][y] = tmp;
		}
		
		
		public function Move2NewState(  direction:int, dst:State, num:Number ):State{
			
			var newState:State = new State( num );
			newState.Copy(this);
			switch( direction ){
				case UP:
					newState.posX--; 
				break;
				case DOWN:
					newState.posX++;
				break;
				case RIGHT:
					newState.posY++;
				break;
				case LEFT:
					newState.posY--;
				break;
			}
			newState.MoveContent( this.posX, this.posY );
			newState.Initiallize( this.f+1, dst );
			newState.parent = this;
			newState.parent2childDirection = direction;

			
			return newState;
			
		}
		
		public static const UP:int = 1;
		public static const DOWN:int = -1;
		public static const LEFT:int = 3;
		public static const RIGHT:int = -3;
	}
}