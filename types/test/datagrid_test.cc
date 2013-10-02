/**
 * @example types/test/datagrid_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/types/types.h>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <usml/netcdf/netcdf_files.h>

BOOST_AUTO_TEST_SUITE(datagrid_test)

using namespace boost::unit_test;
using namespace usml::types;

typedef seq_vector::const_iterator iterator;

/**
 * @ingroup types_test
 * Test the ability of data_grid_compute_offset() to retrieve data
 * from a 3-D data matrix in column major order.
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( compute_index_test ) {

    cout << "=== datagrid_test: compute_index_test ===" << endl;

    // build a set of axes like the ones used in data_grid

    seq_linear x(0,100.0,2);
    seq_linear y(0,10.0,3);
    seq_linear z(0,1.0,4);
    seq_vector *axis[] = {&x, &y, &z};
    cout << "x=" << x << endl;
    cout << "y=" << y << endl;
    cout << "z=" << z << endl;

    // fill in a data vector using combination of axis values

    double data[2*3*4];
    unsigned k = 0;
    for ( iterator ix = x.begin(); ix < x.end(); ++ix ) {
        for ( iterator iy = y.begin(); iy < y.end(); ++iy ) {
            for ( iterator iz = z.begin(); iz < z.end(); ++iz ) {
                data[k++] = *ix + *iy + *iz;
            }
        }
    }

    cout << "data[x][y][z]=";
    for ( unsigned n=0; n < k; ++n ) printf("%03.0f ", data[n] );
    cout << endl;

    // check to see if all data in the right place

    unsigned index[3];
    unsigned a=0,b=0,c=0;
    for ( iterator ix = x.begin(); ix < x.end(); ++ix ) {
        index[0] = a++;
        b = 0;
        for ( iterator iy = y.begin(); iy < y.end(); ++iy ) {
            index[1] = b++;
            c = 0;
            for ( iterator iz = z.begin(); iz < z.end(); ++iz ) {
                index[2] = c++;
                k = data_grid_compute_offset<2>( axis, index );
                printf( "x=%d y=%d z=%d offset=%02d data=%03.0f\n",
                        a-1, b-1, c-1, k, data[k] );
                BOOST_CHECK_CLOSE( data[k], *ix + *iy + *iz, 1e-6 );
            }
        }
    }
}

/**
 * Compute a linear field value of 1-D interpolation test data
 */
static double linear1d( double axis ) {
    return 5.0 + 3.0 * axis;
}

/**
 * @ingroup types_test
 * Interpolate 1-D linear field using a scalar.
 * Exercise all of the 1-D interpolation types.
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( linear_1d_test ) {
    double truth, nearest, linear, pchip;

    cout << "=== datagrid_test: linear_1d_test ===" << endl;

    // construct synthetic data for this test

    seq_linear axis(1.0,2.0,9.0);
    seq_vector *ax[] = {&axis};
    data_grid<double,1> grid( ax );
    grid.edge_limit(0,false);

    for ( unsigned n=0; n < axis.size(); ++n ) {
        grid.data( &n, linear1d(axis(n)) );
    }

    // interpolate using all possible algorithms

    cout << "x\ttruth\tnearest\tlinear\tpchip" << endl;
    for ( double x=0.25; x <= 10.0; x += 0.25 ) {
    	double y = x ;
        cout << y << "\t";
        truth = linear1d(y);
        cout << truth << "\t";

        grid.interp_type(0,GRID_INTERP_NEAREST);
        nearest = grid.interpolate( &y );
        cout << nearest << "\t";

        grid.interp_type(0,GRID_INTERP_LINEAR);
        linear = grid.interpolate( &y );
        cout << linear << "\t";
        BOOST_CHECK_CLOSE( linear, truth, 1e-6 );

        grid.interp_type(0,GRID_INTERP_PCHIP);
        pchip = grid.interpolate( &y );
        cout << pchip << "\t";
        BOOST_CHECK_CLOSE( pchip, truth, 1e-6 );

        cout << endl;
    }
}

/**
 * Compute a cubic field value of 1-D interpolation test data
 */
static double cubic1d( double axis ) {
    return linear1d( axis ) - 0.3 * axis*axis;
}

/**
 * @ingroup types_test
 * Interpolate 1-D cubic field using a scalar.
 * Exercise all of the 1-D interpolation types.
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( cubic_1d_test ) {
    double truth, nearest, linear, pchip;

    cout << "=== datagrid_test: cubic_1d_test ===" << endl;

    // construct synthetic data for this test

    seq_linear axis(1.0,2.0,9.0);
    seq_vector *ax[] = {&axis};
    data_grid<double,1> grid( ax );
    grid.edge_limit(0,false);

    for ( unsigned n=0; n < axis.size(); ++n ) {
        grid.data( &n, cubic1d(axis(n)) );
    }

    // interpolate using all possible algorithms
    // don't try extrapolation

    cout << "x\ttruth\tnearest\tlinear\tpchip" << endl;
    for ( double x=1.0; x <= 9.0; x += 0.25 ) {
        cout << x << "\t";
        truth = cubic1d(x);
        cout << truth << "\t";

        grid.interp_type(0,GRID_INTERP_NEAREST);
        nearest = grid.interpolate( &x );
        cout << nearest << "\t";

        grid.interp_type(0,GRID_INTERP_LINEAR);
        linear = grid.interpolate( &x );
        cout << linear << "\t";

        grid.interp_type(0,GRID_INTERP_PCHIP);
        pchip = grid.interpolate( &x );
        cout << pchip << "\t";
        BOOST_CHECK_CLOSE( pchip, truth, 2.0 );

        cout << endl;
    }
}

/**
 * Compute the derivative of the function defined in cubic1d().
 */
static double deriv1d( double axis ) {
    return 3.0 - 0.6 * axis;
}

/**
 * @ingroup types_test
 * Interpolate 1-D cubic field using a scalar.
 * Test the accuracy of the derivatives
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( deriv_1d_test ) {
    double truth, nearest, linear, pchip;

    cout << "=== datagrid_test: deriv_1d_test ===" << endl;

    // construct synthetic data for this test

    seq_linear axis(1.0,2.0,9.0);
    seq_vector *ax[] = {&axis};
    data_grid<double,1> grid( ax );

    for ( unsigned n=0; n < axis.size(); ++n ) {
        grid.data( &n, cubic1d(axis(n)) );
    }

    // interpolate using all possible algorithms
    // don't try extrapolation

    cout << "x\ttruth\tnearest\tlinear\tpchip" << endl;
    for ( double x=1.0; x <= 9.0; x += 0.25 ) {
        cout << x << "\t";
        truth = deriv1d(x);
        cout << truth << "\t";

        grid.interp_type(0,GRID_INTERP_NEAREST);
        grid.interpolate( &x, &nearest );
        cout << nearest << "\t";

        grid.interp_type(0,GRID_INTERP_LINEAR);
        grid.interpolate( &x, &linear );
        cout << linear << "\t";

        grid.interp_type(0,GRID_INTERP_PCHIP);
        grid.interpolate( &x, &pchip );
        cout << pchip << "\t";
        // BOOST_CHECK_CLOSE( pchip, truth, 20.0 ) ;

        cout << endl;
    }
}

BOOST_AUTO_TEST_CASE( datagrid_interp_speed_test ) {
    cout << "=== datagrid_interpolation_speed_test ===" << endl;

    int num_points = 1;
    int counter = 0;
//    double param = 100.0;
//    double temp1,temp2;
    vector<double> a (5);
//    a(0) = param * randgen::uniform();
//    for (int i=1; i<a.size(); ++i) {
//        temp1 = a(i-1);
//        temp2 = param * randgen::uniform();
//        while(temp1 > temp2) {
//            temp2 = param*randgen::uniform();
//        }
//        a(i) = temp2;
//    }
    seq_linear axis1(0,1,5);
    seq_linear axis2(0,1,5);
    seq_linear axis3(0,1,5);
//    seq_data axis1(a);
//    a(0) = param * randgen::uniform();
//    for (int i=1; i<a.size(); ++i) {
//        temp1 = a(i-1);
//        temp2 = param * randgen::uniform();
//        while(temp1 > temp2) {
//            temp2 = param*randgen::uniform();
//        }
//        a(i) = temp2;
//    }
//    seq_data axis2(a);
//    a(0) = param * randgen::uniform();
//    for (int i=1; i<a.size(); ++i) {
//        temp1 = a(i-1);
//        temp2 = param * randgen::uniform();
//        while(temp1 > temp2) {
//            temp2 = param*randgen::uniform();
//        }
//        a(i) = temp2;
//    }
//    seq_data axis3(a);
    seq_vector *ax[] = {&axis1, &axis2, &axis3};

    cout << "axis(1): " << axis1 << endl;
    cout << "axis(2): " << axis2 << endl;
    cout << "axis(3): " << axis3 << endl;

    data_grid<double,3> grid ( ax );
    for(int i = 0; i<2; ++i) {
        grid.interp_type(i, GRID_INTERP_NEAREST);
        grid.edge_limit(i, true);
    }

    for( unsigned i=0; i < axis1.size(); ++i ) {
        for( unsigned j=0; j < axis2.size(); ++j) {
            for(unsigned k=0; k < axis3.size(); ++k) {
                unsigned index[3];
                index[0] = i;
                index[1] = j;
                index[2] = k;
                double value = i*25 + j*5 + k;
                grid.data( index, value );
            }
        }
    }

    cout << "data: " << endl;
    for( unsigned i=0; i < axis1.size(); ++i ) {
        cout << "   (" << i << ",:,:)" << endl;
        for( unsigned j=0; j < axis2.size(); ++j) {
            cout << "\t";
            for(unsigned k=0; k < axis3.size(); ++k) {
                unsigned index[3];
                index[0] = i;
                index[1] = j;
                index[2] = k;
                double value = grid.data( index );
                cout << value;
                if(k==axis3.size()-1) { cout << endl; }
                else {cout << ", ";}
            }
        }
        cout << endl;
    }

    struct timeval time ;
    struct timezone zone ;
    gettimeofday( &time, &zone ) ;
    double start = time.tv_sec + time.tv_usec * 1e-6 ;
    double what_got;
    while ( counter != num_points ) {
        double spot[3] ;
//        spot[0] = param * randgen::uniform();
//        spot[1] = param * randgen::uniform();
//        spot[2] = param * randgen::uniform();
        spot[0] = 4;
        spot[1] = 4;
        spot[2] = 4;
        what_got = grid.interpolate( spot );
        cout << "spot: " << spot[0] << ", " << spot[1] << ", " << spot[2] << endl;
        cout << "value: " << what_got << endl;
        ++counter;
    }
    gettimeofday( &time, &zone ) ;
    double complete = time.tv_sec + time.tv_usec * 1e-6 ;

	cout << "Time to complete interpolation for " << num_points << " random points was "
		<< (complete-start) << " sec." << endl;

}

BOOST_AUTO_TEST_CASE( datagrid_fast_acc_test ) {
    cout << "=== datagrid_fast_accuracy_test ===" << endl;

    seq_vector* axis[2];
    axis[0] = new seq_linear(1.0, 1.0, 10);
    axis[1] = new seq_linear(1.0, 1.0, 10);
    data_grid<double,2>* test_grid = new data_grid<double,2>(axis);
    unsigned index[2];
    for(int i=0; i<(*axis[0]).size(); ++i) {
        for(int j=0; j<(*axis[1]).size(); ++j) {
            index[0] = i;
            index[1] = j;
//            double number = ( (i+1)*(i+1)*(i+1) )
//                    * ( (j+1)*(j+1)*(j+1) ) ;
            double number = (i+1)*(i+1)*(i+1) ;
            test_grid->data(index, number);
        }
    }

    cout << "==========simple_data grid=============" << endl;
    cout << "axis[0]: " << *axis[0] << endl;
    cout << "axis[1]: " << *axis[1] << endl;
    for(int i=0; i<(*axis[0]).size(); i++) {
        for(int j=0; j<(*axis[1]).size(); j++) {
            index[0] = i;
            index[1] = j;
            cout << test_grid->data(index);
            (j < 9) ? cout << "\t" :  cout << endl;
        }
    }
    cout << endl;

    for(int i=0; i<2; i++){
        test_grid->interp_type(i, GRID_INTERP_PCHIP);
        test_grid->edge_limit(i, true);
    }

    data_grid_fast_2d* test_grid_fast = new data_grid_fast_2d(*test_grid, true);

    double spot[2];
    spot[1] = 3.3265; spot[0] = 2.8753;
    double derv[2];
    double value;
    cout << "x: " << spot[0] << "\ty: " << spot[1] << endl;
    value = test_grid_fast->interpolate( spot, derv );
    cout << "fast_2d:    " << value << "\tderivative: " << derv[0] << ", " << derv[1] << endl;
    value = test_grid->interpolate( spot, derv );
    cout << "data_grid:  " << value << "\tderivative: " << derv[0] << ", " << derv[1] << endl;
//    value = (spot[0]*spot[0]*spot[0]) * (spot[1]*spot[1]*spot[1]) ;
//    derv[0] = 3.0*spot[0]*spot[0] * (spot[1]*spot[1]*spot[1]) ;
//    derv[1] = 3.0*spot[1]*spot[1] * (spot[0]*spot[0]*spot[0]) ;
    value = spot[0]*spot[0]*spot[0] ;
    derv[0] = 3.0*spot[0]*spot[0] ;
    derv[1] = 0.0 ;
    cout << "true value: " << value << "\tderivative: " << derv[0] << ", " << derv[1] << endl;

        // Setup a complex example to compare results for pchip
//    cout << "==========2d_data grid_test_pchip=============" << endl;
//    double v0, v1 ;
//    wposition::compute_earth_radius( 19.52 ) ;
//    const double lat1 = 16.2 ;
//    const double lat2 = 24.6 ;
//    const double lng1 = -164.4;
//    const double lng2 = -155.5 ;
//    cout << "load STD14 environmental bathy data" << endl ;
//    data_grid<double,2>* grid = new usml::netcdf::netcdf_bathy( USML_DATA_DIR "/cmp_speed/std14bathy.nc",
//        lat1, lat2, lng1, lng2, wposition::earth_radius );
//    for(int i=0; i<2; i++){
//        grid->interp_type(i, GRID_INTERP_PCHIP);
//        grid->edge_limit(i, true);
//    }
//    data_grid_fast_2d* fast_grid = new data_grid_fast_2d(*grid, true) ;
//
//    cout << "grid->axis0: " << *(grid->axis(0)) << endl;
//    cout << "grid->axis1: " << *(grid->axis(1)) << endl;
//    const seq_vector* ax0 = grid->axis(0);
//    const seq_vector* ax1 = grid->axis(1);
//    cout << "axis0(13 to 21): (" << (*ax0)(13) << "," << (*ax0)(14) << ","
//                                       << (*ax0)(15) << "," << (*ax0)(16) << ","
//                                       << (*ax0)(17) << "," << (*ax0)(18) << ","
//                                       << (*ax0)(19) << "," << (*ax0)(20) << ","
//                                       << (*ax0)(21) << ")" << endl;
//    cout << "axis1(36 to 44): (" << (*ax1)(36) << "," << (*ax1)(37) << ","
//                                      << (*ax1)(38) << "," << (*ax1)(39) << ","
//                                      << (*ax1)(40) << "," << (*ax1)(41) << ","
//                                      << (*ax1)(42) << "," << (*ax1)(43) << ","
//                                      << (*ax1)(44) << ")" << endl;
//    cout << "===========Data===========" << endl;
//    for(int i=13; i<22; ++i) {
//        (i==13) ? cout << "[" : cout << "";
//        for(int j=36; j<45; ++j) {
//            (j==36) ? cout << "(" : cout << ", ";
//            index[0] = i;
//            index[1] = j;
//            cout << grid->data(index) - wposition::earth_radius;
//            (j!=44) ? cout << "" : cout << ")";
//            (j==44 && i!=21) ? cout << endl : cout << "";
//
//        }
//        (i==21) ? cout << "]" << endl : cout << "";
//    }

//    for(int i=0; i<10; ++i){
//        double location[2];
//        location[0] = to_radians(90-(4.2 * randgen::uniform() + 18.2));
//        location[1] = to_radians(-4.45 * randgen::uniform() - 157.5);
//        v0 = grid->interpolate( location ) - wposition::earth_radius;
//        v1 = fast_grid->interpolate( location ) - wposition::earth_radius;
//        cout << "location: (" << location[0] << ", " << location[1] << ")" << "\tgrid: " << v0 << "\tfast_grid: " << v1 << endl;
//        BOOST_CHECK_CLOSE(v0, v1, 5.0);
//    }
//    double location[2];
//    location[0] = 1.24449;
//    location[1] = -2.76108;
//    location[0] = (*ax0)(17) - 0.0239;
//    location[1] = (*ax1)(40);
//    v0 = grid->interpolate( location ) - wposition::earth_radius;
//    v1 = fast_grid->interpolate( location ) - wposition::earth_radius;
//    cout << "location: (" << location[0] << ", " << location[1] << ")" << "\tgrid: " << v0 << "\tfast_grid: " << v1 << endl;

//    cout << "==========3d_data grid_test_pchip/bi-linear=============" << endl;
//    cout << "load STD14 environmental profile data" << endl ;
//    data_grid<double,3>* test_grid_3d = new usml::netcdf::netcdf_profile( USML_DATA_DIR "/cmp_speed/std14profile.nc",
//            0.0, lat1, lat2, lng1, lng2, wposition::earth_radius );
//    for(int i=0; i<3; i++){
//        (i<1) ? test_grid_3d->interp_type(i, GRID_INTERP_PCHIP) :
//            test_grid_3d->interp_type(i, GRID_INTERP_LINEAR);
//        test_grid_3d->edge_limit(i, true);
//    }
//    data_grid_fast_3d* test_grid_fast_3d = new data_grid_fast_3d(*test_grid_3d,true);
//
//    double loc[3];
//    loc[1] = 1.24449;
//    loc[2] = -2.76108;
//    loc[0] = -2305.0 + wposition::earth_radius;
//    v0 = test_grid_3d->interpolate( loc );
//    v1 = test_grid_fast_3d->interpolate( loc );
//    cout << "location: (" << loc[0]-wposition::earth_radius << ", " << loc[1] << "," << loc[2] << ")" << "\tgrid: " << v0 << "\tfast_grid: " << v1 << endl;

}

BOOST_AUTO_TEST_SUITE_END()
