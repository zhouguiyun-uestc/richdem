//#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
//#include "catch/catch.hpp"
#include "doctest.h"
#include "richdem/common/Array2D.hpp"

#include <richdem/methods/d8_methods.hpp>
#include <richdem/common/grid_cell.hpp>
#include <richdem/depressions/Zhou2016pf.hpp>
#include <richdem/depressions/priority_flood.hpp>
#include <richdem/methods/dall_methods.hpp>
#include <richdem/common/ManagedVector.hpp>
using namespace richdem;

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

using namespace richdem;

TEST_CASE("ManagedVector Construction"){
  auto TestFunc = [](){ManagedVector<int> vec(30,5); return vec;};

  ManagedVector<int>   vec(30,4);
  ManagedVector<float> vecfl(30,4.5);

  auto vec2 = vec;
  auto vec3(vec);
  auto vec4 = TestFunc();

  vec2 = vecfl;
}

TEST_CASE("ManagedVector Resizing"){
  std::vector<int> datavec(30,4);

  ManagedVector<int> mvec(datavec.data(), 30);

  REQUIRE_THROWS(mvec.resize(40));
  REQUIRE_THROWS(mvec.resize(10));
  REQUIRE_NOTHROW(mvec.resize(30));
}

TEST_CASE( "Array2D works" ) {

  SUBCASE( "A 7x11 Array2D<float>" ) {
    Array2D<float> arr(7,11);

    REQUIRE( arr.width()  == 7  );
    REQUIRE( arr.height() == 11 );
    REQUIRE( arr.size()   == 77 );

    /*
      0   1  2  3 4   5  6
      7   8  9 10 11 12 13
      14 15 16 17 18 19 20
    */

    int x,y;

    arr.iToxy(0,x,y);
    REQUIRE( x==0 );
    REQUIRE( y==0 );

    arr.iToxy(arr.size()-1,x,y);
    REQUIRE( x==arr.width()-1  );
    REQUIRE( y==arr.height()-1 );

    REQUIRE( arr.xyToI(2,1)==9  );
    REQUIRE( arr.xyToI(6,2)==20 );

    int val = 0;
    for(int y=0;y<arr.height();y++)
    for(int x=0;x<arr.width();x++)
        arr(x,y) = val++;

    REQUIRE(arr(2,1)==9);
    REQUIRE(arr(6,2)==20);
    REQUIRE(arr(0,0)==0);
            }

  SUBCASE("Resize test"){
    Array2D<float> arr;
    arr.resize(3,5,4);
    REQUIRE( arr.width()  == 3  );
    REQUIRE( arr.height() == 5  );
    REQUIRE( arr.size()   == 15 );  

    REQUIRE( arr(2,1)==4 );
    REQUIRE( arr(2,2)==4 );
    REQUIRE( arr(0,0)==4 );
            }

  SUBCASE("Copy test"){
    Array2D<float> arr0(3,5,4);
    auto arr1 = arr0;
    auto arr2 = arr0;
    auto arr3 = arr0;
    auto arr4 = arr0;
    auto arr5 = arr0;
            }

  SUBCASE("Clear"){
    Array2D<float> arr0(3,5,2);
    auto arr1 = arr0;
    arr0.clear();
  }
}


TEST_CASE("Checking flow accumulation") {
  for(auto p: fs::directory_iterator("flow_accum")){
    fs::path this_path = p.path();
    if(this_path.extension()==".d8"){
        Array2D<d8_flowdir_t> fds(this_path, false);
        Array2D<int32_t>   correct_ans(this_path.replace_extension("out"), false);
        Array2D<int32_t>   my_ans;
        d8_flow_accum(fds,my_ans);
        REQUIRE( correct_ans == my_ans );
      }
    }
  }  





TEST_CASE("Checking GridCellZk_pq") {
  GridCellZk_pq<int> pq;

  SUBCASE("Testing Elevation Ordering"){
    pq.emplace(0,0,0);
    pq.emplace(1,0,1);
    pq.emplace(2,0,2);
    pq.emplace(3,0,3);
    REQUIRE(pq.top().x==0); pq.pop();
    REQUIRE(pq.top().x==1); pq.pop();
    REQUIRE(pq.top().x==2); pq.pop();
    REQUIRE(pq.top().x==3); pq.pop();
    REQUIRE(pq.empty()==true);
  }

  SUBCASE("Testing Insertion Ordering"){
    pq.emplace(0,0,0);
    pq.emplace(1,0,0);
    pq.emplace(2,0,0);
    pq.emplace(3,0,0);
    REQUIRE(pq.top().x==0); pq.pop();
    REQUIRE(pq.top().x==1); pq.pop();
    REQUIRE(pq.top().x==2); pq.pop();
    REQUIRE(pq.top().x==3); pq.pop();
    REQUIRE(pq.empty()==true);
  }

  SUBCASE("Testing Mixed Ordering"){
    pq.emplace(0,0,0);
    pq.emplace(1,0,1);
    pq.emplace(2,0,1);
    pq.emplace(3,0,2);
    REQUIRE(pq.top().x==0); pq.pop();
    REQUIRE(pq.top().x==1); pq.pop();
    REQUIRE(pq.top().x==2); pq.pop();
    REQUIRE(pq.top().x==3); pq.pop();
    REQUIRE(pq.empty()==true);
  }
}


TEST_CASE("Checking depression filling") {
  RDLOG_DEBUG<<"About to load depressions/testdem1.dem";
  Array2D<int> elevation_orig("depressions/testdem1.dem");
  RDLOG_DEBUG<<"Loaded depressions/testdem1.dem";

  SUBCASE("original_priority_flood"){
    auto elevation = elevation_orig;
    original_priority_flood(elevation);
    Array2D<int> manually_flooded("depressions/testdem1.all.out");
    REQUIRE(elevation==manually_flooded);
  }

  SUBCASE("improved_priority_flood"){
    auto elevation = elevation_orig;
    improved_priority_flood(elevation);
    Array2D<int> manually_flooded("depressions/testdem1.all.out");
    REQUIRE(elevation==manually_flooded);
  }

  SUBCASE("Zhou2016"){
    auto elevation = elevation_orig;
    Zhou2016(elevation);
    Array2D<int> manually_flooded("depressions/testdem1.all.out");
    REQUIRE(elevation==manually_flooded);
  }

  SUBCASE("improved_priority_flood_max_dep"){
    auto elevation = elevation_orig;
    improved_priority_flood_max_dep(elevation,1);
    elevation.printAll();
    Array2D<int> manually_flooded("depressions/testdem1.1.out");
    REQUIRE(elevation==manually_flooded);
  }

  SUBCASE("improved_priority_flood_max_dep"){
    auto elevation = elevation_orig;
    improved_priority_flood_max_dep(elevation,2);
    elevation.printAll();
    Array2D<int> manually_flooded("depressions/testdem1.2.out");
    REQUIRE(elevation==manually_flooded);
  }

}

TEST_CASE("Checking depression filling") {
  Array2D<float> beauford("beauford/beauford.tif");

  SUBCASE("FA_Tarboton")          {Array2D<double> accum; FA_Tarboton          (beauford, accum); }
  SUBCASE("FA_Holmgren")          {Array2D<double> accum; FA_Holmgren          (beauford, accum, 0.8); }
  SUBCASE("FA_Quinn")             {Array2D<double> accum; FA_Quinn             (beauford, accum); }
  SUBCASE("FA_Freeman")           {Array2D<double> accum; FA_Freeman           (beauford, accum, 0.8); }
  SUBCASE("FA_FairfieldLeymarie") {Array2D<double> accum; FA_FairfieldLeymarie (beauford, accum); }
  SUBCASE("FA_Rho8")              {Array2D<double> accum; FA_Rho8              (beauford, accum); }
  // SUBCASE("FA_D8")                {Array2D<double> accum; FA_D8                (beauford, accum); }

}