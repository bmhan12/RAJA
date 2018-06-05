#include "RAJA/RAJA.hpp"

int main(int argc, char* argv[])
{

//Using policy = KernelPolicy<Tile<tile_dynamic<0>, seq_exec, …>>;
//RAJA::kernel_param<policy>(
// make_tuple(RangeSegment(0,N)),
//  make_tuple(32),  // param 0 is referenced by tile_dynamic
//  [=](int i, int tile_size){
//
//  });

  using namespace RAJA;

  kernel_param<
    KernelPolicy< 
      statement::Tile<0, statement::tile_dynamic<0>, seq_exec,
        statement::For<0, seq_exec, statement::Lambda<0> > 
      >
    >
  >(make_tuple(RangeSegment{0,25}),
      make_tuple(5),
     //make_tuple(TileSize(10)), // not sure we need this, good for static_assert
     [=](int i, int){
       std::cout << "Running index " << i << std::endl;
});

}
