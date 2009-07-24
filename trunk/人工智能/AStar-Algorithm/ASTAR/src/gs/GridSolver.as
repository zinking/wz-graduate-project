package gs
{
	import mx.collections.Sort;
	
	public class GridSolver{
		public var srcState:State = new State( 0 );
		public var dstState:State = new State( 10000);
		
		public var priority_queue:Array = new Array();
		public var render_queue:Array 	= new Array();
		public var search_stack:Array 	= new Array();
		
		public var num_itr:int = 1;
		
		[bindable]
		public var searchdepth:int = 0;
		[bindable]
		public var searchedNodes:int = 0;
			
		private var sort:Sort = new Sort();
		private var comp:Function = new Function;
		public function GridSolver(){
			comp = astar_comp;
			
			dstState.state_data = [ 
				[4,7,9],
				[8,2,6],
				[1,5,3]
			];
			dstState.posX = 1;
			dstState.posY = 1;
			srcState.label = "start";
			dstState.label = "end";
			srcState.Initiallize( 0, dstState );	
			priority_queue.push( srcState );
			search_stack.push( srcState);
		}
		
		public function Solve( s:State, d:State ):void{
			num_itr = 1;
			emptyStacks();
			srcState = s;
			dstState = d;
			srcState.Initiallize( 0, dstState );
			priority_queue.push( srcState );
			search_stack.push( srcState);
			this.execute();
			
		}
		
		private function emptyStacks():void{
			var i:int = 0;
			var irlen:int  = render_queue.length;
			var prlen:int  = priority_queue.length;
			var srlen:int  = search_stack.length;
			for ( i = 0; i< irlen; i++ ){
				render_queue.pop();
			}
			for ( i = 0; i< prlen; i++ ){
				priority_queue.pop();
			}
			for ( i = 0; i< srlen; i++ ){
				search_stack.pop();
			}
		}
		
		public function setCompareCriteria( index:int):void{
			if ( index == 0 ) comp = astar_comp;
			else if( index == 1 ) comp = mclimb_comp;
		}
		
		private function  astar_comp(a:Object, b:Object, fields:Array = null):int{
			var sa:State = a as State;
			var sb:State = b as State;
			if ( sa.cost == sb.cost ) return -(sa.f - sb.f);
			else return -(sa.cost - sb.cost);
		}
		private function  mclimb_comp(a:Object, b:Object, fields:Array = null):int{
			var sa:State = a as State;
			var sb:State = b as State;
			return -(sa.g - sb.g);
		}
		
		public function smallest():State{
			return render_queue[ render_queue.length -1 ];
		}
		
		private function isNewState( s:State):Boolean{
			var isInPQ:Boolean = true;
			var isInRQ:Boolean = true;
			var os:State;
			for each(  os in  priority_queue ){
				if ( s.Equal( os ) ) isInPQ = false;
			}
			for each(  os in  render_queue ){
				if ( s.Equal( os ) ) isInRQ = false;
			}
			
			return isInPQ && isInRQ;
		}

				
		public function execute():void{
			var smallest:State = priority_queue.pop();
			var nstate:State;
			while ( smallest.g != 0 && num_itr < 100){
				render_queue.push(smallest);
				for each ( var d:int in smallest.getAvailableDirections() ){
					nstate = smallest.Move2NewState( d,dstState, num_itr );
					search_stack.push( nstate );
					if ( isNewState(nstate) ) {
						priority_queue.push( nstate );					
					}
					num_itr ++;
				} 
				priority_queue.sort( comp );
				smallest = priority_queue.pop();
													
			}
			smallest.label = "end";
			render_queue.push(smallest);
			search_stack.push( smallest );	
			this.searchdepth =smallest.f+1;
			this.searchedNodes = search_stack.length+1;	
		}

	}
}